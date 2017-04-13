#ifndef	__DEH_PROPERTY_H__
#define	__DEH_PROPERTY_H__

enum tag_prop_index
{
	/* Control board <-> MCU <-> Cloud */
	PID_POWER,
	PID_WORK_MODE,
	PID_FAN_SPEED,
	PID_HUMIDITY,
	PID_E_HEAT_TEMP,
	PID_E_HEAT,
	PID_NEGATIVE_ION,
	PID_WATER_PUMP,
	PID_TEMPTYPE,

	/* Control board -x- MCU <-> Cloud */
	PID_ACK_CMD,		/* common prpperty */
	PID_DEVICE_INFO,	/* common prpperty */
	PID_CUSTOM_SCENE,	/* common prpperty */
	PID_SETMULTI_VALUE,

	/* Control board --> MCU --> Cloud */
	PID_HUMIDITY_INDOOR,
	PID_TEMP_INDOOR,

	/* Control board -x- MCU --> Cloud */
	PID_HARDWARE_TYPE,	/* common prpperty */
	PID_E_COM_ERROR,

	/* User defined ID. Not real property ID! */
	PID_BEEP = 0x0100
};

struct tag_deh_param
{
	/* Control board <-> MCU <-> Cloud */
	u8 power;
	u32	work_mode;
	u32	fan_speed;
	u32	humidity;
	u32	e_heat_temp_f;	/* Always in F degree, associated with cloud */
	u32	e_heat_temp;	/* In C or F degree, associated with device */
	u8 e_heat;
	u8 negative_ion;
	u8 water_pump;
	u8 temptype;

	/* Control board -x- MCU <-> Cloud */
	u32	setmulti_value;

	/* Control board --> MCU --> Cloud */
	u32	humidity_indoor;
	u32	temp_indoor_f;	/* Always in F degree, associated with cloud */
	u32	temp_indoor;	/* In C or F degree, associated with device */	
};

extern struct tag_deh_param	deh_param;

#endif /* __DEH_PROPERTY_H__ */
