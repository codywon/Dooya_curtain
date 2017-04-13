#include <stdbool.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include <ayla/ayla_proto_mcu.h>
#include "ayla/props.h"
#include "app_protocol_i.h"
#include "app_property.h"
#include "app_control.h"
#include "sac_protocol_i.h"
#include "sac_property.h"

static void	*msg_003_xxx_build_power_ctrl(struct tag_app_layer_hdr *p_app_hdr, u16 prop_id,	u32	arg)
{
	p_app_hdr->msg_sub_type	= arg;
	return p_app_hdr + 1;
}

static void	*msg_101_000_build_write_command_1(struct tag_app_layer_hdr	*p_app_hdr,	u16	prop_id, u32 arg)
{
	struct tag_msg_101_000_build *p_data;

	p_data = (struct tag_msg_101_000_build *)(p_app_hdr	+ 1);

	switch (prop_id)
	{
		/* Control board <-> MCU <-> Cloud */
		//case PID_FAN_SPEED		: SEND_TO_ECB(fan_speed		);	break;
		//case PID_WORK_MODE		: SEND_TO_ECB(work_mode		);	break;
		//case PID_TEMP			: SEND_TO_ECB_F(temp, sac_param.temptype);	break;
		//case PID_FAN_POWER		: SEND_TO_ECB(fan_power		);	break;
		//case PID_FAN_LEFTRIGHT	: SEND_TO_ECB(fan_leftright	);	break;
		//case PID_POWER			: SEND_TO_ECB(power			);	break;
		//case PID_SLEEP			: SEND_TO_ECB(sleep			);	break;
		//case PID_TEMPTYPE		: SEND_TO_ECB(temptype		);	break;
		//case PID_FAN_MUTE		: SEND_TO_ECB(fan_mute		);	break;
		//case PID_ECO			: SEND_TO_ECB(eco			);	break;
		//case PID_RUN_MODE		: SEND_TO_ECB(run_mode		);	break;
		//case PID_TEMP_HEATCOLD	: SEND_TO_ECB(temp_heatcold	);	break;
		//case PID_BACKLIGHT		: SEND_TO_ECB(backlight		);	break;

		//case PID_BEEP			: app_param.beep = arg;			break;
	}

	/*if (prop_id	== PID_BEEP)
	{
		p_data->auto_manual	= app_param.beep;
	}
	else if	(prop_table[prop_id].beep)
	{
		prop_table[prop_id].beep--;
		p_data->auto_manual	= 0;
	}
	else
	{
		p_data->auto_manual	= app_param.beep;
	}*/

	return p_data +	1;
}

static void	*msg_102_000_build_get_status_1(struct tag_app_layer_hdr *p_app_hdr, u16 prop_id, u32 arg)
{
	struct tag_msg_102_000_build *p_data;

	p_data = (struct tag_msg_102_000_build *)(p_app_hdr	+ 1);
	p_data->auto_manual	= 0;

	return p_data +	1;
}

static enum tag_return_code msg_102_000_parse_get_status_1(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct tag_msg_102_000_parse *p_data;

	p_data = (struct tag_msg_102_000_parse *)(p_app_hdr	+ 1);
	if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		return RC_E_APP_LEN_MISMATCH;

	/* Control board <-> MCU <-> Cloud */
	//SEND_TO_CLOUD(sac_param,	status,		PID_STATUS		);
	//SEND_TO_CLOUD(sac_param,	work_mode,		PID_WORK_MODE		);
	//SEND_TO_CLOUD(sac_param,	fan_speed,		PID_FAN_SPEED		);
	//SEND_TO_CLOUD_F(sac_param,	temp,			PID_TEMP,			temptype);
	//SEND_TO_CLOUD(sac_param,	fan_leftright,	PID_FAN_LEFTRIGHT	);
	//SEND_TO_CLOUD(sac_param,	fan_power,		PID_FAN_POWER		);
	//SEND_TO_CLOUD(sac_param,	power,			PID_POWER			);
	//SEND_TO_CLOUD(sac_param,	sleep,			PID_SLEEP			);
	//SEND_TO_CLOUD(sac_param,	fan_mute,		PID_FAN_MUTE		);
	//SEND_TO_CLOUD(sac_param,	eco,			PID_ECO			);
	//SEND_TO_CLOUD(sac_param,	run_mode,		PID_RUN_MODE		);
	//SEND_TO_CLOUD(sac_param,	temp_heatcold,	PID_TEMP_HEATCOLD	);
	//SEND_TO_CLOUD(sac_param,	backlight,		PID_BACKLIGHT		);

	/* Control board --> MCU --> CLOUD */
	//SEND_TO_CLOUD(sac_param,	humidity,		PID_HUMIDITY		);
	//SEND_TO_CLOUD_F(sac_param,	temp_in,		PID_TEMP_IN,		temptype);

	return EC_SUCCESS;
}

/*-------------------------------------------------------------------------------------------------*/

enum tag_return_code dooya_get_status(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct dooya_operation_4 *p_data; //by guoguang

	p_data = (struct dooya_operation_4 *)(p_app_hdr);
	//if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		//return RC_E_APP_LEN_MISMATCH;

	/* Control board <-> MCU <-> Cloud */
	SEND_TO_CLOUD(sac_param,	status,		PID_STATUS		);
	SEND_TO_CLOUD(sac_param,	current_present,		PID_CURRENT_PRESENT		);
	SEND_TO_CLOUD(sac_param,	direction,		PID_DIRECTION		);
	SEND_TO_CLOUD(sac_param,	hand_pull,		PID_HAND_PULL		);
	SEND_TO_CLOUD(sac_param,	route_enable,		PID_ROUTE_ENABLE		);
	
	return EC_SUCCESS;
}

enum tag_return_code dooya_get_error(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct dooya_error *p_data; //by guoguang

	p_data = (struct dooya_error *)(p_app_hdr);
	//if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		//return RC_E_APP_LEN_MISMATCH;

	/* Control board <-> MCU <-> Cloud */
	SEND_TO_CLOUD(sac_param,	error,		PID_ERROR		);
	
	return EC_SUCCESS;
}

/*-------------------------------------------------------------------------------------------------*/

static void	*msg_102_001_build_get_status_2(struct tag_app_layer_hdr *p_app_hdr, u16 prop_id, u32 arg)
{
	return p_app_hdr + 1;
}

static enum tag_return_code msg_102_001_parse_get_status_2(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct tag_msg_102_001_parse *p_data;

	p_data = (struct tag_msg_102_001_parse *)(p_app_hdr	+ 1);
	if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		return RC_E_APP_LEN_MISMATCH;

	if (p_data)
	{
		/* TODO: */
	}

	return EC_SUCCESS;
}

static const struct	tag_msg_sub_switch msg_003_sw_tbl[]	=
{
	{	STPM_xxx_xxx_DUMMY,			msg_003_xxx_build_power_ctrl,					NULL								}
};

static const struct	tag_msg_sub_switch msg_101_sw_tbl[]	=
{
	{	STPM_101_000_SET_STATUS_1,	msg_101_000_build_write_command_1,				NULL								}
};

static const struct	tag_msg_sub_switch msg_102_sw_tbl[]	=
{
	{	STPM_102_000_GET_STATUS_1,	msg_102_000_build_get_status_1,					msg_102_000_parse_get_status_1		},
	{	STPM_102_001_GET_STATUS_2,	msg_102_001_build_get_status_2,					msg_102_001_parse_get_status_2		}
};

static const struct	tag_msg_switch msg_sw_tbl[]	=
{
	{	TPM_003_PWR_CTRL,			(struct	tag_msg_sub_switch *)msg_003_sw_tbl,	sizeof(msg_003_sw_tbl)/sizeof(msg_003_sw_tbl[0])	},
	{	TPM_101_SET_STATUS,			(struct	tag_msg_sub_switch *)msg_101_sw_tbl,	sizeof(msg_101_sw_tbl)/sizeof(msg_101_sw_tbl[0])	},
	{	TPM_102_GET_STATUS,			(struct	tag_msg_sub_switch *)msg_102_sw_tbl,	sizeof(msg_102_sw_tbl)/sizeof(msg_102_sw_tbl[0])	}
};

const struct tag_msg_switch *p_msg_sw_tbl = msg_sw_tbl;
const u8 msg_sw_tbl_item_num = sizeof(msg_sw_tbl)/sizeof(msg_sw_tbl[0]);

