#include "ayla/mcu_platform.h"
#include "app_protocol.h"

enum tag_return_code parse_frame(u8 *frm_buf, u16 frm_len)
{
	return EC_SUCCESS;
}

int build_frame(void *tx_buf, u16 tx_buf_len, union tag_message msg, u16 prop_id, u32 p_arg)
{
	return 10;	/* Length of the whole frame */
}

