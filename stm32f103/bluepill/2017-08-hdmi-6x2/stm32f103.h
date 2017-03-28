#ifndef _STM32F103_H_
#define _STM32F103_H_

#define SYS_CLOCK	72000000  /* 72MHz */

#define READREG(r)	((volatile uint32_t)(*(uint32_t *)(r)))
#define WRITEREG(r,v)	(*(volatile uint32_t *)(r) = v)

enum IRQn
{
	NonMaskableInt_IRQn	= -14,
	MemoryManagement_IRQn	= -12,
	BusFault_IRQn		= -11,
	UsageFault_IRQn		= -10,
	SVCall_IRQn		= -5,
	DebugMonitor_IRQn	= -4,
	PendSV_IRQn		= -2,
	SysTick_IRQn		= -1,

	WWDG_IRQn		= 0,
	PVD_IRQn		= 1,
	TAMPER_IRQn		= 2,
	RTC_IRQn		= 3,
	FLASH_IRQn		= 4,
	RCC_IRQn		= 5,
	EXTI0_IRQn		= 6,
	EXTI1_IRQn		= 7,
	EXTI2_IRQn		= 8,
	EXTI3_IRQn		= 9,
	EXTI4_IRQn		= 10,
	DMA1_Channel1_IRQn	= 11,
	DMA1_Channel2_IRQn	= 12,
	DMA1_Channel3_IRQn	= 13,
	DMA1_Channel4_IRQn	= 14,
	DMA1_Channel5_IRQn	= 15,
	DMA1_Channel6_IRQn	= 16,
	DMA1_Channel7_IRQn	= 17,
	ADC1_2_IRQn		= 18,
	USB_HP_CAN1_TX_IRQn	= 19,
	USB_LP_CAN1_RX0_IRQn	= 20,
	CAN1_RX1_IRQn		= 21,
	CAN1_SCE_IRQn		= 22,
	EXTI9_5_IRQn		= 23,
	TIM1_BRK_IRQn		= 24,
	TIM1_UP_IRQn		= 25,
	TIM1_TRG_COM_IRQn	= 26,
	TIM1_CC_IRQn		= 27,
	TIM2_IRQn		= 28,
	TIM3_IRQn		= 29,
	TIM4_IRQn		= 30,
	I2C1_EV_IRQn		= 31,
	I2C1_ER_IRQn		= 32,
	I2C2_EV_IRQn		= 33,
	I2C2_ER_IRQn		= 34,
	SPI1_IRQn		= 35,
	SPI2_IRQn		= 36,
	USART1_IRQn		= 37,
	USART2_IRQn		= 38,
	USART3_IRQn		= 39,
	EXTI15_10_IRQn		= 40,
	RTC_Alarm_IRQn		= 41,
	USBWakeUp_IRQn		= 42,
};

struct scb_regs {
	volatile uint32_t CPUID;
	volatile uint32_t ICSR;
	volatile uint32_t VTOR;
	volatile uint32_t AIRCR;
	volatile uint32_t SCR;
	volatile uint32_t CCR;
	volatile uint32_t SHPR1;
	volatile uint32_t SHPR2;
	volatile uint32_t SHPR3;
	volatile uint32_t SHCRS;
	volatile uint32_t CFSR;
	volatile uint32_t HFSR;
	volatile uint32_t RESERVED1;
	volatile uint32_t MMAR;
	volatile uint32_t BFAR;
	volatile uint32_t AFSR;
};

#define SCB		((struct scb_regs *) 0xE000ED00)

struct debugmcu_regs {
	volatile uint32_t IDCODE;
	volatile uint32_t CR;
};

#define DEBUGMCU	((struct debugmcu_regs *) 0xE0042000)

struct systick_regs {
	volatile uint32_t CSR;
	volatile uint32_t RVR;
	volatile uint32_t CVR;
	volatile uint32_t CALIB;
};

#define SYSTICK		((struct systick_regs *) 0xE000E010)

struct nvic_regs {
	volatile uint32_t ISER[8];
	volatile uint32_t RESERVED0[24];
	volatile uint32_t ICER[8];
	volatile uint32_t RSERVED1[24];
	volatile uint32_t ISPR[8];
	volatile uint32_t RESERVED2[24];
	volatile uint32_t ICPR[8];
	volatile uint32_t RESERVED3[24];
	volatile uint32_t IABR[8];
	volatile uint32_t RESERVED4[56];
	volatile uint8_t  IP[240];
	volatile uint32_t RESERVED5[644];
	volatile uint32_t STIR;
};

#define NVIC	((struct nvic_regs *) 0xE000E100)

#define RCC_AHBENR_DMA1EN		0x00000001
#define RCC_AHBENR_SRAMEN		0x00000004
#define RCC_AHBENR_FLITFEN		0x00000010
#define RCC_AHBENR_CRCEN		0x00000040

#define RCC_APB1ENR_TIM2EN		0x00000001
#define RCC_APB1ENR_TIM3EN		0x00000002
#define RCC_APB1ENR_WWDGEN		0x00000800
#define RCC_APB1ENR_USART2EN		0x00020000
#define RCC_APB1ENR_I2C1EN		0x00200000
#define RCC_APB1ENR_CAN1EN		0x02000000
#define RCC_APB1ENR_BKPEN		0x08000000
#define RCC_APB1ENR_PWREN		0x10000000
#define RCC_APB1ENR_TIM4EN		0x00000004
#define RCC_APB1ENR_SPI2EN		0x00004000
#define RCC_APB1ENR_USART3EN		0x00040000
#define RCC_APB1ENR_I2C2EN		0x00400000
#define RCC_APB1ENR_USBEN		0x00800000

#define RCC_APB2ENR_AFIOEN		0x00000001
#define RCC_APB2ENR_IOPAEN		0x00000004
#define RCC_APB2ENR_IOPBEN		0x00000008
#define RCC_APB2ENR_IOPCEN		0x00000010
#define RCC_APB2ENR_IOPDEN		0x00000020
#define RCC_APB2ENR_IOPEEN		0x00000040
#define RCC_APB2ENR_ADC1EN		0x00000200
#define RCC_APB2ENR_ADC2EN		0x00000400
#define RCC_APB2ENR_TIM1EN		0x00000800
#define RCC_APB2ENR_SPI1EN		0x00001000
#define RCC_APB2ENR_USART1EN		0x00004000

#define RCC_CR_HSION			0x00000001
#define RCC_CR_HSIRDY			0x00000002
#define RCC_CR_HSITRIM			0x000000F8
#define RCC_CR_HSICAL			0x0000FF00
#define RCC_CR_HSEON			0x00010000
#define RCC_CR_HSERDY			0x00020000
#define RCC_CR_HSEBYP			0x00040000
#define RCC_CR_CSSON			0x00080000
#define RCC_CR_PLLON			0x01000000
#define RCC_CR_PLLRDY			0x02000000

#define RCC_CFGR_SW_HSI			0x00000000
#define RCC_CFGR_SW_HSE			0x00000001
#define RCC_CFGR_SW_PLL			0x00000002

#define RCC_CFGR_SWS			0x0000000C
#define RCC_CFGR_SWS_HSI		0x00000000
#define RCC_CFGR_SWS_HSE		0x00000004
#define RCC_CFGR_SWS_PLL		0x00000008

#define RCC_CFGR_HPRE_DIV1		0x00000000
#define RCC_CFGR_HPRE_DIV2		0x00000080
#define RCC_CFGR_HPRE_DIV4		0x00000090
#define RCC_CFGR_HPRE_DIV8		0x000000A0
#define RCC_CFGR_HPRE_DIV16		0x000000B0
#define RCC_CFGR_HPRE_DIV64		0x000000C0
#define RCC_CFGR_HPRE_DIV128		0x000000D0
#define RCC_CFGR_HPRE_DIV256		0x000000E0
#define RCC_CFGR_HPRE_DIV512		0x000000F0

#define RCC_CFGR_PPRE1_DIV1		0x00000000
#define RCC_CFGR_PPRE1_DIV2		0x00000400
#define RCC_CFGR_PPRE1_DIV4		0x00000500
#define RCC_CFGR_PPRE1_DIV8		0x00000600
#define RCC_CFGR_PPRE1_DIV16		0x00000700

#define RCC_CFGR_PPRE2_DIV1		0x00000000
#define RCC_CFGR_PPRE2_DIV2		0x00002000
#define RCC_CFGR_PPRE2_DIV4		0x00002800
#define RCC_CFGR_PPRE2_DIV8		0x00003000
#define RCC_CFGR_PPRE2_DIV16		0x00003800

#define RCC_CFGR_ADCPRE_DIV2		0x00000000
#define RCC_CFGR_ADCPRE_DIV4		0x00004000
#define RCC_CFGR_ADCPRE_DIV6		0x00008000
#define RCC_CFGR_ADCPRE_DIV8		0x0000C000

#define RCC_CFGR_PLLSRC			0x00010000

#define RCC_CFGR_PLLXTPRE_HSE_DIV2	0x00020000

#define RCC_CFGR_PLLMULL2		0x00000000
#define RCC_CFGR_PLLMULL3		0x00040000
#define RCC_CFGR_PLLMULL4		0x00080000
#define RCC_CFGR_PLLMULL5		0x000C0000
#define RCC_CFGR_PLLMULL6		0x00100000
#define RCC_CFGR_PLLMULL7		0x00140000
#define RCC_CFGR_PLLMULL8		0x00180000
#define RCC_CFGR_PLLMULL9		0x001C0000
#define RCC_CFGR_PLLMULL10		0x00200000
#define RCC_CFGR_PLLMULL11		0x00240000
#define RCC_CFGR_PLLMULL12		0x00280000
#define RCC_CFGR_PLLMULL13		0x002C0000
#define RCC_CFGR_PLLMULL14		0x00300000
#define RCC_CFGR_PLLMULL15		0x00340000
#define RCC_CFGR_PLLMULL16		0x00380000
#define RCC_CFGR_USBPRE			0x00400000

#define RCC_CFGR_MCO_NOCLOCK		0x00000000
#define RCC_CFGR_MCO_SYSCLK		0x04000000
#define RCC_CFGR_MCO_HSI		0x05000000
#define RCC_CFGR_MCO_HSE		0x06000000
#define RCC_CFGR_MCO_PLLCLK_DIV2	0x07000000

struct rcc_regs {
	volatile uint32_t CR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t APB2RSTR;
	volatile uint32_t APB1RSTR;
	volatile uint32_t AHBENR;
	volatile uint32_t APB2ENR;
	volatile uint32_t APB1ENR;
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
};

#define RCC	((struct rcc_regs *) 0x40021000)

struct flash_regs {
	volatile uint32_t ACR;
	volatile uint32_t KEYR;
	volatile uint32_t OPTKEYR;
	volatile uint32_t SR;
	volatile uint32_t CR;
	volatile uint32_t AR;
	volatile uint32_t RESERVED;
	volatile uint32_t OBR;
	volatile uint32_t WRPR;
};

#define FLASH	((struct flash_regs *) 0x40022000)

struct afio_regs {
	volatile uint32_t EVCR;
	volatile uint32_t MAPR;
	volatile uint32_t EXTICR[4];
	uint32_t RESERVED0;
	volatile uint32_t MAPR2;
};

#define AFIO	((struct afio_regs *) 0x40010800)

struct gpio_regs {
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
};

#define GPIOA	((struct gpio_regs *) 0x40010800)
#define GPIOB	((struct gpio_regs *) 0x40010c00)
#define GPIOC	((struct gpio_regs *) 0x40011000)

struct dma_channel_regs {
	volatile uint32_t CCR;
	volatile uint32_t CNDTR;
	volatile uint32_t CPAR;
	volatile uint32_t CMAR;
	volatile uint32_t RESERVED1;
} __attribute__((packed));

struct dma_regs {
	volatile uint32_t ISR;
	volatile uint32_t IFCR;
	struct dma_channel_regs CH[7];
} __attribute__((packed));

#define DMA1	((struct dma_regs *) 0x40020000)

#define DMA1_Channel1	((struct dma_channel_regs *) 0x40020008)
#define DMA1_Channel2	((struct dma_channel_regs *) 0x4002001C)
#define DMA1_Channel3	((struct dma_channel_regs *) 0x40020008)
#define DMA1_Channel4	((struct dma_channel_regs *) 0x40020008)
#define DMA1_Channel5	((struct dma_channel_regs *) 0x40020008)
#define DMA1_Channel6	((struct dma_channel_regs *) 0x40020008)
#define DMA1_Channel7	((struct dma_channel_regs *) 0x40020008)

#define USB_CNTR_FRES		0x00000001
#define USB_CNTR_PDWN		0x00000002
#define USB_CNTR_LP_MODE	0x00000004
#define USB_CNTR_FSUSP		0x00000008
#define USB_CNTR_RESUME		0x00000010
#define USB_CNTR_ESOFM		0x00000100
#define USB_CNTR_SOFM		0x00000200
#define USB_CNTR_RESETM		0x00000400
#define USB_CNTR_SUSPM		0x00000800
#define USB_CNTR_WKUPM		0x00001000
#define USB_CNTR_ERRM		0x00002000
#define USB_CNTR_PMAOVRM	0x00004000
#define USB_CNTR_CTRM		0x00008000

#define USB_ISTR_EP_ID		0x0000000F
#define USB_ISTR_DIR		0x00000010
#define USB_ISTR_ESOF		0x00000100
#define USB_ISTR_SOF		0x00000200
#define USB_ISTR_RESET		0x00000400
#define USB_ISTR_SUSP		0x00000800
#define USB_ISTR_WKUP		0x00001000
#define USB_ISTR_ERR		0x00002000
#define USB_ISTR_PMAOVR		0x00004000
#define USB_ISTR_CTR		0x00008000

#define USB_DADDR_EF		0x00000080

#define USB_EPADDR_FIELD	0x0000000F
#define USB_EPTX_STAT		0x00000030
#define USB_EP_DTOG_TX		0x00000040
#define USB_EP_CTR_TX		0x00000080
#define USB_EP_KIND		0x00000100
#define USB_EP_STATUS_OUT	0x00000100
#define USB_EP_T_FIELD		0x00000600
#define USB_EP_SETUP		0x00000800
#define USB_EPRX_STAT		0x00003000
#define USB_EP_DTOG_RX		0x00004000
#define USB_EP_CTR_RX		0x00008000
#define USB_EPREG_MASK		(USB_EP_CTR_RX | USB_EP_SETUP | USB_EP_T_FIELD | USB_EP_KIND | USB_EP_CTR_TX | USB_EPADDR_FIELD)
#define USB_EP_TYPE_MASK	0x00000600
#define USB_EP_BULK		0x00000000
#define USB_EP_CONTROL		0x00000200
#define USB_EP_ISOCHRONOUS	0x00000400
#define USB_EP_INTERRUPT	0x00000600
#define USB_EP_T_MASK		(~USB_EP_T_FIELD & USB_EPREG_MASK)
#define USB_EPKIND_MASK		(~USB_EP_KIND & USB_EPREG_MASK)
#define USB_EP_TX_MASK		(USB_EPTX_STAT | USB_EPREG_MASK)
#define USB_EP_TX_DIS		0x00000000
#define USB_EP_TX_STALL		0x00000010
#define USB_EP_TX_NAK		0x00000020
#define USB_EP_TX_VALID		0x00000030
#define USB_EP_RX_MASK		(USB_EPRX_STAT | USB_EPREG_MASK)
#define USB_EP_RX_DIS		0x00000000
#define USB_EP_RX_STALL		0x00001000
#define USB_EP_RX_NAK		0x00002000
#define USB_EP_RX_VALID		0x00003000

#define USB_EP_TOGGLE_MASK	0x00007070
#define USB_EP_IRQ_MASK		0x00008080
#define USB_EP_WB_MASK		(~(USB_EP_TOGGLE_MASK | USB_EP_IRQ_MASK))

#define USB_RX_COUNT_MASK	0x000003FF
#define USB_RX_BLCOUNT_SHIFT	10
#define USB_RX_BLSIZE		0x00008000
#define USB_RX_SIZE64		((1 << USB_RX_BLCOUNT_SHIFT) | USB_RX_BLSIZE)
#define USB_RX_SIZE128		((2 << USB_RX_BLCOUNT_SHIFT) | USB_RX_BLSIZE)

struct usb_regs {
	volatile uint32_t EP[8];
	volatile uint32_t RESERVED[8];
	volatile uint32_t CNTR;
	volatile uint32_t ISTR;
	volatile uint32_t FNR;
	volatile uint32_t DADDR;
	volatile uint32_t BTABLE;
};

#define USB	((struct usb_regs *) 0x40005C00)
// USB packet memory
#define USB_PM	((volatile uint32_t *) 0x40006000)

struct btable_entry {
	volatile uint32_t TX_ADDR;
	volatile uint32_t TX_COUNT;
	volatile uint32_t RX_ADDR;
	volatile uint32_t RX_COUNT;
};

#define USB_MAX_ENDPOINTS 8

#define USB_BTABLE	((struct btable_entry *) 0x40006000)

#define USART_SR_PE		0x00000001
#define USART_SR_FE		0x00000002
#define USART_SR_NE		0x00000004
#define USART_SR_ORE		0x00000008
#define USART_SR_IDLE		0x00000010
#define USART_SR_RXNE		0x00000020
#define USART_SR_TC		0x00000040
#define USART_SR_TXE		0x00000080
#define USART_SR_LBD		0x00000100
#define USART_SR_CTS		0x00000200

struct usart_sr_bits {
	volatile uint32_t PE;
	volatile uint32_t FE;
	volatile uint32_t NE;
	volatile uint32_t ORE;
	volatile uint32_t IDLE;
	volatile uint32_t RXNE;
	volatile uint32_t TC;
	volatile uint32_t TXE;
	volatile uint32_t LBD;
	volatile uint32_t CTS;
};

#define USART_CR1_SBK		0x00000001
#define USART_CR1_RWU		0x00000002
#define USART_CR1_RE		0x00000004
#define USART_CR1_TE		0x00000008
#define USART_CR1_IDLEIE	0x00000010
#define USART_CR1_RXNEIE	0x00000020
#define USART_CR1_TCIE		0x00000040
#define USART_CR1_TXEIE		0x00000080
#define USART_CR1_PEIE		0x00000100
#define USART_CR1_PS		0x00000200
#define USART_CR1_PCE		0x00000400
#define USART_CR1_WAKE		0x00000800
#define USART_CR1_M		0x00001000
#define USART_CR1_UE		0x00002000

struct usart_regs {
	volatile uint32_t SR;
	volatile uint32_t DR;
	volatile uint32_t BRR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t CR3;
	volatile uint32_t GTPR;
};

#define USART1		((struct usart_regs *) 0x40013800)
#define USART1_SR	((struct usart_sr_bits *) 0x42270000)

static inline void NVIC_EnableIRQ(enum IRQn irq)
{
	NVIC->ISER[irq >> 5] = 1 << (irq & 0x1F);
}

static inline void NVIC_DisableIRQ(enum IRQn irq)
{
	NVIC->ICER[irq >> 5] = 1 << (irq & 0x1F);
}

static inline void NVIC_ClearPendingIRQ(enum IRQn irq)
{
	NVIC->ICPR[irq >> 5] = 1 << (irq & 0x1F);
}

static inline void __disable_irq(void) {
	/* CPSID is "self synchronizing within the instruction stream",
	 * no additional isb needed. */
	asm volatile (
		"cpsid i\n"
		: : : "memory"
	);
}

static inline void __enable_irq(void) {
	asm volatile (
		"cpsie i"
	);
}

static inline void __disable_irq_save(uint32_t *saved) {
	asm volatile (
		"mrs %0, PRIMASK\n"
		"cpsid i\n"
		: "=r" (*saved)
		: : "memory"
	);
}

static inline void __restore_irq_save(uint32_t saved) {
	asm volatile ("msr PRIMASK, %0" : : "r" (saved));
}

static inline void __ISB(void) {
	asm volatile ("isb");
}

static inline void __DSB(void) {
	asm volatile ("dsb");
}

static inline void __DMB(void) {
	asm volatile ("dmb");
}

static inline void __WFI(void) {
	asm volatile ("wfi");
}

#define likely(x) __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)
#define min(a, b) ((a) < (b) ? (a) : (b))

#define mdelay(x) udelay(1000 * x)
static inline void udelay(int n)
{
	// Quick&dirty udelay
	n *= 10;
	while (--n) {
		asm volatile ("nop");
	};
}

#endif
