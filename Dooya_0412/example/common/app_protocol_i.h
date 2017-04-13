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

/*enum tag_crc_mode
{
	CRCMD_1B_ADD = 0,
	CRCMD_2B_ADD = 1,
	CRCMD_2B_CRC = 2,
	CRCMD_4B_CRC = 3
};*/

struct tag_link_layer_hdr
{
	u8 tag;					/* 0xF4	*/
	u8 address1;					/* 0xF5	*/
	u8 address2;						/* 0 = fail, 1 = succeed, 8	= unknown */
	u8 operation;
	u8 data_address;
	//u16 crc;

	
//	struct
//	{
//		u8 has_flg2			:1;	/* 0 = no flag2, 1 = has flag2 */
//		u8 frm_idx			:4;	/* Frame index number */
//		u8 payload_len_size	:1;	/* 0 = 1 byte, 1 = 2 bytes */
//		u8 crc_mode			:2;	/* CRC length */
//	} PACKED flg1;
} PACKED;

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


struct tag_link_layer_tail
{
	u8 tag1;					/* 0xF4	*/
	u8 tag2;					/* 0xFB	*/
} PACKED;

struct tag_netwk_layer_hdr
{
	u16 ack;						/* 0, 1, 3,	8 */
	struct
	{
		u8 has_flg2			:1;	/* 0 = no flag2, 1 = has flag2 */
		u8 reserve			:2;
		u8 addr_len			:2;	/* IP address length */
		u8 addr_num			:1;	/* 0 = addr1~2,	1 =	addr1~5	*/
		u8 addr_fmt_com_ieee:1;
		u8 addr_fmt_ipid	:1;	/* 0 = IP, 1 = ID */
	} PACKED flg1;
} PACKED;

struct tag_transf_layer_hdr
{
	u8 ack;						/* 0, 1, 2,	3, 8 */
	struct
	{
		u8 has_flg2			:1;	/* 0 = no flag2, 1 = has flag2 */
		u8 retry_count		:3;	/* Count of	retry for no ack */
		u8 encry_mode		:4;	/* Data	encry mode */
	} PACKED flg1;
} PACKED;

struct tag_app_layer_hdr
{
	u8 msg_type;
	u8 msg_sub_type;
	u8 result;
} PACKED;

struct tag_app_layer_hdr_w_frm_info
{
	u8 msg_type;
	u8 msg_sub_type;
	u8 result;
	u8 frm_flg;
	u8 frm_idx;
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

typedef	void *(*MSG_FUNC_BUILD)(struct tag_app_layer_hdr *p_app_hdr, u16 prop_id, u32 arg);
typedef	enum tag_return_code (*MSG_FUNC_PARSE)(struct tag_app_layer_hdr *p_app_hdr, u16 len);

struct tag_msg_sub_switch
{
	u8							sub_type;
	MSG_FUNC_BUILD				func_build;
	MSG_FUNC_PARSE				func_parse;
};

struct tag_msg_switch
{
	u8							type;
	struct tag_msg_sub_switch	*sub_sw_tbl;
	u8							sub_sw_tbl_size;
};

#endif /* __APP_PROTOCOL_I_H__ */
