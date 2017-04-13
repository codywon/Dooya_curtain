#ifndef	__APP_PROTOCOL_H__
#define	__APP_PROTOCOL_H__

enum tag_return_code
{
	EC_SUCCESS = 0,

	RC_E_LNK_INVALID_FRM_HDR_TAGS	= 0x0001,
	RC_E_LNK_TRAN_FAIL,
	RC_E_LNK_UNABLE_PROC,
	RC_E_LNK_RESERVED,
	RC_E_LNK_CHKSUM,
	RC_E_LNK_INVALID_FRM_TAIL_TAGS,

	RC_E_NET_TRAN_FAIL				= 0x0100,
	RC_E_NET_INVALID_IP_ADDR_MSB,
	RC_E_NET_UNABLE_PROC,
	RC_E_NET_RESERVED,

	RC_E_TRF_TRAN_FAIL				= 0x0200,
	RC_E_TRF_ID_MISMATCH,
	RC_E_TRF_KEY_MISMATCH,
	RC_E_TRF_UNABLE_PROC,
	RC_E_TRF_RESERVED,

	RC_W_APP_OPERATION_FAIL			= 0x0300,
	RC_E_APP_INVALID_ID,
	RC_E_APP_NO_CTRLER_W_THIS_ID,
	RC_E_APP_WAIT,
	RC_E_APP_INVALID_VALUE,
	RC_E_APP_CMD_WO_ARGS,
	RC_E_APP_CLOSE_CMD_MAN,
	RC_E_APP_INVALID_CMD,
	RC_E_APP_INVALID_CMD_FMT,
	RC_E_APP_UNDEFINED,
	RC_E_APP_NO_MSG_HDL,
	RC_W_APP_DUMMY_MSG_HDL,
	RC_E_APP_LEN_MISMATCH
};

enum tag_message_type
{
	TPM_000_BLANK						= 0,
	TPM_001_SET_MASS_DATA				= 1,
	TPM_003_PWR_CTRL					= 3,
	TPM_004_GET_NEW_DEV					= 4,
	TPM_005_SET_DEV_ADDR				= 5,
	TPM_007_GET_DEV						= 7,
	TPM_009_HOST_OWNER					= 9,
	TPM_010_FORCE_RW_DEV_ADDR			= 10,
	TPM_014_GET_GW_NB_REG_TBL			= 14,
	TPM_026_SET_WIFI					= 26,
	TPM_030_WIFI_CTRL_INTACT			= 30,
	TPM_101_SET_STATUS					= 101,
	TPM_102_GET_STATUS					= 102,
	TPM_103_GET_DEV_INF					= 103,
	TPM_201_CAT1_GET_MASS_DATA			= 201,
	TPM_202_BATCH_CMD					= 202
};

enum tag_message_sub_type
{
	STPM_000_000_BLANK					= 0,
	STPM_001_000_SET_EEPROM_DATA		= 0,
	STPM_003_000_PWR_CTRL				= 0,
	STPM_004_000_GET_NEW_DEV_1			= 0,
	STPM_005_000_SET_DEV_ADDR_1			= 0,
	STPM_007_001_RD_DEV_SW_PTCL_VER		= 1,
	STPM_007_004_RD_MCU_SW_PTCL_VER		= 4,
	STPM_009_002_HOST_TRANSFER_OWNER_1	= 2,
	STPM_010_000_FORCE_RW_ADDR_1		= 0,
	STPM_010_004_FORCE_RW_ADDR_3		= 4,
	STPM_014_000_GET_GW_NB_REG_TBL		= 0,
	STPM_026_000_WIFI_BAUDRATE_1		= 0,
	STPM_026_001_WIFI_BAUDRATE_2		= 1,
	STPM_030_000_WIFI_CTRL_INTACT		= 0,
	STPM_101_000_SET_STATUS_1			= 0,
	STPM_101_001_SET_STATUS_2			= 1,
	STPM_101_032_SET_STATUS_32			= 32,
	STPM_102_000_GET_STATUS_1			= 0,
	STPM_102_001_GET_STATUS_2			= 1,
	STPM_102_016_GET_STATUS_17			= 16,
	STPM_103_000_GET_DEV_INF_1			= 0,
	STPM_103_001_GET_DEV_MODEL			= 1,
	STPM_103_002_GET_1TON_DEV_IN_OUT_INF= 2,
	STPM_201_001_GET_EEPROM				= 1,
	STPM_201_011_GET_PROGRAM			= 11,
	STPM_201_022_GET_CMD_KEY			= 22,
	STPM_201_023_GET_EEPROM_KEY			= 23,
	STPM_201_024_GET_PROGRAM_KEY		= 24,
	STPM_201_025_GET_PWD_KEY			= 25,
	STPM_202_000_BAT_CMD_1				= 0,

	STPM_xxx_xxx_DUMMY					= 255
};

enum tag_message_id
{
	IDM_000_000_BLANK						= (TPM_000_BLANK				<<8)+STPM_000_000_BLANK,
	IDM_001_000_DOWNLOAD_EEPROM_DATA_4		= (TPM_001_SET_MASS_DATA		<<8)+STPM_001_000_SET_EEPROM_DATA,
	IDM_003_000_POWER_CTRL					= (TPM_003_PWR_CTRL				<<8)+STPM_003_000_PWR_CTRL,
	IDM_004_000_RDUIRE_NEW_DEV_1			= (TPM_004_GET_NEW_DEV			<<8)+STPM_004_000_GET_NEW_DEV_1,
	IDM_005_000_WRITE_DEV_ADDR_1			= (TPM_005_SET_DEV_ADDR			<<8)+STPM_005_000_SET_DEV_ADDR_1,
	IDM_007_001_RDUIRE_DEV_SW_PROTOCOL_VER	= (TPM_007_GET_DEV				<<8)+STPM_007_001_RD_DEV_SW_PTCL_VER,
	IDM_007_004_RDUIRE_MCU_SW_PROTOCOL_VER	= (TPM_007_GET_DEV				<<8)+STPM_007_004_RD_MCU_SW_PTCL_VER,
	IDM_009_002_HOST_TRANSFER_OWNER_1		= (TPM_009_HOST_OWNER			<<8)+STPM_009_002_HOST_TRANSFER_OWNER_1,
	IDM_010_000_FORCE_RW_DEV_ADDR_1			= (TPM_010_FORCE_RW_DEV_ADDR	<<8)+STPM_010_000_FORCE_RW_ADDR_1,
	IDM_010_004_FORCE_RW_DEV_ADDR_3			= (TPM_010_FORCE_RW_DEV_ADDR	<<8)+STPM_010_004_FORCE_RW_ADDR_3,
	IDM_014_000_GET_GATEWAY_NB_REG_TABLE	= (TPM_014_GET_GW_NB_REG_TBL	<<8)+STPM_014_000_GET_GW_NB_REG_TBL,
	IDM_026_000_WIFI_BAUDRATE_1				= (TPM_026_SET_WIFI				<<8)+STPM_026_000_WIFI_BAUDRATE_1,
	IDM_026_001_WIFI_BAUDRATE_2				= (TPM_026_SET_WIFI				<<8)+STPM_026_001_WIFI_BAUDRATE_2,
	IDM_030_000_WIFI_CTRL_INTACT_1			= (TPM_030_WIFI_CTRL_INTACT		<<8)+STPM_030_000_WIFI_CTRL_INTACT,
	IDM_101_000_SET_STATUS_1				= (TPM_101_SET_STATUS			<<8)+STPM_101_000_SET_STATUS_1,
	IDM_101_001_SET_STATUS_2				= (TPM_101_SET_STATUS			<<8)+STPM_101_001_SET_STATUS_2,
	IDM_101_032_SET_STATUS_32				= (TPM_101_SET_STATUS			<<8)+STPM_101_032_SET_STATUS_32,
	IDM_102_000_GET_STATUS_1				= (TPM_102_GET_STATUS			<<8)+STPM_102_000_GET_STATUS_1,
	IDM_102_001_GET_STATUS_2				= (TPM_102_GET_STATUS			<<8)+STPM_102_001_GET_STATUS_2,
	IDM_102_016_GET_STATUS_17				= (TPM_102_GET_STATUS			<<8)+STPM_102_016_GET_STATUS_17,
	IDM_103_000_GET_DEV_INF_1				= (TPM_103_GET_DEV_INF			<<8)+STPM_103_000_GET_DEV_INF_1,
	IDM_103_001_GET_DEV_MODEL				= (TPM_103_GET_DEV_INF			<<8)+STPM_103_001_GET_DEV_MODEL,
	IDM_103_002_GET_1TON_DEV_IN_OUT_INF		= (TPM_103_GET_DEV_INF			<<8)+STPM_103_002_GET_1TON_DEV_IN_OUT_INF,
	IDM_201_001_GET_EEPROM					= (TPM_201_CAT1_GET_MASS_DATA	<<8)+STPM_201_001_GET_EEPROM,
	IDM_201_011_GET_PROGRAM					= (TPM_201_CAT1_GET_MASS_DATA	<<8)+STPM_201_011_GET_PROGRAM,
	IDM_201_022_GET_CMD_KEY					= (TPM_201_CAT1_GET_MASS_DATA	<<8)+STPM_201_022_GET_CMD_KEY,
	IDM_201_023_GET_EEPROM_KEY				= (TPM_201_CAT1_GET_MASS_DATA	<<8)+STPM_201_023_GET_EEPROM_KEY,
	IDM_201_024_GET_PROGRAM_KEY				= (TPM_201_CAT1_GET_MASS_DATA	<<8)+STPM_201_024_GET_PROGRAM_KEY,
	IDM_201_025_GET_PWD_KEY					= (TPM_201_CAT1_GET_MASS_DATA	<<8)+STPM_201_025_GET_PWD_KEY,
	IDM_202_000_BAT_CMD_1					= (TPM_202_BATCH_CMD			<<8)+STPM_202_000_BAT_CMD_1
};

union tag_message
{
	u16	id;
	struct
	{
		u8 sub_type;
		u8 type;
	} t;
};

int build_frame(void *tx_buf, u16 tx_buf_len, union tag_message msg, u16 prop_id, u32 p_arg);
enum tag_return_code parse_frame(u8 *frm_buf, u16 frm_len);

#endif /* __APP_PROTOCOL_H__ */
