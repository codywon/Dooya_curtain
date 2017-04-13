#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include "ayla/ayla_proto_mcu.h"
#include "ayla/props.h"
#include "ayla/schedeval.h"
#include "app_property.h"
#include "app_control.h"
#include "sac_property.h"

struct tag_sac_param sac_param;

void sac_param_set_status(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_STATUS, i);
}

#define	sac_param_send_status		prop_send_generic

void sac_param_set_delete_route(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_DELETE_ROUTE, i);
}

#define	sac_param_send_delete_route			prop_send_generic

void sac_param_set_reset_factory(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_RESET_FACTORY, i);
}

#define	sac_param_send_reset_factory			prop_send_generic

void sac_param_set_present(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_PRESENT, i);
}

#define	sac_param_send_present		prop_send_generic

void sac_param_set_direction(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_DIRECTION, i);
}

#define	sac_param_send_direction			prop_send_generic

void sac_param_set_hand_pull(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_HAND_PULL, i);
}

#define	sac_param_send_hand_pull			prop_send_generic

void sac_param_set_up_route(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_UP_ROUTE, i);
}

#define	sac_param_send_up_route  		prop_send_generic

void sac_param_set_down_route(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_DOWN_ROUTE, i);
}

#define	sac_param_send_down_route			prop_send_generic



/*void sac_param_set_fan_speed(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_SPEED, i);
}

#define	sac_param_send_fan_speed		prop_send_generic

void sac_param_set_work_mode(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_WORK_MODE, i);
}

#define	sac_param_send_work_mode		prop_send_generic

void sac_param_set_temp(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_TEMP, i);
}

#define	sac_param_send_temp				prop_send_generic

void sac_param_set_fan_power(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_POWER, i);
}

#define	sac_param_send_fan_power		prop_send_generic

void sac_param_set_fan_leftright(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_LEFTRIGHT, i);
}

#define	sac_param_send_fan_leftright	prop_send_generic

void sac_param_set_power(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_003_000_POWER_CTRL, PID_POWER, i);
}

#define	sac_param_send_power			prop_send_generic

void sac_param_set_sleep(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_SLEEP, i);
}

#define	sac_param_send_sleep			prop_send_generic

void sac_param_set_temptype(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_TEMPTYPE, i);
}

#define	sac_param_send_temptype			prop_send_generic

void sac_param_set_fan_mute(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_MUTE, i);
}

#define	sac_param_send_fan_mute			prop_send_generic

void sac_param_set_eco(struct prop *prop, void *arg, void *valp, size_t	len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_ECO, i);
}

#define	sac_param_send_eco				prop_send_generic

void sac_param_set_run_mode(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_RUN_MODE, i);
}

#define	sac_param_send_run_mode			prop_send_generic

void sac_param_set_temp_heatcold(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_TEMP_HEATCOLD, i);
}

#define	sac_param_send_temp_heatcold	prop_send_generic

void sac_param_set_backlight(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_BACKLIGHT, i);
}

#define	sac_param_send_backlight		prop_send_generic

void sac_param_set_custom_scene(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	if (len	>= sizeof(sac_param.custom_scene) -	1) {
		return;
	}
	memcpy(sac_param.custom_scene, valp, len);
	sac_param.custom_scene[len]	= '\0';
}

#define	sac_param_send_custom_scene		prop_send_generic

void sac_param_set_setmulti_value(struct prop *prop, void *arg,	void *valp,	size_t len)
{
	struct tag_combind_property
	{
		u8 work_mode		:4;
		u8 fan_speed		:4;
		u8 temp				:8;
		u8 fan_updown		:1;
		u8					:3;
		u8 fan_leftright	:1;
		u8					:3;
	} PACKED;

	struct tag_combind_property	*cbd_prop;
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	sac_param.setmulti_value = i;
	cbd_prop = (struct tag_combind_property	*)valp;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_BEEP,			0						);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_WORK_MODE,		cbd_prop->work_mode		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_SPEED,		cbd_prop->fan_speed		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_TEMP,			cbd_prop->temp			);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_POWER,		cbd_prop->fan_updown	);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_LEFTRIGHT,	cbd_prop->fan_leftright	);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_BEEP,			1						);
}

#define	sac_param_send_setmulti_value		prop_send_generic*/

void sac_param_set_device_info(struct prop *prop, void *arg, void *valp, size_t	len)
{
	struct prop	*p_prop;
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	if (i != 0)
	{
		i =	0;
		for	(p_prop	= prop_table; p_prop->name != NULL;	p_prop++)
		{
			p_prop->send_mask =	valid_dest_mask;
		}
	}

	sac_param.device_info =	i;
}

#define	sac_param_send_device_info		prop_send_generic
#define	sac_param_current_present			prop_send_generic
#define	sac_param_route_enable				prop_send_generic
#define	sac_param_device_type			prop_send_generic
//#define	sac_param_manufacturer_code			prop_send_generic
//#define	sac_param_protocol_version			prop_send_generic
//#define	sac_param_soft_version			prop_send_generic
#define	sac_param_error								prop_send_generic

//#define	sac_param_send_humidity			prop_send_generic
//#define	sac_param_send_temp_in			prop_send_generic
//#define	sac_param_send_e_com_error		prop_send_generic

#ifdef DEMO_SCHED_LIB
extern struct sched_prop sched_table[];
void set_schedule(struct	prop *prop,	void *arg, void	*valp, size_t len);
#endif /* DEMO_SCHED_LIB */

int	send_version(struct	prop *prop,	void *arg);
int	send_template_version(struct prop *, void *arg);

struct prop	prop_table[] = {
	/* Control board <-> MCU <-> Cloud */
	{ "t_status",		2, ATLV_INT,	sac_param_set_status,	sac_param_send_status,		&sac_param.status,		sizeof(sac_param.status)						},
	{ "t_delete_route",			1, ATLV_BOOL,	sac_param_set_delete_route,		sac_param_send_delete_route,			&sac_param.delete_route,			sizeof(sac_param.delete_route)							},
	{ "t_reset_factory",			1, ATLV_BOOL,	sac_param_set_reset_factory,		sac_param_send_reset_factory,			&sac_param.reset_factory,			sizeof(sac_param.reset_factory)							},
	{ "t_present",		2, ATLV_INT,	sac_param_set_present,	sac_param_send_present,		&sac_param.present,		sizeof(sac_param.present)						},
	{ "t_direction",			1, ATLV_BOOL,	sac_param_set_direction,		sac_param_send_direction,			&sac_param.direction,			sizeof(sac_param.direction)							},
	{ "t_hand_pull",			1, ATLV_BOOL,	sac_param_set_hand_pull,		sac_param_send_hand_pull,			&sac_param.hand_pull,			sizeof(sac_param.hand_pull)							},
	{ "t_up_route",			1, ATLV_BOOL,	sac_param_set_up_route,		sac_param_send_up_route,			&sac_param.up_route,			sizeof(sac_param.up_route)							},
	{ "t_down_route",			1, ATLV_BOOL,	sac_param_set_down_route,		sac_param_send_down_route,			&sac_param.down_route,			sizeof(sac_param.down_route)							},
	
	{ "f_current_present",			1, ATLV_INT,	NULL,						sac_param_current_present,		&sac_param.current_present,		sizeof(sac_param.current_present),		AFMT_READ_ONLY	},
	{ "f_route_enable",			1, ATLV_BOOL,	NULL,						sac_param_route_enable,		&sac_param.route_enable,		sizeof(sac_param.route_enable),		AFMT_READ_ONLY	},
	{ "f_device_type",			1, ATLV_INT,	NULL,						sac_param_device_type,		&sac_param.device_type,		sizeof(sac_param.device_type),		AFMT_READ_ONLY	},
//	{ "f_manufacturer_code",			1, ATLV_INT,	NULL,						sac_param_manufacturer_code,		&sac_param.manufacturer_code,		sizeof(sac_param.manufacturer_code),		AFMT_READ_ONLY	},
//	{ "f_protocol_version",			1, ATLV_INT,	NULL,						sac_param_protocol_version,		&sac_param.protocol_version,		sizeof(sac_param.protocol_version),		AFMT_READ_ONLY	},
//	{ "f_soft_version",			1, ATLV_INT,	NULL,						sac_param_soft_version,		&sac_param.soft_version,		sizeof(sac_param.soft_version),		AFMT_READ_ONLY	},
	{ "f_error",			1, ATLV_INT,	NULL,						sac_param_error,		&sac_param.error,		sizeof(sac_param.error),		AFMT_READ_ONLY	},
	
	
	
	//{ "t_fan_speed",		2, ATLV_INT,	sac_param_set_fan_speed,	sac_param_send_fan_speed,		&sac_param.fan_speed,		sizeof(sac_param.fan_speed)						},
	//{ "t_work_mode",		2, ATLV_INT,	sac_param_set_work_mode,	sac_param_send_work_mode,		&sac_param.work_mode,		sizeof(sac_param.work_mode)						},
	//{ "t_temp",				2, ATLV_INT,	sac_param_set_temp,			sac_param_send_temp,			&sac_param.temp_f,			sizeof(sac_param.temp_f)						},
	//{ "t_fan_power",		2, ATLV_BOOL,	sac_param_set_fan_power,	sac_param_send_fan_power,		&sac_param.fan_power,		sizeof(sac_param.fan_power)						},
	//{ "t_fan_leftright",	2, ATLV_BOOL,	sac_param_set_fan_leftright,sac_param_send_fan_leftright,	&sac_param.fan_leftright,	sizeof(sac_param.fan_leftright)					},
	//{ "t_power",			1, ATLV_BOOL,	sac_param_set_power,		sac_param_send_power,			&sac_param.power,			sizeof(sac_param.power)							},
	//{ "t_sleep",			1, ATLV_INT,	sac_param_set_sleep,		sac_param_send_sleep,			&sac_param.sleep,			sizeof(sac_param.sleep)							},
	//{ "t_temptype",			1, ATLV_BOOL,	sac_param_set_temptype,		sac_param_send_temptype,		&sac_param.temptype,		sizeof(sac_param.temptype)						},
	//{ "t_fan_mute",			1, ATLV_BOOL,	sac_param_set_fan_mute,		sac_param_send_fan_mute,		&sac_param.fan_mute,		sizeof(sac_param.fan_mute)						},
	//{ "t_eco",				1, ATLV_BOOL,	sac_param_set_eco,			sac_param_send_eco,				&sac_param.eco,				sizeof(sac_param.eco)							},
	//{ "t_run_mode",			1, ATLV_BOOL,	sac_param_set_run_mode,		sac_param_send_run_mode,		&sac_param.run_mode,		sizeof(sac_param.run_mode)						},
	//{ "t_temp_heatcold",	1, ATLV_BOOL,	sac_param_set_temp_heatcold,sac_param_send_temp_heatcold,	&sac_param.temp_heatcold,	sizeof(sac_param.temp_heatcold)					},
	//{ "t_backlight",		1, ATLV_BOOL,	sac_param_set_backlight,	sac_param_send_backlight,		&sac_param.backlight,		sizeof(sac_param.backlight)						},

	/* Control board -x- MCU <-> Cloud */
	//{ PN_ACK_CMD,			1, ATLV_BOOL,	app_param_set_ack_cmd,		app_param_send_ack_cmd,			&app_param.ack_cmd,			sizeof(app_param.ack_cmd)						},
	{ PN_DEVICE_INFO,		1, ATLV_BOOL,	app_param_set_device_info,	app_param_send_device_info,		&app_param.device_info,		sizeof(app_param.device_info)					},
	//{ PN_CUSTOM_SCENE,		1, ATLV_UTF8,	app_param_set_custom_scene,	app_param_send_custom_scene,	&app_param.custom_scene,	sizeof(app_param.custom_scene)					},
	//{ "t_setmulti_value",	1, ATLV_INT,	sac_param_set_setmulti_value,sac_param_send_setmulti_value,	&sac_param.setmulti_value,	sizeof(sac_param.setmulti_value)				},

	/* Control board --> MCU --> Cloud */
	//{ "f_humidity",			1, ATLV_INT,	NULL,						sac_param_send_humidity,		&sac_param.humidity,		sizeof(sac_param.humidity),		AFMT_READ_ONLY	},
	//{ "f_temp_in",			1, ATLV_INT,	NULL,						sac_param_send_temp_in,			&sac_param.temp_in_f,		sizeof(sac_param.temp_in_f),	AFMT_READ_ONLY	},

	/* Control board -x- MCU --> Cloud */
	//{ PN_HARDWARE_TYPE,		1, ATLV_INT,	NULL,						app_param_send_hardware_type,	&app_param.hardware_type,	sizeof(app_param.hardware_type),AFMT_READ_ONLY	},
	//{ PN_COM_ERR,			1, ATLV_BOOL,	NULL,						app_param_send_e_com_error,		&app_param.e_com_error,		sizeof(app_param.e_com_error),	AFMT_READ_ONLY	},
#ifdef DEMO_SCHED_LIB
	{ "schedule_0",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[0]																},
	{ "schedule_1",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[1]																},
	{ "schedule_2",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[2]																},
	{ "schedule_3",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[3]																},
	{ "schedule_4",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[4]																},
	{ "schedule_5",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[5]																},
	{ "schedule_6",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[6]																},
	{ "schedule_7",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[7]																},
	{ "schedule_8",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[8]																},
	{ "schedule_9",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[9]																},
#endif
	{ PN_OEM_HOST_VERSION,	1, ATLV_UTF8,	NULL,						send_template_version																						},
	{ PN_VERSION,			1, ATLV_UTF8,	NULL,						send_version,					NULL,						0,								AFMT_READ_ONLY	},
	{ NULL		}
};
u8 prop_count =	(sizeof(prop_table)	/ sizeof(prop_table[0])) - 1;

