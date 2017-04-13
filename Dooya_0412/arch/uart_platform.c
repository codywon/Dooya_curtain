/*
 * Copyright 2013 Ayla Networks, Inc.  All rights reserved.
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
#include <stdlib.h>
#include <ayla/mcu_platform.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#include <ayla/uart.h>

#define USART_BaudRate	115200

#define GP_USART	USART2
#define GP_USART_GPIO	GPIOA		/* all but CTS are on GPIOA */
#define GP_USART_GPIO_CTS GPIOD		/* CTS is on PD3 */

#define GP_USART_CTS	GPIO_Pin_3
#define GP_USART_RTS	GPIO_Pin_1
#define GP_USART_TX	GPIO_Pin_2
#define GP_USART_RX	GPIO_Pin_3

/*
 * Set the UART/GPIO settings
 */
void uart_platform_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint32_t apbclock;
	uint32_t br;
	RCC_ClocksTypeDef RCC_ClocksStatus;

	/* Enable the RCC,  USART2, GPIOA, and GPIOD peripheral */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	/* Initialize the RTS and TX */
	GPIO_InitStructure.GPIO_Pin = GP_USART_RTS | GP_USART_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GP_USART_GPIO, &GPIO_InitStructure);

	/* Initialize the RX */
	GPIO_InitStructure.GPIO_Pin = GP_USART_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GP_USART_GPIO, &GPIO_InitStructure);

	/* Initialize the CTS */
	GPIO_InitStructure.GPIO_Pin = GP_USART_CTS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GP_USART_GPIO_CTS, &GPIO_InitStructure);

	/* Set alternate function 7 for USART pins */
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_7);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_7);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

	/* Setting up USART */
	/* Use 1 Stop Bit (no change to CR2 register) */
	/* Set WordLength to 9b, Parity to Odd, enable rx and tx */
	GP_USART->CR1 |= USART_CR1_M | USART_CR1_PCE | USART_CR1_PS |
	    USART_CR1_TE | USART_CR1_RE;
	GP_USART->CR3 |= USART_CR3_CTSE | USART_CR3_RTSE;

	/* Configure the USART Baud Rate */
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;	
	br = apbclock / USART_BaudRate;
	GP_USART->BRR = ((br & 0xf) >> 1) | (br & ~0xf);

	/* Enable the UART */
	GP_USART->CR1 |= USART_CR1_UE;
}

/*
 * Poll for data. If RXNE, receive the packet. If TXE, see if there's another
 * packet to transmit.
 */
void uart_platform_poll(void)
{
	u8 *dr;

	if (GP_USART->ISR & USART_ISR_RXNE) {
		uart_recv(GP_USART->RDR & 0xff);
	}
	if (GP_USART->ISR & USART_ISR_TXE) {
		dr = uart_tx();
		if (dr != NULL) {
			GP_USART->TDR = *dr & 0xff;
		}
	}
}
