#include <stddef.h>
#include <stdint.h>

#include "stm32f103.h"
#include "printf.h"

static void halt(void)
{
	for(;;);
}

extern int usb_irqs;
extern int usb_errs;
extern volatile int systicks;

extern void poll_usb(void);
extern void poll_usb_console(void);

static uint32_t x = 0xbadc0ffe;
static uint32_t x0;

void _start(void)
{
	int i = 0;
	int last_tick = systicks + 1;
	// Prevent stopping the clock during WFI to prevent openocd issues.
	DEBUGMCU->CR |= 7;

	printf("start %x %x\n", x, x0);
	while (1) {
		if (last_tick != systicks) {
			last_tick = systicks;
			printf("Hello world! %x %x %x\n",
				i++, usb_irqs, usb_errs);
		}
		__WFI();
		poll_usb();
		poll_usb_console();
	}

	i = x;

	halt();
}
