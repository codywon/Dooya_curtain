#include <stdbool.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include "ayla/ayla_proto_mcu.h"
#include "ayla/props.h"
#include "mcu_io.h"
#include "app_uart.h"
#include "app_protocol.h"
#include "app_property.h"
#include "app_control.h"

#define SESSION_TIMER					100		/* in SysTick */
#define	RX_TIMEOUT						500		/* in SysTick */
#define	MSG_QUEUE_LEN					32

#define COM_TIMEOUT_MAX					4

void custom_init(void);
void custom_poll(void);

/*********************************************************************
 *                                                                   *
 * Variables used by the communication between MCU and control board *
 *                                                                   *
 *********************************************************************/
enum tag_rx_fsm rx_fsm;
u8 tx_buf[TX_BUF_LEN];
u8 rx_buf[RX_BUF_LEN];
u16 rx_data_len;
u32 rx_timeout_cntdn;
u32 session_timer;
static u8 rx_prv_55, rx_prv_fe, *p_rx;
static u8 com_timeout_cnt;

/*********************************************************************
 *                                                                   *
 *               Variables used by cloud request queue               *
 *                                                                   *
 *********************************************************************/
static struct tag_msg_queue
{
	bool	empty;
	bool	full;
	u8		head;
	u8		tail;
	struct
	{
		union tag_message	msg;
		u16					prop_id;
		u32 				arg;
	} queue[MSG_QUEUE_LEN];
} msg_que;

void com_rx_hdl(u8 ch)
{
	switch (rx_fsm)
	{
		case CRF_IDLE:
			/* Skip any characters when in idle status. */
			//break;
			
		case CRF_FIND_FRM_HDR:
			/* When start a receiving session, find the frame header first. */
			switch (ch)
			{
				case 0x55:
					/* 0x55 is found, update 0x55 counter. */
					//p_rx = rx_buf; //by guoguang
					rx_prv_55 = 1;
					break;
				
				case 0xFE:
					rx_prv_fe++;
					if(rx_prv_fe == 2)
					{
						if(rx_prv_55 == 1)
						{
							p_rx = rx_buf;
							*(p_rx++) = 0x55;
							*(p_rx++) = 0xFE;
							*(p_rx++) = 0xFE;
							rx_prv_55 = 0;
						}
						rx_fsm = CRF_PAYLOAD;
						rx_prv_fe = 0;
					}
					break;
					
				default:
					/* For other character, just reset the 0x55 counter. */
					rx_prv_55 = 0;
			}
			break;

		case CRF_PAYLOAD:
			if (ch == 0x55)
			{	/* 0x55 found. */
				if (rx_prv_55 == 0)
				{
					/* It's the first 0x55, update 0x55 counter. */
					rx_prv_55 = 1;
				}
				else
				{
					/* It's the second 0xF4, treat them as one 0xF4. */
					rx_prv_55 = 0;
					*(p_rx++) = 0x55;
				}
			}
			else
			{	/* Other character except 0xF4. */
				if (rx_prv_55 != 0)
				{
					/* The previous character is 0xF4, save it first. */
					*(p_rx++) = 0x55;
					
					if (ch == 0xFE)
					{
						/* They are 0xF4, 0xF5, frame header found. Through away current receiving. */
						/* Restart from receiving the payload data. */
						/* Save current character. */
						*(p_rx++) = ch;
						*(p_rx++) = ch;
						p_rx = rx_buf + 3;
					}
				}
				else
				{
					switch(ch)
					{
						case 0x01:
							*(p_rx++) = ch;
							rx_fsm = CRF_01;
						break;
						case 0x02:
							*(p_rx++) = ch;
							rx_fsm = CRF_02;
						break;
						case 0x03:
							*(p_rx++) = ch;
							rx_fsm = CRF_03;
						break;
						case 0x04:
							*(p_rx++) = ch;
							rx_fsm = CRF_04;
						break;
					}
					/* The previous character is not 0xF4, save current character. */
					//*(p_rx++) = ch;
				}				

				/* Reset 0xF4 counter. */
				rx_prv_55 = 0;
			}
			break;
		
		case CRF_01:	
			*(p_rx++) = ch;
			rx_data_len = p_rx - rx_buf;
			if(rx_data_len == 9)
				rx_fsm = CRF_DATA_READY;
			break;
			
		case CRF_02:	
			*(p_rx++) = ch;
			rx_data_len = p_rx - rx_buf;
			if(rx_data_len == 8)
				rx_fsm = CRF_DATA_READY;
			break;


		case CRF_03:	
			*(p_rx++) = ch;
			if((ch==0x04)||(ch==0x05)||(ch==0x06))
			{
				rx_fsm = CRF_03_01;
			}else
			{
				rx_fsm = CRF_03_02;
			}
			break;
		
		case CRF_03_01:	
			*(p_rx++) = ch;
			rx_data_len = p_rx - rx_buf;
			if(rx_data_len == 8)
				rx_fsm = CRF_DATA_READY;
			break;
			
		case CRF_03_02:	
			*(p_rx++) = ch;
			rx_data_len = p_rx - rx_buf;
			if(rx_data_len == 7)
			{
				//p_rx = rx_buf;
			  //rx_fsm = CRF_FIND_FRM_HDR;
				rx_fsm = CRF_DATA_READY;
			}	
			break;
		
		
		case CRF_04:	
			*(p_rx++) = ch;
			if(ch == 0x02)
			{
				rx_fsm = CRF_04_02;
			}else
			{
				rx_fsm = CRF_04_03;
			}
			break;
			
		case CRF_04_02:
			*(p_rx++) = ch;
			rx_data_len = p_rx - rx_buf;
			if(rx_data_len == 16)
			{
				rx_fsm = CRF_DATA_READY;
			}
			break;
		
		case CRF_04_03:
			*(p_rx++) = ch;
			rx_data_len = p_rx - rx_buf;
			if(rx_data_len == 8)
			{
				rx_fsm = CRF_DATA_READY;
			}
			break;
			
		case CRF_DATA_READY:
			/* Skip any characters when in idle status. */
			break;
	}	
	
	/* Reset timeout counter on each incoming character. */
	rx_timeout_cntdn = tick + RX_TIMEOUT;
}

void com_rx_start(void)
{
	p_rx				= rx_buf;
	rx_timeout_cntdn	= tick + RX_TIMEOUT;
	rx_prv_55			= 0;

	rx_fsm				= CRF_FIND_FRM_HDR;
}

void com_tx_start(u8 *p_buf, u16 len)
{
	u8 ch;
	u32 i;
	
	session_timer = tick + SESSION_TIMER;

	/* Set the RS485 in TX mode. */
	GPIO_SetBits(GPIOA, GPIO_Pin_8);

	/* Send the frame header */
	i = 2;
	while (i--)
	{
		USART_SendData(USART1, *(p_buf++));
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
	}

	/* Send the payload data */
	i = len - 4;
	while (i--)
	{
		ch = *(p_buf++);
		if (ch == 0xF4)
		{
			USART_SendData(USART1, ch);
			while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
		}

		USART_SendData(USART1, ch);
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
	}

	/* Send the frame tail */
	i = 2;
	while (i--)
	{
		USART_SendData(USART1, *(p_buf++));
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
	}

	/* Must add a delay to avoid communication error. */
	for (i = 10000; i--; );

	/* Set the RS485 in RX mode. */
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

void com_err_reset(void)
{
	com_timeout_cnt = 0;
}

void com_err_handle(void)
{
	if (++com_timeout_cnt > COM_TIMEOUT_MAX)
	{
		com_timeout_cnt = 0;
		if (app_param.e_com_error == 0)
		{
			app_param.e_com_error = 1;
			prop_lookup(PN_COM_ERR)->send_mask = valid_dest_mask;
		}
	}
}

void com_init(void)
{
	session_timer = tick + SESSION_TIMER;
	rx_fsm = CRF_IDLE;
	com_err_reset();
	USART_Configuration();
}

void msg_queue_init(void)
{
	msg_que.empty	= true;
	msg_que.full	= false;
	msg_que.head	= 0;
	msg_que.tail	= 0;
	memset(msg_que.queue, 0, sizeof(msg_que.queue));
}

int msg_queue_put(enum tag_message_id msg_id, u16 prop_id, u32 arg)
{
	u8 idx;

	if (msg_que.full)
	{
		log((0, "----- MSG QUE FULL!\r\n"));
		return -1;
	}
	
	idx = msg_que.head;
	msg_que.queue[idx].msg.id	= msg_id;
	msg_que.queue[idx].prop_id	= prop_id;
	msg_que.queue[idx].arg		= arg;

	idx++;
	if (idx >= sizeof(msg_que.queue)/sizeof(msg_que.queue[0]))
		idx = 0;
	if (idx == msg_que.tail)
		msg_que.full = true;
	msg_que.head = idx;

	msg_que.empty = false;
	return 0;
}

int msg_queue_get(union tag_message *p_msg, u16 *p_prop_id, u32 *p_arg)
{
	u8 idx;

	if (msg_que.empty)
		return -1;

	idx	= msg_que.tail;
	*p_msg		= msg_que.queue[idx].msg;
	*p_prop_id	= msg_que.queue[idx].prop_id;
	*p_arg		= msg_que.queue[idx].arg;

	idx++;
	if (idx >= sizeof(msg_que.queue)/sizeof(msg_que.queue[0]))
		idx = 0;
	if (idx == msg_que.head)
		msg_que.empty = true;
	msg_que.tail = idx;

	msg_que.full = false;
	if (p_msg->id != IDM_102_000_GET_STATUS_1)
		log((0, "Msg=%04x Prop=%02u Arg=%u\r\n", *p_msg, *p_prop_id, *p_arg));
	return 0;
}

void app_init(void)
{
	app_param.beep = 1;

	com_init();
	msg_queue_init();

	custom_init();
}

void app_poll(void)
{
	custom_poll();
}

void delay_10ms(u8 time)
{
	u32 i;
	i = tick + time;
	while (tick < i);
}

