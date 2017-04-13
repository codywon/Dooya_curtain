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
					switch(frm_buf[4])
					{
						case 0xf0:
							rc = dooya_get_device_type(frm_buf + 6, 1); 
						break;
						case 0xfc:;
						break;
						case 0xfd:;
						break;
						case 0xfe:;
						break;
						default:;
					}
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
						p_hdr->operation			= 0x01;
						p_hdr->data_address		= arg;
						p_data	=(struct tag_data *)(p_hdr +	1);
						p_data->data					= 0x01; 
						chk_num = 6;
						p_crc	=(struct tag_crc *)(p_data + 1);
			break;
	}

	p_crc->crc	= qioucrc16(0xffff,tx_buf,chk_num);
	
	/*-----------------------------------------------------------*/
	
	return (int)((u8 *)(p_crc	+ 1) - (u8 *)tx_buf);
	
}
