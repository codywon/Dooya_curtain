#ifndef	__APP_PROPERTY_H__
#define	__APP_PROPERTY_H__

/* Common property name define */
#define PN_ACK_CMD			"ack_cmd"
#define PN_DEVICE_INFO		"t_device_info"
#define PN_HARDWARE_TYPE	"hardware_type"
#define PN_CUSTOM_SCENE		"t_custom_scene"
#define PN_COM_ERR			"f_e_com_error"
#define PN_OEM_HOST_VERSION	"oem_host_version"
#define	PN_VERSION			"version"

struct tag_app_param
{
	/* Control board <-> MCU <-> Cloud */

	/* Control board -x- MCU <-> Cloud */
	u8 ack_cmd;
	u8 device_info;
	u32 hardware_type;
	u8 custom_scene[256];

	/* Control board --> MCU --> Cloud */

	/* Control board -x- MCU --> Cloud */
	u8 e_com_error;

	/* Internal use only */
	u8 beep;
};

extern struct tag_app_param	app_param;

void app_param_set_ack_cmd(struct prop	*prop, void	*arg, void *valp, size_t len);
#define	app_param_send_ack_cmd			prop_send_generic
void app_param_set_device_info(struct prop *prop, void *arg, void *valp, size_t len);
#define	app_param_send_device_info		prop_send_generic
void app_param_set_custom_scene(struct prop *prop, void *arg, void *valp, size_t len);
#define	app_param_send_custom_scene		prop_send_generic
#define	app_param_send_hardware_type	prop_send_generic
#define	app_param_send_e_com_error		prop_send_generic

#endif /* __APP_PROPERTY_H__ */
