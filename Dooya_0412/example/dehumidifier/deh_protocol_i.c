#include <stdbool.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include <ayla/ayla_proto_mcu.h>
#include "ayla/props.h"
#include "app_protocol_i.h"
#include "app_property.h"
#include "app_control.h"
#include "deh_protocol_i.h"
#include "deh_property.h"

static void	*msg_101_000_build_write_command_1(struct tag_app_layer_hdr	*p_app_hdr,	u16	prop_id, u32 arg)
{
	struct tag_msg_101_000_build *p_data;

	p_data = (struct tag_msg_101_000_build *)(p_app_hdr	+ 1);

	switch (prop_id)
	{
		/* Control board <-> MCU <-> Cloud */
		case PID_POWER			: SEND_TO_ECB(power			);	break;
		case PID_WORK_MODE		: SEND_TO_ECB(work_mode		);	break;
		case PID_FAN_SPEED		: SEND_TO_ECB(fan_speed		);	break;
		case PID_HUMIDITY		: SEND_TO_ECB(humidity		);	break;
		case PID_E_HEAT_TEMP	: SEND_TO_ECB_F(e_heat_temp, deh_param.temptype);	break;
		case PID_E_HEAT			: SEND_TO_ECB(e_heat		);	break;
		case PID_NEGATIVE_ION	: SEND_TO_ECB(negative_ion	);	break;
		case PID_WATER_PUMP		: SEND_TO_ECB(water_pump	);	break;
		case PID_TEMPTYPE		: SEND_TO_ECB(temptype		);	break;

		case PID_BEEP			: app_param.beep = arg;			break;
	}

	if (prop_id	== PID_BEEP)
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
	}

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
	SEND_TO_CLOUD(deh_param,	temptype,		PID_TEMPTYPE		);
	SEND_TO_CLOUD(deh_param,	power,			PID_POWER			);
	SEND_TO_CLOUD(deh_param,	work_mode,		PID_WORK_MODE		);
	SEND_TO_CLOUD(deh_param,	fan_speed,		PID_FAN_SPEED		);
	SEND_TO_CLOUD(deh_param,	humidity,		PID_HUMIDITY		);
	SEND_TO_CLOUD_F(deh_param,	e_heat_temp,	PID_E_HEAT_TEMP,	temptype);
	SEND_TO_CLOUD(deh_param,	e_heat,			PID_E_HEAT			);
	SEND_TO_CLOUD(deh_param,	negative_ion,	PID_NEGATIVE_ION	);
	SEND_TO_CLOUD(deh_param,	water_pump,		PID_WATER_PUMP		);

	/* Control board --> MCU --> CLOUD */
	SEND_TO_CLOUD(deh_param,	humidity_indoor,PID_HUMIDITY_INDOOR	);
	SEND_TO_CLOUD_F(deh_param,	temp_indoor,	PID_TEMP_INDOOR,	temptype);

	return EC_SUCCESS;
}

static const struct	tag_msg_sub_switch msg_101_sw_tbl[]	=
{
	{	STPM_101_000_SET_STATUS_1,	msg_101_000_build_write_command_1,				NULL								}
};

static const struct	tag_msg_sub_switch msg_102_sw_tbl[]	=
{
	{	STPM_102_000_GET_STATUS_1,	msg_102_000_build_get_status_1,					msg_102_000_parse_get_status_1		}
};

static const struct	tag_msg_switch msg_sw_tbl[]	=
{
	{	TPM_101_SET_STATUS,			(struct	tag_msg_sub_switch *)msg_101_sw_tbl,	sizeof(msg_101_sw_tbl)/sizeof(msg_101_sw_tbl[0])	},
	{	TPM_102_GET_STATUS,			(struct	tag_msg_sub_switch *)msg_102_sw_tbl,	sizeof(msg_102_sw_tbl)/sizeof(msg_102_sw_tbl[0])	}
};

const struct tag_msg_switch *p_msg_sw_tbl = msg_sw_tbl;
const u8 msg_sw_tbl_item_num = sizeof(msg_sw_tbl)/sizeof(msg_sw_tbl[0]);

