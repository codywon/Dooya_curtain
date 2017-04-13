#ifndef	__SAC_PROTOCOL_I_H__
#define	__SAC_PROTOCOL_I_H__


struct dooya_operation_4
{
	u8 current_present;
	u8 direction;
	u8 hand_pull;
	u8 status;
	u8 reserve5;
	u8 reserve6;
	u8 reserve7;
	u8 route_enable;
}PACKED;

struct dooya_error
{
	u8 error;
}PACKED;


struct dooya_get_device_type
{
	u8 device_type;
}PACKED;


#endif /* __SAC_PROTOCOL_I_H__ */
