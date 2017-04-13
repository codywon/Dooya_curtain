#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include "ayla/ayla_proto_mcu.h"
#include "ayla/props.h"
#include "ayla/schedeval.h"
#include "app_property.h"

struct tag_app_param app_param;

void app_param_set_ack_cmd(struct prop	*prop, void	*arg, void *valp, size_t len)
{
	u8	i =	*(u8 *)valp;

	if (len	!= sizeof(i)) return;

	app_param.ack_cmd = i ? 0 : 1;
	prop_lookup(PN_ACK_CMD)->send_mask = valid_dest_mask;
}

void app_param_set_device_info(struct prop *prop, void *arg, void *valp, size_t len)
{
	struct prop	*p_prop;
	u8 i = *(u8	*)valp;

	if (len	!= sizeof(i)) return;

	if (i != 0)
	{
		app_param.device_info = 0;
		for	(p_prop	= prop_table; p_prop->name != NULL;	p_prop++)
		{
			p_prop->send_mask =	valid_dest_mask;
		}
		prop_lookup(PN_DEVICE_INFO)->send_mask = valid_dest_mask;
	}
}

void app_param_set_custom_scene(struct prop *prop, void *arg, void *valp, size_t	len)
{
	if (len	>= sizeof(app_param.custom_scene) -	1) {
		return;
	}
	memcpy(app_param.custom_scene, valp, len);
	app_param.custom_scene[len]	= '\0';
}

