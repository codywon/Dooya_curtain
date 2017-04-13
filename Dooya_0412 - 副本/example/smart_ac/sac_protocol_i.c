#include <stdbool.h>
#include <string.h>
#include "ayla/mcu_platform.h"
#include <ayla/ayla_proto_mcu.h>
#include "ayla/props.h"
#include "app_protocol_i.h"
#include "app_property.h"
#include "app_control.h"
#include "sac_protocol_i.h"
#include "sac_property.h"


/*-------------------------------------------------------------------------------------------------*/

enum tag_return_code dooya_get_status(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct dooya_operation_4 *p_data; //by guoguang

	p_data = (struct dooya_operation_4 *)(p_app_hdr);
	//if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		//return RC_E_APP_LEN_MISMATCH;

	/* Control board <-> MCU <-> Cloud */
	SEND_TO_CLOUD(sac_param,	status,		PID_STATUS		);
	SEND_TO_CLOUD(sac_param,	current_present,		PID_CURRENT_PRESENT		);
	SEND_TO_CLOUD(sac_param,	direction,		PID_DIRECTION		);
	SEND_TO_CLOUD(sac_param,	hand_pull,		PID_HAND_PULL		);
	SEND_TO_CLOUD(sac_param,	route_enable,		PID_ROUTE_ENABLE		);
	
	return EC_SUCCESS;
}

enum tag_return_code dooya_get_error(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct dooya_error *p_data; //by guoguang

	p_data = (struct dooya_error *)(p_app_hdr);
	//if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		//return RC_E_APP_LEN_MISMATCH;

	/* Control board <-> MCU <-> Cloud */
	SEND_TO_CLOUD(sac_param,	error,		PID_ERROR		);
	
	return EC_SUCCESS;
}

enum tag_return_code dooya_get_device_type(struct tag_app_layer_hdr *p_app_hdr, u16 app_len)
{
	struct dooya_get_device_type *p_data; //by guoguang

	p_data = (struct dooya_get_device_type *)(p_app_hdr);
	//if (app_len	!= sizeof(*p_app_hdr) +	sizeof(*p_data))
		//return RC_E_APP_LEN_MISMATCH;

	/* Control board <-> MCU <-> Cloud */
	SEND_TO_CLOUD(sac_param,	device_type,		PID_DEVICE_TYPE		);
	
	return EC_SUCCESS;
}

/*-------------------------------------------------------------------------------------------------*/

