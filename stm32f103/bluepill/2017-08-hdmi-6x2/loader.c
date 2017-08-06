#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f103.h"
#include "tasklet.h"

static void halt(void)
{
	for(;;);
}

extern int usb_irqs;
extern int usb_errs;
extern volatile int systicks;

static uint32_t x = 0xbadc0ffe;
static uint32_t x0;
extern int uart_wfi;

struct gpio_pin {
	struct gpio_regs *bank;
	int pin;
};

static void gpio_pin_cfg(struct gpio_pin *pin, int cfg)
{
	const int shift = 4 * (pin->pin & 7);
	const uint32_t mask = 0xf << shift;
	const uint32_t value = (cfg & 0xf) << shift;
	if (pin->pin < 8) {
		printf("%08x CRL p%d m%08x v%08x\n",
			(int)pin->bank, pin->pin, (int)mask, (int)value);
		pin->bank->CRL &= ~mask;
		pin->bank->CRL |= value;
	} else {
		printf("%08x CRH p%d m%08x v%08x\n",
			(int)pin->bank, pin->pin, (int)mask, (int)value);
		pin->bank->CRH &= ~mask;
		pin->bank->CRH |= value;
	}
}

static void gpio_pin_setvalue(struct gpio_pin *pin, int value)
{
	const uint32_t mask = 1 << pin->pin;
	if (value) {
		pin->bank->BSRR = mask;
	} else {
		pin->bank->BRR = mask;
	}
}

static int gpio_pin_read(struct gpio_pin *pin)
{
	const uint32_t mask = 1 << pin->pin;
	return !!(pin->bank->IDR & mask);
}

struct i2c_pair {
	struct gpio_pin sda;
	struct gpio_pin scl;
	int delay_us;
};

static void i2c_stop(struct i2c_pair *pair)
{
	gpio_pin_setvalue(&pair->sda, 0);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->scl, 1);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->sda, 1);
}

static void i2c_start(struct i2c_pair *pair)
{
	gpio_pin_setvalue(&pair->sda, 1);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->scl, 1);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->sda, 0);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->scl, 0);
}

static void i2c_setup(struct i2c_pair *pair)
{
	// Open-drain 50MHz
	gpio_pin_cfg(&pair->sda, 0x7);
	// Push-pull 50MHz
	gpio_pin_cfg(&pair->scl, 0x3);
	i2c_stop(pair);
}

static void i2c_send_bit(struct i2c_pair *pair, int bit)
{
	gpio_pin_setvalue(&pair->sda, bit);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->scl, 1);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->scl, 0);
}

static int i2c_recv_bit(struct i2c_pair *pair)
{
	int value = 0;
	gpio_pin_setvalue(&pair->sda, 1);
	udelay(pair->delay_us);
	gpio_pin_setvalue(&pair->scl, 1);
	udelay(pair->delay_us);
	value = gpio_pin_read(&pair->sda);
	gpio_pin_setvalue(&pair->scl, 0);
	return value;
}

static int i2c_send_byte(struct i2c_pair *pair, uint8_t x)
{
	for (int i = 0; i < 8; i++) {
		i2c_send_bit(pair, !!(x & 0x80));
		x <<= 1;
	}
	int nak = i2c_recv_bit(pair);
	return nak;
}

static uint8_t i2c_recv_byte(struct i2c_pair *pair, int nak)
{
	uint8_t x = 0;
	for (int i = 0; i < 8; i++) {
		x <<= 1;
		x |= i2c_recv_bit(pair);
	}
	i2c_send_bit(pair, nak);
	return x;
}

static int i2c_read_byte(struct i2c_pair *pair, uint8_t addr)
{
	int value = 0;
	i2c_start(pair);
	if (i2c_send_byte(pair, addr | 1)) {
		printf("read fail: addr=%02x\n", addr);
		value = -1;
		goto stop;
	}
	value = i2c_recv_byte(pair, 1);
stop:
	i2c_stop(pair);
	return value;
}

static int i2c_read_eep_byte(struct i2c_pair *pair, uint8_t addr, uint8_t eep_addr)
{
	int value = 0;
	i2c_start(pair);
	if (i2c_send_byte(pair, addr & 0xfe)) {
		printf("read fail: addr=%02x\n", addr);
		value = -1;
		goto stop;
	}
	if (i2c_send_byte(pair, eep_addr)) {
		printf("read fail: eep_addr=%02x\n", eep_addr);
		value = -1;
		goto stop;
	}
	i2c_start(pair);
	if (i2c_send_byte(pair, addr | 1)) {
		printf("read fail: repeat addr=%02x\n", addr);
		value = -1;
		goto stop;
	}
	value = i2c_recv_byte(pair, 1);
stop:
	i2c_stop(pair);
	return value;
}

static void i2c_dump_eep(struct i2c_pair *pair, uint8_t addr)
{
	int i = 0;
	int j = 0;
	i2c_start(pair);
	if (i2c_send_byte(pair, addr & 0xfe)) {
		printf("dump fail: addr=%02x\n", addr);
		goto stop;
	}
	if (i2c_send_byte(pair, 0x00)) {
		printf("dump fail: reset eep_addr\n");
		goto stop;
	}
	i2c_start(pair);
	if (i2c_send_byte(pair, addr | 1)) {
		printf("dump fail: repeat addr=%02x\n", addr);
		goto stop;
	}
	while (i < 0x100) {
		printf("  %02x:", i);
		for (j = 0; j < 0x10; j++) {
			int val = i2c_recv_byte(pair, 0);
			if (val == -1) {
				printf(" nak\n");
				goto stop;
			}
			printf(" %02x", val);
		}
		printf("\n");
		i += 0x10;
	}
stop:
	i2c_stop(pair);
}

static void i2c_scan_bus(struct i2c_pair *pair, int dump)
{
	for (int addr = 0; addr < 0x100; addr += 2) {
		i2c_start(pair);
		if (!i2c_send_byte(pair, addr | 1)) {
			int value = i2c_recv_byte(pair, 1);
			if (!dump) {
				printf(" %02x(%02x)", addr, value);
			} else {
				i2c_stop(pair);
				printf(" [%02x: %02x]\n", addr, value);
				i2c_dump_eep(pair, addr);
			}
		}
		i2c_stop(pair);
	}
	printf("\n");
}

static uint32_t gpio_pupd(uint32_t cfg)
{
	int shift = 0;
	printf("  %08x ", (int)cfg);
	while (shift < 32) {
		if (((cfg >> shift) & 0xf) == 0x4) {
			cfg &= ~(0xf << shift);
			cfg |= 0x8 << shift;
		}
		shift += 4;
	}
	printf("-> %08x", (int)cfg);
	return cfg;
}

static void gpio_mod(struct gpio_regs *regs) {
	printf("%08x cfg:", (int) regs);

	regs->CRH = gpio_pupd(regs->CRH);
	regs->CRL = gpio_pupd(regs->CRL);

	printf(" => %08x%08x\n", (int)regs->CRH, (int)regs->CRL);
}

static void gpio_test(struct gpio_regs *regs, uint16_t *last_in) {
	uint16_t idr = regs->IDR;
	if (regs == GPIOA) {
		idr &= ~0x6200;  // Mask out usart TXD, SWDIO, SWCLK
	}
	if (idr != *last_in) {
		switch ((int)regs) {
		case (int)GPIOA: printf("A "); break;
		case (int)GPIOB: printf("B "); break;
		case (int)GPIOC: printf("C "); break;
		case (int)GPIOD: printf("D "); break;
		default: break;
		}
		printf("%08x: %04x -> %04x (%04x)\n",
			(int)regs, *last_in, idr, *last_in ^ idr);
		*last_in = idr;
	}
}

static struct i2c_pair bus0 = {
	.scl = { GPIOB, 6 },
	.sda = { GPIOB, 7 },
	.delay_us = 4,
};
static struct i2c_pair bus1 = {
	.scl = { GPIOC, 8 },
	.sda = { GPIOC, 7 },
	.delay_us = 4,
};

static struct gpio_pin power_led = { GPIOB, 11};

void _start(void)
{
	int i = 0;
	int last_tick = systicks + 1;
	int last_wfi = uart_wfi;
	int ticks = 16;
	uint32_t odr_val = 0;
	uint16_t last_in[4] = { 0 };
	// Prevent stopping the clock during WFI to prevent openocd issues.
	DEBUGMCU->CR |= 7;

	printf("start %lx %lx\n", x, x0);

	// Enable power LED
	gpio_pin_cfg(&power_led, 0x3); // Push-pull 50MHz
	gpio_pin_setvalue(&power_led, 0); // Enable LED

	gpio_mod(GPIOA);
	gpio_mod(GPIOB);
	gpio_mod(GPIOC);
	gpio_mod(GPIOD);

	i2c_setup(&bus0);
	i2c_setup(&bus1);

	printf("cfg now: A=%08x%08x B=%08x%08x C=%08x%08x D=%08x%08x\n",
		(int)GPIOA->CRH, (int)GPIOA->CRL,
		(int)GPIOB->CRH, (int)GPIOB->CRL,
		(int)GPIOC->CRH, (int)GPIOC->CRL,
		(int)GPIOD->CRH, (int)GPIOD->CRL);

	printf("bus0 scan: ");
	i2c_scan_bus(&bus0, 0);
	printf("bus1 scan: ");
	i2c_scan_bus(&bus1, 0);
	printf("bus0 dump: ");
	i2c_scan_bus(&bus0, 1);
	printf("bus1 dump: ");
	i2c_scan_bus(&bus1, 1);

	while (1) {
		if (last_tick != systicks) {
			if (ticks++ > 16) {
				ticks = 0;
				odr_val ^= 0xffff;
				GPIOA->ODR = odr_val;
				GPIOB->ODR = odr_val;
				GPIOC->ODR = odr_val;
				GPIOD->ODR = odr_val;
			}

			last_tick = systicks;
#if 0
			printf("%d A=%04x B=%04x C=%04x D=%04x\n", i++,
				(int)GPIOA->IDR,
				(int)GPIOB->IDR,
				(int)GPIOC->IDR,
				(int)GPIOD->IDR);
#else
			(void)i;
#endif
			if (last_wfi != uart_wfi) {
				printf("uart_wfi=%d\n", uart_wfi);
				last_wfi = uart_wfi;
			}
		}
		//__WFI();
		//tasklet_run_scheduled();
		gpio_test(GPIOA, &last_in[0]);
		gpio_test(GPIOB, &last_in[1]);
		gpio_test(GPIOC, &last_in[2]);
		gpio_test(GPIOD, &last_in[3]);
	}

	i = x;

	halt();
}
