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
	PID_DEVICE_TYPE,
	//PID_MANUFACTURER_CODE,
	//PID_PROTOCOL_VERSION,
	//PID_SOFT_VERSION,
	PID_ERROR,
	
	/* Control board -x- MCU <-> Cloud */
	PID_DEVICE_INFO,
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
	
	/* Control board --> MCU --> Cloud */
	u32 current_present;
	//u32 angle;
	u8  route_enable;
	u32 device_type;
//	u32 manufacturer_code;
//	u32 protocol_version;
//	u32 soft_version;
	u32 error;
	/* Control board -x- MCU <-> Cloud */
	u8 device_info;
};

extern struct tag_sac_param	sac_param;

#endif /* __SAC_PROPERTY_H__ */
