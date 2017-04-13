#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include "ayla/ayla_proto_mcu.h"
#include "ayla/props.h"
#include "ayla/schedeval.h"
#include "app_property.h"
#include "app_control.h"
#include "deh_property.h"

struct tag_deh_param deh_param;

static void	deh_param_set_power(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_POWER, i);
}

#define	deh_param_send_power			prop_send_generic

static void	deh_param_set_work_mode(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_WORK_MODE, i);
}

#define	deh_param_send_work_mode		prop_send_generic

static void	deh_param_set_fan_speed(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_SPEED, i);
}

#define	deh_param_send_fan_speed		prop_send_generic

static void	deh_param_set_humidity(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_HUMIDITY, i);
}

#define	deh_param_send_humidity			prop_send_generic

static void	deh_param_set_e_heat_temp(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_E_HEAT_TEMP, i);
}

#define	deh_param_send_e_heat_temp		prop_send_generic

static void	deh_param_set_e_heat(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_E_HEAT,	i);
}

#define	deh_param_send_e_heat			prop_send_generic

static void	deh_param_set_negative_ion(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_NEGATIVE_ION, i);
}

#define	deh_param_send_negative_ion		prop_send_generic

static void	deh_param_set_water_pump(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_WATER_PUMP,	i);
}

#define	deh_param_send_water_pump		prop_send_generic

static void	deh_param_set_temptype(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_TEMPTYPE, i);
}

#define	deh_param_send_temptype			prop_send_generic

static void	deh_param_set_setmulti_value(struct	prop *prop,	void *arg, void	*valp, size_t len)
{
	struct tag_combind_property
	{
		u8 work_mode		:4;
		u8 fan_speed		:4;
		u8 humidity			:7;
		u8 e_heat			:1;
		u8 temp				:7;
		u8 temptype			:1;
	} PACKED;

	struct tag_combind_property	*cbd_prop;
	u32	i =	*(u32 *)valp;

	if (len	!= sizeof(i)) return;

	deh_param.setmulti_value = i;
	cbd_prop = (struct tag_combind_property	*)valp;

	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_BEEP,			0						);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_WORK_MODE,		cbd_prop->work_mode		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_FAN_SPEED,		cbd_prop->fan_speed		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_HUMIDITY,		cbd_prop->humidity		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_E_HEAT,			cbd_prop->e_heat		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_E_HEAT_TEMP,	cbd_prop->temp			);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_TEMPTYPE,		cbd_prop->temptype		);
	msg_queue_put(IDM_101_000_SET_STATUS_1,	PID_BEEP,			1						);

	prop_table[PID_SETMULTI_VALUE].send_mask =	valid_dest_mask;
}

#define	deh_param_send_setmulti_value	prop_send_generic

#define	deh_param_send_humidity_indoor	prop_send_generic
#define	deh_param_send_temp_indoor		prop_send_generic

#ifdef DEMO_SCHED_LIB
extern struct sched_prop sched_table[];
void set_schedule(struct	prop *prop,	void *arg, void	*valp, size_t len);
#endif /* DEMO_SCHED_LIB */

int	send_version(struct	prop *prop,	void *arg);
int	send_template_version(struct prop *, void *arg);

struct prop	prop_table[] = {
	/* Control board <-> MCU <-> Cloud */
	{ "t_power",			1, ATLV_BOOL,	deh_param_set_power,		deh_param_send_power,			&deh_param.power,			sizeof(deh_param.power)								},
	{ "t_work_mode",		2, ATLV_INT,	deh_param_set_work_mode,	deh_param_send_work_mode,		&deh_param.work_mode,		sizeof(deh_param.work_mode)							},
	{ "t_fan_speed",		2, ATLV_INT,	deh_param_set_fan_speed,	deh_param_send_fan_speed,		&deh_param.fan_speed,		sizeof(deh_param.fan_speed)							},
	{ "t_humidity",			2, ATLV_INT,	deh_param_set_humidity,		deh_param_send_humidity,		&deh_param.humidity,		sizeof(deh_param.humidity)							},
	{ "t_heat_temp",		2, ATLV_INT,	deh_param_set_e_heat_temp,	deh_param_send_e_heat_temp,		&deh_param.e_heat_temp_f,	sizeof(deh_param.e_heat_temp_f)						},
	{ "t_heat",				2, ATLV_BOOL,	deh_param_set_e_heat,		deh_param_send_e_heat,			&deh_param.e_heat,			sizeof(deh_param.e_heat)							},
	{ "t_negativeions",		1, ATLV_BOOL,	deh_param_set_negative_ion,	deh_param_send_negative_ion,	&deh_param.negative_ion,	sizeof(deh_param.negative_ion)						},
	{ "t_waterpump",		1, ATLV_BOOL,	deh_param_set_water_pump,	deh_param_send_water_pump,		&deh_param.water_pump,		sizeof(deh_param.water_pump)						},
	{ "t_temptype",			2, ATLV_BOOL,	deh_param_set_temptype,		deh_param_send_temptype,		&deh_param.temptype,		sizeof(deh_param.temptype)							},

	/* Control board -x- MCU <-> Cloud */
	{ PN_ACK_CMD,			1, ATLV_BOOL,	app_param_set_ack_cmd,		app_param_send_ack_cmd,			&app_param.ack_cmd,			sizeof(app_param.ack_cmd)							},
	{ PN_DEVICE_INFO,		1, ATLV_BOOL,	app_param_set_device_info,	app_param_send_device_info,		&app_param.device_info,		sizeof(app_param.device_info)						},
	{ PN_CUSTOM_SCENE,		1, ATLV_UTF8,	app_param_set_custom_scene,	app_param_send_custom_scene,	&app_param.custom_scene,	sizeof(app_param.custom_scene)						},
	{ "t_setmulti_value",	1, ATLV_INT,	deh_param_set_setmulti_value,deh_param_send_setmulti_value,	&deh_param.setmulti_value,	sizeof(deh_param.setmulti_value)					},

	/* Control board --> MCU --> Cloud */
	{ "f_humidity",			1, ATLV_INT,	NULL,						deh_param_send_humidity_indoor,	&deh_param.humidity_indoor,	sizeof(deh_param.humidity_indoor),	AFMT_READ_ONLY	},
	{ "f_temp_in",			1, ATLV_INT,	NULL,						deh_param_send_temp_indoor,		&deh_param.temp_indoor_f,	sizeof(deh_param.temp_indoor_f),	AFMT_READ_ONLY	},

	/* Control board -x- MCU --> Cloud */
	{ PN_HARDWARE_TYPE,		1, ATLV_INT,	NULL,						app_param_send_hardware_type,	&app_param.hardware_type,	sizeof(app_param.hardware_type),	AFMT_READ_ONLY	},
	{ PN_COM_ERR,			1, ATLV_BOOL,	NULL,						app_param_send_e_com_error,		&app_param.e_com_error,		sizeof(app_param.e_com_error),		AFMT_READ_ONLY	},
#ifdef DEMO_SCHED_LIB
	{ "schedule_0",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[0]																	},
	{ "schedule_1",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[1]																	},
	{ "schedule_2",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[2]																	},
	{ "schedule_3",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[3]																	},
	{ "schedule_4",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[4]																	},
	{ "schedule_5",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[5]																	},
	{ "schedule_6",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[6]																	},
	{ "schedule_7",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[7]																	},
	{ "schedule_8",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[8]																	},
	{ "schedule_9",			1, ATLV_SCHED,	set_schedule,				NULL,							&sched_table[9]																	},
#endif
	{ "oem_host_version",	1, ATLV_UTF8,	NULL,						send_template_version																							},
	{ "version",			1, ATLV_UTF8,	NULL,						send_version,					NULL,						0,									AFMT_READ_ONLY	},
	{ NULL		}
};
u8 prop_count =	(sizeof(prop_table)	/ sizeof(prop_table[0])) - 1;

