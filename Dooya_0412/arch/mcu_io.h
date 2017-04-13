/*
 * Copyright 2011-2012 Ayla Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Ayla Networks, Inc.
 */
#ifndef __AYLA_MCU_IO_H__
#define __AYLA_MCU_IO_H__

#define CPU_CLK_HZ		24000000
#define SYSTICK_HZ		100
#define BUTTON_ON_MIN_TICKS  (2 * SYSTICK_HZ)   /* button debounce time */
#define BUTTON_DEBOUNCE_TICKS (20 * SYSTICK_HZ / 1000) /* minimum of 10ms */

#define LED0_PIN		0
#define LED_GPIO		GPIOB

#define BUTTON_GPIO		GPIOA
#define BUTTON_PIN		11
#define BUTTON_EXT_LINE	EXTI_Line0
#define BUTTON_IRQ		EXTI0_IRQn

#define	RS485_DIR_GPIO	GPIOA
#define	RS485_DIR_PIN	8

#define READY_N_GPIO	GPIOA
#define READY_N_PIN		12

#define RESET_N_GPIO	GPIOA
#define RESET_N_PIN		2

#define WKUP_GPIO       GPIOA
#define WKUP_PIN        1

struct intr_stats {
	u32 button;
	u32 intr_line;
};

extern struct intr_stats intr_stats;
extern volatile u32 tick;

void mcu_io_init(void);

/*
 * Return mask for bit number.
 */
static inline u32 bit(u32 i)
{
	return 1U << i;
}

#endif /* __AYLA_MCU_IO_H__ */
