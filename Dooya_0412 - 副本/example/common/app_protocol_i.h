#ifndef	__APP_PROTOCOL_I_H__
#define	__APP_PROTOCOL_I_H__

#include "app_protocol.h"

#define STATUS 					0
#define DELETE_ROUTE 		1
#define RESET_FACTOTY 	2
#define PRECENT 				3
#define DIRECTION 			4
#define HAND_PULL 			5
#define UP_ROUTE 				6
#define DOWN_ROUTE			7


struct tag_hdr
{
	u8 tag;					
	u8 address1;					
	u8 address2;						
	u8 operation;
	u8 data_address;
} PACKED;

struct tag_crc
{
	u16 crc;						
} PACKED;

struct tag_data
{
	u8 data;						
} PACKED;

struct tag_length
{
	u8 length;						
} PACKED;


struct tag_app_layer_hdr
{
	u8 msg_type;
	u8 msg_sub_type;
	u8 result;
} PACKED;


#define SEND_TO_ECB(member)			{	p_data->member	= arg; \
										p_data->member##_set =	true; \
									}
#define SEND_TO_ECB_F(member, unit)	{	p_data->member = unit ? arg : f2c(arg); \
										p_data->member##_set =	true; \
									}
#define	SEND_TO_CLOUD(param, member, prop_id) \
									{	if(	param.member != p_data->member ) \
										{ \
											param.member = p_data->member; \
											prop_table[prop_id].send_mask =	valid_dest_mask; \
										} \
									}
#define	SEND_TO_CLOUD_F(param, member, prop_id, unit) \
									{	if(	param.member != p_data->member ) \
										{ \
											param.member = p_data->member; \
											param.member##_f = param.unit ? param.member : c2f(param.member); \
											prop_table[prop_id].send_mask =	valid_dest_mask; \
										} \
									}


#endif /* __APP_PROTOCOL_I_H__ */
