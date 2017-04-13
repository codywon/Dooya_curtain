#ifndef __APP_CONTROL_H__
#define __APP_CONTROL_H__

#include "app_protocol.h"

#define RX_BUF_LEN		256
#define TX_BUF_LEN		256

enum tag_rx_fsm
{
	CRF_IDLE,
	CRF_FIND_FRM_HDR,
	CRF_PAYLOAD,
	CRF_01,
	CRF_02,
	CRF_03,
	CRF_03_01,
	CRF_03_02,
	CRF_04,
	CRF_04_02,
	CRF_04_03,	
	CRF_DATA_READY
};

extern enum tag_rx_fsm rx_fsm;
extern u8 tx_buf[TX_BUF_LEN];
extern u8 rx_buf[RX_BUF_LEN];
extern u16 rx_data_len;
extern u32 rx_timeout_cntdn;
extern u32 session_timer;

void com_rx_hdl(u8 ch);
void com_rx_start(void);
void com_tx_start(u8 *p_buf, u16 len);
void com_err_reset(void);
void com_err_handle(void);
void com_init(void);

void msg_queue_init(void);
int msg_queue_put(enum tag_message_id msg_id, u16 prop_id, u32 arg);
int msg_queue_get(union tag_message *p_msg, u16 *p_prop_id, u32 *p_arg);

void delay_10ms(u8 time);

u8 f2c(u8 f);
u8 c2f(u8 c);

#endif /* __APP_CONTROL_H__ */
