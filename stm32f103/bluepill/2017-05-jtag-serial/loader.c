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

extern void poll_usb_console(void);

static uint32_t x = 0xbadc0ffe;
static uint32_t x0;
extern int uart_wfi;
extern int usb_ring_full;
extern int usb_overrun;
extern int usb_noise;
extern int usb_puts, usb_gets, usb_busy;

void _start(void)
{
	int i = 0;
	int last_tick = systicks + 1;
	int last_wfi = uart_wfi;
	int last_usb_full = usb_ring_full;
	int last_usb_overrun = usb_overrun;
	int last_usb_noise = usb_noise;
	// Prevent stopping the clock during WFI to prevent openocd issues.
	DEBUGMCU->CR |= 7;

	printf("start %lx %lx\n", x, x0);
	while (1) {
		if (last_tick != systicks) {
			last_tick = systicks;
#if 0
			printf("Hello world! %x %x %x\n",
				i++, usb_irqs, usb_errs);
#else
			(void)i;
#endif
			if (last_wfi != uart_wfi) {
				printf("uart_wfi=%d\n", uart_wfi);
				last_wfi = uart_wfi;
			}
			if (last_usb_full != usb_ring_full) {
				printf("usb_ring_full=%d\n", usb_ring_full);
				last_usb_full = usb_ring_full;
			}
			if (last_usb_overrun != usb_overrun) {
				printf("usb_overrun=%d\n", usb_overrun);
				last_usb_overrun = usb_overrun;
			}
			if (last_usb_noise != usb_noise) {
				printf("usb_noise=%d\n", usb_noise);
				last_usb_noise = usb_noise;
			}
			if (usb_busy) {
				printf("usb_busy\n");
			}
		}
		__WFI();
		tasklet_run_scheduled();
		poll_usb_console();
	}

	i = x;

	halt();
}
