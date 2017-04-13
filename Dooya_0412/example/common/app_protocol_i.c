#include <stdbool.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include <ayla/ayla_proto_mcu.h>
#include "ayla/props.h"
#include "app_protocol_i.h"

extern const struct tag_msg_switch *p_msg_sw_tbl;
extern const u8 msg_sw_tbl_item_num;




/*------------------Dooya CRC-------------------------------*/ 

unsigned int calccrc(unsigned char crcbuf,unsigned int crc) {
     unsigned char i;
     unsigned char chk;
     crc=crc ^ crcbuf;
     for(i=0;i<8;i++)
     {
       chk=crc&1;
       crc=crc>>1;
       crc=crc&0x7fff;
       if (chk==1)
       crc=crc^0xa001;
       crc=crc&0xffff;
}

return crc; }



unsigned int qioucrc16(unsigned int crc,unsigned char *buf,unsigned int x)
{
     unsigned char hi,lo;
     unsigned int i;
     for (i=0;i<x;i++)
     {
        crc=calccrc(*buf,crc);
buf++; }
     hi=crc%256;
     lo=crc/256;
     crc=(lo<<8)|hi;
     return crc;

}
/*------------------Dooya CRC end----------------------------*/ 


/*static struct tag_msg_sub_switch *get_msg_func(u8 msg_type, u8 msg_sub_type)
{
	struct tag_msg_sub_switch *p_sub_sw_tbl;
	u8 sub_sw_tbl_cnt, sub_type, i,	j;

	for	(i = 0;	i <	msg_sw_tbl_item_num; i++)
	{
		if (p_msg_sw_tbl[i].type == msg_type)
		{
			p_sub_sw_tbl	= p_msg_sw_tbl[i].sub_sw_tbl;
			sub_sw_tbl_cnt	= p_msg_sw_tbl[i].sub_sw_tbl_size;
			for	(j = 0;	j <	sub_sw_tbl_cnt;	j++)
			{
				sub_type = p_sub_sw_tbl[j].sub_type;
				if (sub_type ==	255	|| sub_type	== msg_sub_type)
				{
					return &p_sub_sw_tbl[j];
				}
			}
		}
	}

	return NULL;
}*/

/*static u8 calc_chksum_1b_acc(u8	*p_bgn,	u8 *p_end)
{
	u8 chksum;
	u16	len;

	chksum = 0;
	len	= p_end	- p_bgn;
	while (len--)
		chksum += *(p_bgn++);

	return chksum;
}*/

/*static u16 calc_chksum_2b_acc(u8 *p_bgn, u8	*p_end)
{
	u16	chksum;
	u16	len;

	chksum = 0;
	len	= p_end	- p_bgn;
	while (len--)
		chksum += *(p_bgn++);

	return chksum;
}*/

/*static u16 calc_chksum_2b_crc(u8 *p_bgn, u8	*p_end)
{
	return 0;
}

static u32 calc_chksum_4b_crc(u8 *p_bgn, u8	*p_end)
{
	return 0;
}*/

/*static enum tag_return_code parse_frame_link(struct tag_link_layer_hdr *p_link_hdr, u16 link_len,
							   struct tag_netwk_layer_hdr **p_netwk_hdr, u16 *p_netwk_len)
{
	struct tag_link_layer_tail *p_link_tail;
	u8 flg2, *p_ch;
	u16	netwk_len;
	u32	chksum_expect, chksum_real;

	if ((p_link_hdr->tag != 0xF4) || (p_link_hdr->tag2	!= 0xF5))
	{
		return RC_E_LNK_INVALID_FRM_HDR_TAGS;
	}

	switch (p_link_hdr->ack)
	{
		case 0:		return RC_E_LNK_TRAN_FAIL;
		case 1:		break;
		case 8:		return RC_E_LNK_UNABLE_PROC;
		default:	return RC_E_LNK_RESERVED;
	}*/

	//p_ch = (u8 *)(p_link_hdr + 1);	/* Point to	the	member next	to the header. It maybe	flg2 or	payload	length */
	/*if (p_link_hdr->flg1.has_flg2)
	{
		flg2 = *p_ch;
		p_ch +=	sizeof(u8);
		if (flg2)
		{*/
			/* Reserved	*/
//		}
//	}

/*	if (p_link_hdr->flg1.payload_len_size)
	{
		netwk_len =	*(u16 *)p_ch;
		p_ch +=	sizeof(u16);
	}
	else
	{
		netwk_len =	*(u8 *)p_ch;
		p_ch +=	sizeof(u8);
	}

	*p_netwk_hdr = (struct tag_netwk_layer_hdr *)p_ch;
	*p_netwk_len = netwk_len;*/

//	p_ch +=	netwk_len;		/* Point to	chksum */
//	switch (p_link_hdr->flg1.crc_mode)
//	{
//		case 0:	/* 1 byte ACC */
//			chksum_expect =	*p_ch;
//			chksum_real	= calc_chksum_1b_acc(&p_link_hdr->ack, p_ch);
//			p_ch +=	sizeof(u8);
//			break;
//		case 1:	/* 2 bytes ACC */
//			chksum_expect =	ntohs(*(u16	*)p_ch);
//			chksum_real	= calc_chksum_2b_acc(&p_link_hdr->ack, p_ch);
//			p_ch +=	sizeof(u16);
//			break;
//		case 2:	/* 2 bytes CRC */
//			chksum_expect =	ntohs(*(u16	*)p_ch);
//			chksum_real	= calc_chksum_2b_crc(&p_link_hdr->ack, p_ch);
//			p_ch +=	sizeof(u16);
//			break;
//		case 3:	/* 4 bytes CRC */
//			chksum_expect =	ntohl(*(u32	*)p_ch);
//			chksum_real	= calc_chksum_4b_crc(&p_link_hdr->ack, p_ch);
//			p_ch +=	sizeof(u32);
//			break;
//	}

/*	if ( chksum_expect != chksum_real)
	{
		return RC_E_LNK_CHKSUM;
	}

	p_link_tail	= (struct tag_link_layer_tail *)p_ch;
	if ((p_link_tail->tag1 != 0xF4)	|| (p_link_tail->tag2 != 0xFB))
	{
		return RC_E_LNK_INVALID_FRM_TAIL_TAGS;
	}

	return EC_SUCCESS;
}

static enum tag_return_code parse_frame_netwk(struct	tag_netwk_layer_hdr	*p_netwk_hdr, u16 netwk_len,
							struct tag_transf_layer_hdr	**p_transf_hdr,	u16	*p_transf_len)
{
	u8 flg2, *p_ch,	addr_len, addr_num,	i;
	u8 addr[5][8];

	switch (p_netwk_hdr->ack)
	{
		case 0:		return RC_E_NET_TRAN_FAIL;
		case 1:		break;
		case 3:		return RC_E_NET_INVALID_IP_ADDR_MSB;
		case 8:		return RC_E_NET_UNABLE_PROC;
		default:	return RC_E_NET_RESERVED;
	}*/

//	p_ch = (u8 *)(p_netwk_hdr +	1);	/* Point to	the	member next	to the header. It maybe	flg2 or	addresses */
/*	if (p_netwk_hdr->flg1.has_flg2)
	{
		flg2 = *p_ch;
		p_ch +=	sizeof(u8);
		if (flg2)
		{*/
			/* Reserved	*/
/*	}

	switch (p_netwk_hdr->flg1.addr_len)
	{
		case 0:		addr_len = 2;	break;
		case 1:		addr_len = 4;	break;
		case 2:		addr_len = 6;	break;
		case 3:		addr_len = 8;	break;
	}

	if (p_netwk_hdr->flg1.addr_len)
		addr_num = 5;
	else
		addr_num = 2;

	memset(&addr[0][0],	0, sizeof(addr));

	for	(i = 0;	i <	addr_num; i++, p_ch	+= addr_len)
	{
		memcpy(addr[i],	p_ch, addr_len);
	}

	*p_transf_hdr =	(struct	tag_transf_layer_hdr *)p_ch;
	*p_transf_len =	netwk_len -	(p_ch -	(u8	*)p_netwk_hdr);

	return EC_SUCCESS;
}

static enum tag_return_code parse_frame_transf(struct tag_transf_layer_hdr *p_transf_hdr, u16 transf_len,
								struct tag_app_layer_hdr **p_app_hdr, u16 *p_app_len)
{
	u8 flg2, *p_ch;
	u16	encry_ptr;
	u8 encry_array[10];

	switch (p_transf_hdr->ack)
	{
		case 0:		return RC_E_TRF_TRAN_FAIL;
		case 1:		break;
		case 2:		return RC_E_TRF_ID_MISMATCH;
		case 3:		return RC_E_TRF_KEY_MISMATCH;
		case 8:		return RC_E_TRF_UNABLE_PROC;
		default:	return RC_E_TRF_RESERVED;
	}*/

//	p_ch = (u8 *)(p_transf_hdr + 1); /*	Point to the member	next to	the	header.	It maybe encry pointer,	flg2, encry	array or app data */

/*	if (p_transf_hdr->flg1.encry_mode &	0xC)
	{
		encry_ptr =	*(u16 *)p_ch;
		p_ch +=	sizeof(u16);
		if (encry_ptr)
		{*/
			/* Reserved	*/
/*		}
	}

	if (p_transf_hdr->flg1.has_flg2)
	{
		flg2 = *p_ch;
		p_ch +=	sizeof(u8);
		if (flg2)
		{*/
			/* Reserved	*/
/*		}
	}

	if (p_transf_hdr->flg1.encry_mode != 0)
	{
		memcpy(encry_array,	p_ch, sizeof(encry_array));
		p_ch +=	sizeof(encry_array);
	}

	*p_app_hdr = (struct tag_app_layer_hdr *)p_ch;
	*p_app_len = transf_len	- (p_ch	- (u8 *)p_transf_hdr);

	return EC_SUCCESS;
}

static enum tag_return_code parse_frame_app(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct tag_msg_sub_switch *p_msg_func;

	switch (p_app_hdr->result)
	{
		case 0:		return RC_W_APP_OPERATION_FAIL;
		case 1:		break;
		case 2:		return RC_E_APP_INVALID_ID;
		case 3:		return RC_E_APP_NO_CTRLER_W_THIS_ID;
		case 4:		return RC_E_APP_WAIT;
		case 5:		return RC_E_APP_INVALID_VALUE;
		case 6:		return RC_E_APP_CMD_WO_ARGS;
		case 7:		return RC_E_APP_CLOSE_CMD_MAN;
		case 8:		return RC_E_APP_INVALID_CMD;
		case 9:		return RC_E_APP_INVALID_CMD_FMT;
		default:	return RC_E_APP_UNDEFINED;
	}

	p_msg_func = get_msg_func(p_app_hdr->msg_type, p_app_hdr->msg_sub_type);
	if (p_msg_func == NULL)*/
//		return RC_E_APP_NO_MSG_HDL;			/* Can not find	the	message	handle */
/*	else
	{
		if (p_msg_func->func_parse == NULL)*/
//			return RC_W_APP_DUMMY_MSG_HDL;	/* The message handle is not used */
/*		else
			return (*p_msg_func->func_parse)(p_app_hdr,	app_len);
	}
}*/



enum tag_return_code parse_frame(u8 *frm_buf, u16 frm_len)
{
	enum tag_return_code rc;
	u16 crc_check = qioucrc16(0xffff,frm_buf,frm_len-2);
	if(frm_buf[0] == 0x55 || frm_buf[1] == 0xFE || frm_buf[2] == 0xFE)
	{
		if(crc_check == ((frm_buf[frm_len-1] << 8) + frm_buf[frm_len-2]))
		{
			switch(frm_buf[3])
			{
				case 0x01:
					rc = EC_SUCCESS;
				break;
				case 0x02:
					rc = EC_SUCCESS;
				break;
				case 0x03:
					rc = EC_SUCCESS; 
				break;
				case 0x04:
				{
					switch(frm_buf[4])
					{
						case 0x02:
							rc = dooya_get_status(frm_buf + 6, 8);
						break;
						
						case 0x03:
							rc = dooya_get_error(frm_buf + 5, 1);
						break;
					}
				}
				break;
				default:
					rc = EC_SUCCESS; // will delete it
			}
		}
	}


	/*struct tag_link_layer_hdr	*p_link_hdr;
	struct tag_netwk_layer_hdr	*p_netwk_hdr;
	struct tag_transf_layer_hdr	*p_transf_hdr;
	struct tag_app_layer_hdr	*p_app_hdr;
	u16	link_len, netwk_len, transf_len, app_len;
	enum tag_return_code rc;

	p_link_hdr = (struct tag_link_layer_hdr	*)frm_buf;
	link_len = frm_len;

	rc = parse_frame_link(p_link_hdr, link_len,	&p_netwk_hdr, &netwk_len);
	if (rc == EC_SUCCESS)
	{
		rc = parse_frame_netwk(p_netwk_hdr, netwk_len, &p_transf_hdr, &transf_len);
		if (rc == EC_SUCCESS)
		{
			rc = parse_frame_transf(p_transf_hdr, transf_len, &p_app_hdr, &app_len);
			if (rc == EC_SUCCESS)
			{
				rc = parse_frame_app(p_app_hdr, app_len);
				//rc = dooya_get_status(p_app_hdr, app_len);
			}
		}
	}*/

	if (rc != EC_SUCCESS)
		log((0, "Parse rc=%04x\r\n", rc));
	return rc;
}

int build_frame(void *tx_buf, u16 tx_buf_len, union	tag_message	msg, u16 prop_id, u32 arg)
{
#define	PAYLOAD_LEN_TYPE	u8
	struct tag_hdr	*p_hdr;
	struct tag_crc	*p_crc;
	struct tag_data	*p_data;
	struct tag_length	*p_length;
//	struct tag_link_layer_tail	*p_link_tail;
//	struct tag_netwk_layer_hdr	*p_netwk_hdr;
//	struct tag_transf_layer_hdr	*p_transf_hdr;
//	struct tag_app_layer_hdr	*p_app_hdr;
//	struct tag_msg_sub_switch	*p_msg_func;

//	PAYLOAD_LEN_TYPE *p_payload_len;
//	u16	*p_chksum, *p_address;
	int chk_num;

	memset(tx_buf, 0, tx_buf_len);

	/*-----------------------------------------------------------*/

	p_hdr	= (struct tag_hdr *)tx_buf;
	p_hdr->tag					= 0x55;
	p_hdr->address1			= 0xFE;
	p_hdr->address2			= 0xFE;
	switch(prop_id)
	{
		case STATUS: 
						p_hdr->operation			= 0x03;
						if(arg == 0x00)
						{
							p_hdr->data_address	= 0x03;
						}
						else
						{
							p_hdr->data_address	= arg;
						}
						chk_num = 5;	
						p_crc	=(struct tag_crc *)(p_hdr + 1);
			break;
		case DELETE_ROUTE:
						p_hdr->operation			= 0x03;
						if(arg)
							p_hdr->data_address	= 0x07;
						else
							return 0;
						chk_num = 5;	
						p_crc	=(struct tag_crc *)(p_hdr + 1);
			break;
		case RESET_FACTOTY:	
						p_hdr->operation			= 0x03;
						if(arg)
							p_hdr->data_address	= 0x08;
						else
							return 0;
						chk_num = 5;	
						p_crc	=(struct tag_crc *)(p_hdr + 1);
			break;				
		case PRECENT: 
						p_hdr->operation			= 0x03;
						p_hdr->data_address		= 0x04;
						p_data	=(struct tag_data *)(p_hdr +	1);
						p_data->data					= arg; 
						chk_num = 6;
						p_crc	=(struct tag_crc *)(p_data + 1);
			break;
		case DIRECTION: 
						p_hdr->operation			= 0x02;
						p_hdr->data_address	= 0x03;
						p_length	=(struct tag_length *)(p_hdr +	1);
						p_length->length					= 0x01; 
						p_data	=(struct tag_data *)(p_length +	1);
						p_data->data					= arg; 
						chk_num = 7;
						p_crc	=(struct tag_crc *)(p_data + 1);
			break;
		case HAND_PULL:
						p_hdr->operation			= 0x02;
						p_hdr->data_address	= 0x04;
						p_length	=(struct tag_length *)(p_hdr +	1);
						p_length->length					= 0x01; 
						p_data	=(struct tag_data *)(p_length +	1);
						p_data->data					= arg; 
						chk_num = 7;
						p_crc	=(struct tag_crc *)(p_data + 1);
			break;
		case UP_ROUTE: 
						p_hdr->operation			= 0x03;
						p_hdr->data_address		= 0x05;
						p_data	=(struct tag_data *)(p_hdr +	1);
						p_data->data					= arg; 
						chk_num = 6;
						p_crc	=(struct tag_crc *)(p_data + 1);
			break;
		case DOWN_ROUTE: 
						p_hdr->operation			= 0x03;
						p_hdr->data_address		= 0x06;
						p_data	=(struct tag_data *)(p_hdr +	1);
						p_data->data					= arg; 
						chk_num = 6;
						p_crc	=(struct tag_crc *)(p_data + 1);
			break;
		default: 
						chk_num = 5;
						p_crc	=(struct tag_crc *)(p_hdr + 1);
	}

	p_crc->crc	= qioucrc16(0xffff,tx_buf,chk_num);
	
	/*-----------------------------------------------------------*/
	
	return (int)((u8 *)(p_crc	+ 1) - (u8 *)tx_buf);
	
}





	//p_link_hdr->crc.crc_h			= 0x38;
	//p_link_hdr->crc.crc_l			= 0xE5;
	
	
	/*if(arg==0)
	{
		p_link_hdr->data_address	= 0x03; 
		p_link_hdr->crc.crc_h			= 0x38;
		p_link_hdr->crc.crc_l			= 0xE5;
	}else
	{
	p_link_hdr->data_address	= 0x01; 
	p_link_hdr->crc.crc_h			= 0xB9;
	p_link_hdr->crc.crc_l			= 0x24;
	}*/
	//p_link_hdr->flg1.has_flg2			= 0;			/* No flag2	*/
	//p_link_hdr->flg1.frm_idx			= 0;
	//p_link_hdr->flg1.payload_len_size	= 0;			/* Payload length is 1 byte	*/
	//p_link_hdr->flg1.crc_mode			= CRCMD_2B_ADD;	/* 2 bytes add */

	//p_payload_len =	(PAYLOAD_LEN_TYPE *)(p_link_hdr	+ 1);

	/*-----------------------------------------------------------*/

	//p_netwk_hdr	=(struct tag_netwk_layer_hdr *)(p_payload_len +	1);
	//p_netwk_hdr->ack					= 0;			/* Must	be 0 */
	//p_netwk_hdr->flg1.has_flg2			= 0;			/* No flag2	*/
	//p_netwk_hdr->flg1.addr_len			= 0;			/* 2 bytes per address */
	//p_netwk_hdr->flg1.addr_num			= 0;			/* 2 addresses */
	//p_netwk_hdr->flg1.addr_fmt_com_ieee	= 0;
	//p_netwk_hdr->flg1.addr_fmt_ipid		= 0;

	//p_address =	(u16 *)(p_netwk_hdr	+ 1);
	//*(p_address++) = htons(0x0101);
	//*(p_address++) = htons(0xFE01);

	/*-----------------------------------------------------------*/

	//p_transf_hdr=(struct tag_transf_layer_hdr *)p_address;
	//p_transf_hdr->ack					= 0;			/* Must	be 0 */
	//p_transf_hdr->flg1.has_flg2			= 0;			/* No flag2	*/
	//p_transf_hdr->flg1.retry_count		= 0;
	//p_transf_hdr->flg1.encry_mode		= 0;			/* Mode	0: Not encrypt.	No encry pointer and array */

	/*-----------------------------------------------------------*/

	//p_app_hdr =	(struct	tag_app_layer_hdr *)(p_transf_hdr +	1);
	//p_app_hdr->msg_type					= msg.t.type;
	//p_app_hdr->msg_sub_type				= msg.t.sub_type;
	//p_app_hdr->result					= 0;

	//p_msg_func = get_msg_func(msg.t.type, msg.t.sub_type);
	//if (p_msg_func == NULL)
	//	return -1;

	//p_chksum = (*p_msg_func->func_build)(p_app_hdr,	prop_id, arg);

	//*p_payload_len = (PAYLOAD_LEN_TYPE)((u8	*)p_chksum - (u8 *)p_netwk_hdr);
//	*p_chksum =	htons(calc_chksum_2b_acc(&p_link_hdr->ack, (u8 *)p_chksum));

	/*-----------------------------------------------------------*/

	//p_link_tail	= (struct tag_link_layer_tail *)(p_chksum +	1);
	//p_link_tail->tag1 =	0xF4;
	//p_link_tail->tag2 =	0xFB;


//}

