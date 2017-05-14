#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>

#include "stm32f103.h"
#include "usb.h"
#include "jtag.h"

#define GPIO_CONFIG_TX		0xa	// Alternate function push-pull 2MHz
#define GPIO_CONFIG_RX		0x4	// Floating input

struct ring_info {
	char *buf;
	uint32_t *rptr;
	uint32_t *wptr;
	uint32_t mask;
};

int ring_empty(struct ring_info *ring) {
	uint32_t saved, empty;
	__disable_irq_save(&saved);
	empty = *ring->rptr == *ring->wptr;
	__restore_irq_save(saved);
	return empty;
}

int ring_full(struct ring_info *ring) {
	uint32_t saved, full;
	__disable_irq_save(&saved);
	full = *ring->rptr == *ring->wptr + 1;
	__restore_irq_save(saved);
	return full;
}

int ring_try_getc(struct ring_info *ring, char *c) {
	uint32_t saved, empty;
	__disable_irq_save(&saved);
	empty = *ring->rptr == *ring->wptr;
	if (!empty) {
		*c = ring->buf[*ring->rptr];
		*ring->rptr = (*ring->rptr + 1) & ring->mask;
	}
	__restore_irq_save(saved);
	return !empty;
}

int ring_try_putc(struct ring_info *ring, const char c) {
	uint32_t saved, full;
	__disable_irq_save(&saved);
	full = *ring->rptr == *ring->wptr + 1;
	if (!full) {
		ring->buf[*ring->wptr] = c;
		*ring->wptr = (*ring->wptr + 1) & ring->mask;
	}
	__restore_irq_save(saved);
	return !full;
}

char uart3_tx_ring[64];
uint32_t uart3_tx_rptr, uart3_tx_wptr;
struct ring_info uart3_tx_info = {
	.buf = uart3_tx_ring,
	.rptr = &uart3_tx_rptr,
	.wptr = &uart3_tx_wptr,
	.mask = sizeof(uart3_tx_ring) - 1,
};

char uart3_rx_ring[64];
uint32_t uart3_rx_rptr, uart3_rx_wptr;
struct ring_info uart3_rx_info = {
	.buf = uart3_rx_ring,
	.rptr = &uart3_rx_rptr,
	.wptr = &uart3_rx_wptr,
	.mask = sizeof(uart3_rx_ring) - 1,
};

char uart_tx_ring[1024];
int empty;
uint32_t uart_tx_rptr, uart_tx_wptr;
uint32_t usb_tx_rptr;
#define UART_TX_PTR_MASK (sizeof(uart_tx_ring) - 1)

void uart_init(void)
{
	uart_tx_rptr = uart_tx_wptr = usb_tx_rptr = 0;
	empty = 1;

	// Enable alternate function IO, GPIOA and USART1
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// Configure TX pin on PA9
	GPIOA->CRH &= ~(0xF << 4);
	GPIOA->CRH |= GPIO_CONFIG_TX << 4;

	// Configure RX pin on PA10
	GPIOA->CRH &= ~(0xF << 8);
	GPIOA->CRH |= GPIO_CONFIG_RX << 8;

	// Setup UART1
	USART1->CR1 = USART_CR1_UE|USART_CR1_RXNEIE|USART_CR1_TE|USART_CR1_RE;
	USART1->CR2 = 0x0000;
	USART1->CR3 = 0x0000;
	USART1->BRR = SYS_CLOCK / 115200;
	USART1->GTPR = 0x0000;

	// Enable USART1 interrupts
	NVIC_EnableIRQ(USART1_IRQn);

	// Enable alternate function IO, GPIOB and USART3
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	// Configure TX pin on PB10
	GPIOB->CRH &= ~(0xF << 8);
	GPIOB->CRH |= GPIO_CONFIG_TX << 8;

	// Configure RX pin on PB11
	GPIOB->CRH &= ~(0xF << 12);
	GPIOB->CRH |= GPIO_CONFIG_RX << 12;

	// Setup UART1
	USART3->CR1 = USART_CR1_UE|USART_CR1_RXNEIE|USART_CR1_TE|USART_CR1_RE;
	USART3->CR2 = 0x0000;
	USART3->CR3 = 0x0000;
	USART3->BRR = SYS_CLOCK / 2 / 115200;
	USART3->GTPR = 0x0000;

	// Enable USART3 interrupts
	NVIC_EnableIRQ(USART3_IRQn);
}

static int uart_try_putc(int ch)
{
	uint32_t saved;
	uint32_t retval = 1;

	__disable_irq_save(&saved);

	if (likely(((uart_tx_wptr + 1) & UART_TX_PTR_MASK) != uart_tx_rptr)) {
		uart_tx_ring[uart_tx_wptr++] = ch;
		uart_tx_wptr &= UART_TX_PTR_MASK;
		if (uart_tx_wptr == usb_tx_rptr) {
			usb_tx_rptr += 1;
			usb_tx_rptr &= UART_TX_PTR_MASK;
		}
	} else {
		retval = 0;
	}

	if (unlikely(empty && uart_tx_wptr != uart_tx_rptr)) {
		uart_tx_rptr += 1;
		uart_tx_rptr &= UART_TX_PTR_MASK;
		USART1->DR = ch;
		USART1->CR1 |= USART_CR1_TXEIE;
		empty = 0;
	}

	__restore_irq_save(saved);
	return retval;
}

void USART1_IRQHandler(void)
{
	uint32_t sr = USART1->SR;

	if (sr & USART_SR_TXE) {
		if (uart_tx_rptr != uart_tx_wptr) {
			USART1->DR = uart_tx_ring[uart_tx_rptr++];
			uart_tx_rptr &= UART_TX_PTR_MASK;
		} else {
			// Disable USART TX empty irq
			USART1->CR1 &= ~USART_CR1_TXEIE;
			empty = 1;
		}
	}
	if (sr & USART_SR_RXNE) {
		char c = USART1->DR;
		uart_try_putc(c);
	}
}

volatile int usb_ring_full;
volatile int usb_overrun;
volatile int usb_noise;
volatile int usb_puts, usb_gets;

void USART3_IRQHandler(void)
{
	uint32_t sr = USART3->SR;

	if (sr & USART_SR_RXNE) {
		char c = USART3->DR;
		if (!ring_try_putc(&uart3_rx_info, c)) {
			usb_ring_full++;
		} else {
			usb_puts++;
		}
	}
	if (sr & USART_SR_TXE) {
		char c;
		if (ring_try_getc(&uart3_tx_info, &c)) {
			USART3->DR = c;
		} else {
			// Disable USART TX empty irq
			USART3->CR1 &= ~USART_CR1_TXEIE;
		}
	}
	if (sr & USART_SR_ORE) {
		usb_overrun++;
	}
	if (sr & USART_SR_NE) {
		usb_noise++;
	}
}

int usb_try_getc(char *c)
{
	int ret = ring_try_getc(&uart3_rx_info, c);
	if (ret) {
		usb_gets++;
	}
	return ret;
}

int usb_try_putc(char c)
{
	int ret = ring_try_putc(&uart3_tx_info, c);
	if (ret) {
		USART3->CR1 |= USART_CR1_TXEIE;
	} else {
		usb_ring_full++;
	}
	return ret;
}

volatile int uart_wfi;

void board_putc(int ch)
{
	while (!uart_try_putc(ch)) {
		__WFI();
		uart_wfi++;
	}
}

static void board_init(void)
{
	FLASH->ACR = 0x12;	// 2 wait states, prefetch enabled

	// Start the HSE (external crystal)
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY));

	// Configure bus clock dividers and PLL
	RCC->CFGR = RCC_CFGR_HPRE_DIV1
		| RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_PPRE2_DIV1
		| RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLSRC
		| RCC_CFGR_PLLMULL9;

	// Start the PLL
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));

	// Switch to the PLL
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

volatile int systicks;

void SysTick_Handler(void)
{
	systicks++;
}

static void fatal_putchar(char ch)
{
	while (!(USART1->SR & USART_SR_TXE));
	USART1->DR = ch;
}

static void fatal_puts(const char *msg)
{
	USART1->CR1 &= ~USART_CR1_TXEIE;
	do {
		fatal_putchar(*(msg++));
	} while (*msg);
	fatal_putchar('\r');
	fatal_putchar('\n');
	while (1);
}

void HardFault_Handler(void)
{
	fatal_puts("hardfault");
}

void BusFault_Handler(void)
{
	fatal_puts("busfault");
}

static void systick_init(void)
{
	systicks = 0;
	// Set up systick for 1Hz
	SYSTICK->CVR = SYSTICK->RVR = SYS_CLOCK / 8;
	// Enable systick and interrupt, external clock.
	SYSTICK->CSR = 3;
}

static void dma_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
}

void SystemInit(void)
{
	board_init();
	systick_init();
	dma_init();
	uart_init();
	jtag_init();
	usb_init();
}
