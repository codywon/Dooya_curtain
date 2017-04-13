#ifndef	__SAC_PROPERTY_H__
#define	__SAC_PROPERTY_H__

enum tag_prop_index
{
	/* Control board <-> MCU <-> Cloud */
	PID_STATUS,
	PID_DELETE_ROUTE,
	PID_RESET_FACTORY,
	PID_PRESENT,
	PID_DIRECTION,
	PID_HAND_PULL,
	PID_UP_ROUTE,
	PID_DOWN_ROUTE,
	
	/* Control board --> MCU --> Cloud */
	PID_CURRENT_PRESENT,
	PID_ROUTE_ENABLE,
	//PID_DEVICE_TYPE,
	//PID_MANUFACTURER_CODE,
	//PID_PROTOCOL_VERSION,
	//PID_SOFT_VERSION,
	PID_ERROR,
	
	/* Control board -x- MCU <-> Cloud */
	PID_DEVICE_INFO,
	
	
	//PID_FAN_SPEED,
	//PID_WORK_MODE,
	//PID_TEMP,
	//PID_FAN_POWER,
	//PID_FAN_LEFTRIGHT,
	//PID_POWER,
	//PID_SLEEP,
	//PID_TEMPTYPE,
	//PID_FAN_MUTE,
	//PID_ECO,
	//PID_RUN_MODE,
	//PID_TEMP_HEATCOLD,
	//PID_BACKLIGHT,

	/* Control board -x- MCU <-> Cloud */
	//PID_ACK_CMD,		/* common prpperty */
	//PID_DEVICE_INFO,	/* common prpperty */
	//PID_CUSTOM_SCENE,	/* common prpperty */
	//PID_SETMULTI_VALUE,

	/* Control board --> MCU --> Cloud */
	//PID_HUMIDITY,
	//PID_TEMP_IN,

	/* Control board -x- MCU --> Cloud */
	//PID_HARDWARE_TYPE,	/* common prpperty */
	//PID_E_COM_ERROR,

	/* User defined ID. Not real property ID! */
	//PID_BEEP = 0x0100
};

struct tag_sac_param
{
	/* Control board <-> MCU <-> Cloud */
	u32 status;
	u8  delete_route;
	u8  reset_factory;
	u32 present;
	u8  direction;
	u8  hand_pull;
	u8  up_route;
	u8  down_route;
	//u32 light;
	//u32 hint;
	//u32 angle_coe;
	//u8  inching_mode;
	//u32 inching_speed;
	
	/* Control board --> MCU --> Cloud */
	u32 current_present;
	//u32 angle;
	u8  route_enable;
//	u32 device_type;
//	u32 manufacturer_code;
//	u32 protocol_version;
//	u32 soft_version;
	u32 error;
	/* Control board -x- MCU <-> Cloud */
	u8 device_info;
	
	
	
	//u32	fan_speed;
	//u32	work_mode;
	//u32	temp_f;		/* Always in F degree, associated with cloud */
	//u32	temp;		/* In C or F degree, associated with device */
	//u8 fan_power;
	//u8 fan_leftright;
	//u8 power;
	//u32	sleep;
	//u8 temptype;
	//u8 fan_mute;
	//u8 eco;
	//u8 run_mode;
	//u8 temp_heatcold;
	//u8 backlight;

	/* Control board -x- MCU <-> Cloud */
	//u8 custom_scene[256];
	//u32	setmulti_value;
	//u8 device_info;

	/* Control board --> MCU --> Cloud */
	//u32	humidity;
	//u32	temp_in_f;	/* Always in F degree, associated with cloud */
	//u32	temp_in;	/* In C or F degree, associated with device */	
};

extern struct tag_sac_param	sac_param;

#endif /* __SAC_PROPERTY_H__ */
