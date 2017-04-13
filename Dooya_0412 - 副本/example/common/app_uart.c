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
 
#include <stdbool.h>
#include "ayla/mcu_platform.h"
#include "app_control.h"

/*******************************************************************************
* Name  : NVIC_Configuration
* Deion	: Configures NVIC and Vector Table base location.
* Input	: None
* Output: None
* Return: None
*******************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
  
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */
    /* NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); */
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    /* NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); */
#endif
  
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Name	: UART1_GPIO_Configuration
* Deion	: Configures the uart1 GPIO ports.
* Input	: None
* Output: None
* Return: None
*******************************************************************************/
void UART1_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);

	//GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

	/* Configure USART1_Tx as alternate push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;//GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;//GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
}

/*******************************************************************************
* Name  : UART1_Configuration
* Deion	: Configures the uart1 
* Input	: None
* Output: None
* Return: None
*******************************************************************************/
void USART_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
	/*USART_ClockInitTypeDef  USART_ClockInitStructure;*/

	UART1_GPIO_Configuration();

#if 0
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	/* Configure the USART1 synchronous paramters */
	USART_ClockInit(USART1, &USART_ClockInitStructure);
#endif

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =  USART_HardwareFlowControl_None;

	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* Configure USART1 basic and asynchronous paramters */
	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 Receive interrupts */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	NVIC_Configuration();
}

void USART1_IRQHandler(void)
{	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		com_rx_hdl((u8)(USART1->DR));
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_TXE);
	}

	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}
}

