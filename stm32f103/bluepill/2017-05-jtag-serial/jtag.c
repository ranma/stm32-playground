#include <stdio.h>
#include "stm32f103.h"
#include "usb.h"
#include "jtag.h"

#define OSBDM_CMD_INIT 0x11
#define OSBDM_CMD_SPECIAL 0x27
#define OSBDM_CMD_SPECIAL_SRST 0x01
#define OSBDM_CMD_SPECIAL_SWAP 0x05

enum jtag_state {
	RESET = 0,
	IDLE,
	DR_SCAN,
	IR_SCAN,
	CAPTURE_DR,
	SHIFT_DR,
	EXIT1_DR,
	PAUSE_DR,
	EXIT2_DR,
	UPDATE_DR,
	CAPTURE_IR,
	SHIFT_IR,
	EXIT1_IR,
	PAUSE_IR,
	EXIT2_IR,
	UPDATE_IR,
};

static const char const *jtag_state_name[] = {
	"RST",
	"IDL",
	"DRS",
	"IRS",
	"CDR",
	"SDR",
	"E1DR",
	"PDR",
	"E2DR",
	"UDR",
	"CIR",
	"SIR",
	"E1IR",
	"PIR",
	"E2IR",
	"UIR",
};

enum jtag_state jtag_sm[][2] = {
	{IDLE, RESET}, // RESET
	{IDLE, DR_SCAN}, // IDLE
	{CAPTURE_DR, IR_SCAN}, // DR_SCAN
	{CAPTURE_IR, RESET}, // IR_SCAN
	{SHIFT_DR, EXIT1_DR}, // CAPTURE_DR
	{SHIFT_DR, EXIT1_DR}, // SHIFT_DR
	{PAUSE_DR, UPDATE_DR}, // EXIT1_DR
	{PAUSE_DR, EXIT2_DR}, // PAUSE_DR
	{SHIFT_DR, UPDATE_DR}, // EXIT2_DR
	{IDLE, DR_SCAN}, // UPDATE_DR
	{SHIFT_IR, EXIT1_IR}, // CAPTURE_IR
	{SHIFT_IR, EXIT1_IR}, // SHIFT_IR
	{PAUSE_IR, UPDATE_IR}, // EXIT1_IR
	{PAUSE_IR, EXIT2_IR}, // PAUSE_IR
	{SHIFT_IR, UPDATE_IR}, // EXIT2_IR
	{IDLE, IR_SCAN}, // UPDATE_IR
};

enum jtag_state jtag_state = RESET;
uint32_t jtag_si;
uint32_t jtag_so;
int jtag_bits;

#define DEBUG_JTAG 0

static int jtag_toggle_tck(void)
{
	int ret = 0;
	GPIOA->BSRR = 0x00010000;
	udelay(1);
	if (GPIOA->IDR & 0x0008) {
		ret = 1;
	}
	GPIOA->BSRR = 0x00000001;
	udelay(1);

#if DEBUG_JTAG == 1
	int tms = !!(GPIOA->IDR & 0x0002);
	int tdi = !!(GPIOA->IDR & 0x0004);

	printf(" %d%d%d", !!tms, !!tdi, ret);

	enum jtag_state new = jtag_sm[jtag_state][tms];
	if (new != jtag_state) {
		printf(" %s", jtag_state_name[new]);
		jtag_state = new;
		if (jtag_state == UPDATE_DR || jtag_state == UPDATE_IR) {
			if (jtag_state == UPDATE_DR) {
				printf(" DR");
			} else {
				printf(" IR");
			}
			if (jtag_bits < 32) {
				printf("(%d)=%lx:%lx",
					jtag_bits,
					jtag_si >> (32 - jtag_bits),
					jtag_so >> (32 - jtag_bits));
			} else {
				printf("(%d)=%lx:%lx", jtag_bits, jtag_si, jtag_so);
			}
			jtag_si = 0;
			jtag_so = 0;
			jtag_bits = 0;
		}
	}
	if (jtag_state == SHIFT_IR || jtag_state == SHIFT_DR) {
		jtag_bits++;
		jtag_si >>= 1;
		if (tdi) {
			jtag_si |= 0x8000000;
		}
		jtag_so >>= 1;
		if (ret) {
			jtag_so |= 0x8000000;
		}
	}
#endif
	return ret;
}

static void jtag_set_tmstdi(int tms, int tdi)
{
	uint32_t sr = 0;
	if (tms) {
		sr |= 0x00000002;
	} else {
		sr |= 0x00020000;
	}
	if (tdi) {
		sr |= 0x00000004;
	} else {
		sr |= 0x00040000;
	}
	GPIOA->BSRR = sr;
}

static void jtag_tap_reset(void)
{
	// Go to RESET state.
	jtag_set_tmstdi(1, 1);
	for (int i = 0; i < 6; i++) {
		jtag_toggle_tck();
	}
	// Go to IDLE state.
	jtag_set_tmstdi(0, 1);
	jtag_toggle_tck();
}

static void jtag_swap1(uint8_t *indata, uint8_t *outdata)
{
	uint8_t bits = indata[0];
	uint16_t tdi = (indata[1] << 8) | indata[2];
	uint16_t tms = (indata[3] << 8) | indata[4];
	uint16_t tdo = 0;
	uint16_t mask = 1;

	for (int i = 0; i < bits; i++) {
		jtag_set_tmstdi(tms & mask, tdi & mask);
		tdo >>= 1;
		tdo |= jtag_toggle_tck() ? 0x8000 : 0;
		mask <<= 1;
	}

	outdata[0] = tdo >> 8;
	outdata[1] = tdo & 0xff;
}

int jtag_rxtx(uint8_t *jtag_rx, uint8_t *jtag_tx)
{
	int ntx = 0;

	switch (jtag_rx[0]) {
	case OSBDM_CMD_INIT:
		ntx = 2;
		jtag_tx[0] = jtag_rx[0];
		jtag_tx[1] = ntx;
		jtag_tap_reset();
		break;
	case OSBDM_CMD_SPECIAL:
		switch (jtag_rx[1]) {
		case OSBDM_CMD_SPECIAL_SRST:
			printf("srst=%02x\n", jtag_rx[4]);
			ntx = 2;
			jtag_tx[0] = jtag_rx[0];
			jtag_tx[1] = ntx;
			break;
		case OSBDM_CMD_SPECIAL_SWAP: {
				// e.g. 27 05 00 00 00 01 05 00 00 00 1f
				//      <cmd> <???> <len> bc <tdi> <tms>
				int nswap = jtag_rx[5];
				ntx = 4 + 2 * nswap;
				jtag_tx[0] = jtag_rx[0];
				jtag_tx[1] = ntx;
				jtag_tx[2] = 0;
				jtag_tx[3] = ntx - 4;
				uint8_t *rx = &jtag_rx[6];
				uint8_t *tx = &jtag_tx[4];
				for (int i = 0; i < nswap; i++, rx += 5, tx += 2) {
					jtag_swap1(rx, tx);
				}
			}
			break;
		}
		break;
	}

	return ntx;
}

static void jtag_set_ir(int ir, int len)
{
	// IDLE->IRSCAN
	jtag_set_tmstdi(1, 1);
	jtag_toggle_tck(); // Select-DR-Scan
	jtag_toggle_tck(); // Select-IR-Scan
	jtag_set_tmstdi(0, 1);
	jtag_toggle_tck(); // Capture-IR
	jtag_toggle_tck(); // Shift-IR
	for (int i = 0; i < len; i++) {
		jtag_set_tmstdi(i == (len - 1), ir & 1);
		jtag_toggle_tck(); // Shift-IR
		ir >>= 1;
	}
	jtag_toggle_tck(); // Update-IR
	jtag_set_tmstdi(0, 1);
	jtag_toggle_tck(); // IDLE
}

static uint32_t jtag_get_dr(int len)
{
	uint32_t dr = 0;
	// IDLE->DRSCAN
	jtag_set_tmstdi(1, 1);
	jtag_toggle_tck(); // Select-DR-Scan
	jtag_set_tmstdi(0, 1);
	jtag_toggle_tck(); // Capture-DR
	jtag_toggle_tck(); // Shift-DR
	for (int i = 0; i < len; i++) {
		jtag_set_tmstdi(i == (len - 1), 1);
		dr >>= 1;
		dr |= jtag_toggle_tck() ? 0x80000000 : 0; // Shift-DR
	}
	jtag_toggle_tck(); // Update-DR
	jtag_set_tmstdi(0, 1);
	jtag_toggle_tck(); // IDLE
	return dr;
}

void jtag_init(void)
{
	// Enable GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// A0-A2 as output (TCK, TMS, TDI)
	// A3 as input (TDO)
	GPIOA->CRL = ~0xffff;
	GPIOA->CRL |= 0x4111; // Push-pull output

	puts("tap_reset");
	jtag_tap_reset();
	printf("\nset_ir");
	jtag_set_ir(1, 5); // IDCODE
	printf("\nget_dr");
	uint32_t id = jtag_get_dr(32);
	printf("\nid=%lx\n", id);
}
