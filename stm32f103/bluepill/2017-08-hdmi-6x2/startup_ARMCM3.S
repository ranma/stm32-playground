/* File: startup_ARMCM3.S
 * Purpose: startup file for Cortex-M3 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V2.01
 * Date: 12 June 2014
 *
 */
/* Copyright (c) 2011 - 2014 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

#define __STARTUP_CLEAR_BSS 1

	.syntax	unified
	.arch	armv7-m

	.section .stack
	.align	3
#ifdef __STACK_SIZE
	.equ	Stack_Size, __STACK_SIZE
#else
	.equ	Stack_Size, 0x00000400
#endif
	.globl	__StackTop
	.globl	__StackLimit
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, . - __StackTop

	.section .heap
	.align	3
#ifdef __HEAP_SIZE
	.equ	Heap_Size, __HEAP_SIZE
#else
	.equ	Heap_Size, 0x00000C00
#endif
	.globl	__HeapBase
	.globl	__HeapLimit
__HeapBase:
	.if	Heap_Size
	.space	Heap_Size
	.endif
	.size	__HeapBase, . - __HeapBase
__HeapLimit:
	.size	__HeapLimit, . - __HeapLimit

	.section .vectors
	.align	2
	.globl	__Vectors
__Vectors:
	.long	__StackTop            /* Top of Stack */
	.long	Reset_Handler         /* Reset Handler */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	MemManage_Handler     /* MPU Fault Handler */
	.long	BusFault_Handler      /* Bus Fault Handler */
	.long	UsageFault_Handler    /* Usage Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_Handler           /* SVCall Handler */
	.long	DebugMon_Handler      /* Debug Monitor Handler */
	.long	0                     /* Reserved */
	.long	PendSV_Handler        /* PendSV Handler */
	.long	SysTick_Handler       /* SysTick Handler */

	/* External interrupts */

	.long	WWDG_IRQHandler			/*  0 Window WatchDog                        */
	.long	PVD_IRQHandler			/*  1 PVD through EXTI Line detection        */
	.long	TAMPER_IRQHandler		/*  2 Tamper                                 */
	.long	RTC_IRQHandler			/*  3 RTC global                             */
	.long	FLASH_IRQHandler		/*  4 FLASH global                           */
	.long	RCC_IRQHandler			/*  5 RCC global                             */
	.long	EXTI0_IRQHandler		/*  6 EXTI Line0                             */
	.long	EXTI1_IRQHandler		/*  7 EXTI Line1                             */
	.long	EXTI2_IRQHandler		/*  8 EXTI Line2                             */
	.long	EXTI3_IRQHandler		/*  9 EXTI Line3                             */
	.long	EXTI4_IRQHandler		/*  a EXTI Line4                             */
	.long	DMA1_Channel1_IRQHandler	/*  b DMA1 Channel 1 global                  */
	.long	DMA1_Channel2_IRQHandler	/*  c DMA1 Channel 2 global                  */
	.long	DMA1_Channel3_IRQHandler	/*  d DMA1 Channel 3 global                  */
	.long	DMA1_Channel4_IRQHandler	/*  e DMA1 Channel 4 global                  */
	.long	DMA1_Channel5_IRQHandler	/*  f DMA1 Channel 5 global                  */
	.long	DMA1_Channel6_IRQHandler	/* 10 DMA1 Channel 6 global                  */
	.long	DMA1_Channel7_IRQHandler	/* 11 DMA1 Channel 7 global                  */
	.long	ADC1_2_IRQHandler		/* 12 ADC1 and ADC2 global                   */
	.long	USB_HP_CAN_TX_IRQHandler	/* 13 USB Device High Priority or CAN1 TXs   */
	.long	USB_LP_CAN_RX0_IRQHandler	/* 14 USB Device Low Priority or CAN1 RX0s   */
	.long	CAN_RX1_IRQHandler		/* 15 CAN1 RX1                               */
	.long	CAN_SCE_IRQHandler		/* 16 CAN1 SCE                               */
	.long	EXTI9_5_IRQHandler		/* 17 External Line[9:5]s                    */
	.long	TIM1_BRK_IRQHandler		/* 18 TIM1 Break                             */
	.long	TIM1_UP_IRQHandler		/* 19 TIM1 Update                            */
	.long	TIM1_TRG_COM_IRQHandler		/* 1a TIM1 Trigger and Commutation           */
	.long	TIM1_CC_IRQHandler		/* 1b TIM1 Capture Compare                   */
	.long	TIM2_IRQHandler			/* 1c TIM2 global                            */
	.long	TIM3_IRQHandler			/* 1d TIM3 global                            */
	.long	TIM4_IRQHandler			/* 1e TIM4 global                            */
	.long	I2C1_EV_IRQHandler		/* 1f I2C1 Event                             */
	.long	I2C1_ER_IRQHandler		/* 20 I2C1 Error                             */
	.long	I2C2_EV_IRQHandler		/* 21 I2C2 Event                             */
	.long	I2C2_ER_IRQHandler		/* 22 I2C2 Error                             */
	.long	SPI1_IRQHandler			/* 23 SPI1 global                            */
	.long	SPI2_IRQHandler			/* 24 SPI2 global                            */
	.long	USART1_IRQHandler		/* 25 USART1 global                          */
	.long	USART2_IRQHandler		/* 26 USART2 global                          */
	.long	USART3_IRQHandler		/* 27 USART3 global                          */
	.long	EXTI15_10_IRQHandler		/* 28 External Line[15:10]                   */
	.long	RTCAlarm_IRQHandler		/* 29 RTC Alarm through EXTI Line            */
	.long	USBWakeUp_IRQHandler		/* 2a USB Device WakeUp from suspend through EXTI Line */

	.size	__Vectors, . - __Vectors

	.text
	.thumb
	.thumb_func
	.align	2
	.globl	Reset_Handler
	.globl	startup
	.type	Reset_Handler, %function
startup:
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r4, =__copy_table_start__
	ldr	r5, =__copy_table_end__

.L_loop0:
	cmp	r4, r5
	bge	.L_loop0_done
	ldr	r1, [r4]
	ldr	r2, [r4, #4]
	ldr	r3, [r4, #8]

.L_loop0_0:
	subs	r3, #4
	ittt	ge
	ldrge	r0, [r1, r3]
	strge	r0, [r2, r3]
	bge	.L_loop0_0

	adds	r4, #12
	b	.L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__etext
	ldr	r2, =__data_start__
	ldr	r3, =__data_end__

.L_loop1:
	cmp	r2, r3
	ittt	lt
	ldrlt	r0, [r1], #4
	strlt	r0, [r2], #4
	blt	.L_loop1
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
	ldr	r3, =__zero_table_start__
	ldr	r4, =__zero_table_end__

.L_loop2:
	cmp	r3, r4
	bge	.L_loop2_done
	ldr	r1, [r3]
	ldr	r2, [r3, #4]
	movs	r0, 0

.L_loop2_0:
	subs	r2, #4
	itt	ge
	strge	r0, [r1, r2]
	bge	.L_loop2_0

	adds	r3, #8
	b	.L_loop2
.L_loop2_done:
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =__bss_start__
	ldr	r2, =__bss_end__

	movs	r0, 0
.L_loop3:
	cmp	r1, r2
	itt	lt
	strlt	r0, [r1], #4
	blt	.L_loop3
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

	ldr	r1, =__Vectors
	ldr	r2, =0xE000ED00 /* SCB base */
	str	r1, [r2, 0x08] /* Set VTOR */

	/* Set PSP to the MSP stack */
	mrs	r0, MSP
	msr	PSP, r0

#ifndef __NO_SYSTEM_INIT
	bl	SystemInit
#endif

#ifndef __START
#define __START _start
#endif
	bl	__START

	.pool
	.size	Reset_Handler, . - Reset_Handler

	.align	1
	.thumb_func
	.weak	Default_Handler
	.type	Default_Handler, %function
Default_Handler:
	b	.
	.size	Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

	def_irq_handler	NMI_Handler
	def_irq_handler	HardFault_Handler
	def_irq_handler	MemManage_Handler
	def_irq_handler	BusFault_Handler
	def_irq_handler	UsageFault_Handler
	def_irq_handler	SVC_Handler
	def_irq_handler	DebugMon_Handler
	def_irq_handler	PendSV_Handler
	def_irq_handler	SysTick_Handler

	def_irq_handler	WWDG_IRQHandler
	def_irq_handler	PVD_IRQHandler
	def_irq_handler	TAMPER_IRQHandler
	def_irq_handler	RTC_IRQHandler
	def_irq_handler	FLASH_IRQHandler
	def_irq_handler	RCC_IRQHandler
	def_irq_handler	EXTI0_IRQHandler
	def_irq_handler	EXTI1_IRQHandler
	def_irq_handler	EXTI2_IRQHandler
	def_irq_handler	EXTI3_IRQHandler
	def_irq_handler	EXTI4_IRQHandler
	def_irq_handler	DMA1_Channel1_IRQHandler
	def_irq_handler	DMA1_Channel2_IRQHandler
	def_irq_handler	DMA1_Channel3_IRQHandler
	def_irq_handler	DMA1_Channel4_IRQHandler
	def_irq_handler	DMA1_Channel5_IRQHandler
	def_irq_handler	DMA1_Channel6_IRQHandler
	def_irq_handler	DMA1_Channel7_IRQHandler
	def_irq_handler	ADC1_2_IRQHandler
	def_irq_handler	USB_HP_CAN_TX_IRQHandler
	def_irq_handler	USB_LP_CAN_RX0_IRQHandler
	def_irq_handler	CAN_RX1_IRQHandler
	def_irq_handler	CAN_SCE_IRQHandler
	def_irq_handler	EXTI9_5_IRQHandler
	def_irq_handler	TIM1_BRK_IRQHandler
	def_irq_handler	TIM1_UP_IRQHandler
	def_irq_handler	TIM1_TRG_COM_IRQHandler
	def_irq_handler	TIM1_CC_IRQHandler
	def_irq_handler	TIM2_IRQHandler
	def_irq_handler	TIM3_IRQHandler
	def_irq_handler	TIM4_IRQHandler
	def_irq_handler	I2C1_EV_IRQHandler
	def_irq_handler	I2C1_ER_IRQHandler
	def_irq_handler	I2C2_EV_IRQHandler
	def_irq_handler	I2C2_ER_IRQHandler
	def_irq_handler	SPI1_IRQHandler
	def_irq_handler	SPI2_IRQHandler
	def_irq_handler	USART1_IRQHandler
	def_irq_handler	USART2_IRQHandler
	def_irq_handler	USART3_IRQHandler
	def_irq_handler	EXTI15_10_IRQHandler
	def_irq_handler	RTCAlarm_IRQHandler
	def_irq_handler	USBWakeUp_IRQHandler

	.end
