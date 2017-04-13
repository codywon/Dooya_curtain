#include <stdbool.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include "ayla/ayla_proto_mcu.h"
#include "ayla/props.h"
#include "mcu_io.h"
#include "app_property.h"
#include "app_control.h"
#include "sac_property.h"

#define INQUIRE_INTERVAL		300		/* in SysTick */

void custom_init(void)
{
	app_param.hardware_type = 12345;
	memset(&sac_param, 0, sizeof(sac_param));
}

void custom_poll(void)
{
	static u32 inquire_interval;

	static union tag_message msg;
	static u16 prop_id;
	static u32 arg;
	static int frm_len;

	static bool retry = false;
	static u8 retry_cnt = 0;

	enum tag_return_code rc;

	if (((rx_fsm == CRF_IDLE) || (rx_fsm == CRF_DATA_READY)) && (tick > inquire_interval))
	{
		inquire_interval = tick + INQUIRE_INTERVAL;
		//msg_queue_put(IDM_102_000_GET_STATUS_1, NULL, NULL);
	}

	switch(rx_fsm)
	{
		case CRF_FIND_FRM_HDR:
		case CRF_PAYLOAD:
			if (tick > rx_timeout_cntdn)
			{
				com_err_handle();
				rx_fsm = CRF_IDLE;
				retry = true;
				log((0, "----- RX TIME OUT!\r\n"));
			}
			break;			

		case CRF_IDLE:
			if (tick > session_timer)
			{
				if (retry)  
				{
					retry_cnt++;
					log((0, "----- RETRY(%u)\r\n", retry_cnt));
					com_tx_start(tx_buf, frm_len);
					com_rx_start();
				}
				else if (!msg_queue_get(&msg, &prop_id, &arg))
				{
					frm_len = build_frame(tx_buf, sizeof(tx_buf), msg, prop_id, arg);
					
					if (frm_len > 0)
					{
						com_tx_start(tx_buf, frm_len);
						com_rx_start();
					}
					else
					{
						/* Frame build error. Handle it by the customer. */
					}
				}
			}
			break;

		case CRF_DATA_READY:
			com_err_reset();
			rc = parse_frame(rx_buf, rx_data_len);
			switch(rc)
			{
				case EC_SUCCESS:
				//case RC_E_LNK_INVALID_FRM_HDR_TAGS:
				//case RC_E_LNK_TRAN_FAIL:
				//case RC_E_LNK_UNABLE_PROC:
				//case RC_E_LNK_RESERVED:
				//case RC_E_LNK_CHKSUM:
				//case RC_E_LNK_INVALID_FRM_TAIL_TAGS:
				//case RC_E_NET_TRAN_FAIL:
				//case RC_E_NET_INVALID_IP_ADDR_MSB:
				//case RC_E_NET_UNABLE_PROC:
				//case RC_E_NET_RESERVED:
				//case RC_E_TRF_TRAN_FAIL:
				//case RC_E_TRF_ID_MISMATCH:
				//case RC_E_TRF_KEY_MISMATCH:
				//case RC_E_TRF_UNABLE_PROC:
				//case RC_E_TRF_RESERVED:
				case RC_W_APP_OPERATION_FAIL:
				//case RC_E_APP_INVALID_ID:
				//case RC_E_APP_NO_CTRLER_W_THIS_ID:
				//case RC_E_APP_WAIT:
				//case RC_E_APP_INVALID_VALUE:
				//case RC_E_APP_CMD_WO_ARGS:
				//case RC_E_APP_CLOSE_CMD_MAN:
				//case RC_E_APP_INVALID_CMD:
				//case RC_E_APP_INVALID_CMD_FMT:
				//case RC_E_APP_UNDEFINED:
				//case RC_E_APP_NO_MSG_HDL:
				case RC_W_APP_DUMMY_MSG_HDL:
				//case RC_E_APP_LEN_MISMATCH:
					retry = false;
					retry_cnt = 0;
					break;
					
				default:
					retry = true;
					log((0, "----- RESPONSE ERROR!\r\n"));
					break;
			}
			
			rx_fsm = CRF_IDLE;
			break;
	}
}

