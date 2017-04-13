/*
 * Copyright 2011 Ayla Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Ayla Networks, Inc.
 */
#include <string.h>
#include <ayla/mcu_platform.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/serial_msg.h>
#ifdef AYLA_FILE_PROP
#include <ayla/prop_dp.h>
#endif

u8 valid_dest_mask; /* initial value is 0 */
static u8 prop_request_all;
static u16 prop_request_all_req_id;
static u8 send_ads_listen_cmd;
static struct prop *prop_active = NULL;
static struct prop *prop_retry;
static u8 prop_retry_is_resp;
static void *prop_retry_arg;


/*
 * prop_lookup() - Lookup a property table entry by name.
 */
struct prop *prop_lookup(const char *name)
{
	struct prop *prop;

	for (prop = prop_table; prop->name != NULL; prop++) {
		if (!strcmp(prop->name, name)) {
			return prop;
		}
	}
	return NULL;
}

/*
 * Schedule property to be sent to module.
 */
void prop_send_req(const char *name)
{
	struct prop *prop;

	prop = prop_lookup(name);
	if (prop) {
		prop->send_mask = ALL_DESTS_BIT;
	}
}

/*
 * Schedule property to be sent only to service
 */
void prop_send_req_to_ads_only(const char *name)
{
	struct prop *prop;

	prop = prop_lookup(name);
	if (prop) {
		prop->send_mask = ADS_BIT;
	}
}

/*
 * prop_lookup_len() - Lookup a property table entry by name and name_length.
 */
struct prop *prop_lookup_len(const char *name, size_t name_len)
{
	struct prop *prop;

	for (prop = prop_table; prop->name != NULL; prop++) {
		if (prop->name_len == 0) {
			prop->name_len = strlen(prop->name);
		}
		if (prop->name_len == name_len && 
		    !memcmp(prop->name, name, name_len)) {
			return prop;
		}
	}
	return NULL;
}

/*
 * prop_send(struct prop *, const void *, size_t, void *)
 * Send the specified property as a new datapoint to the network service.
 * This may be changed to do queuing someday.
 */
int prop_send(struct prop *prop, const void *val, size_t val_len, void *arg)
{
	if (prop->name_len == 0) {
		prop->name_len = strlen(prop->name);
	}
	return serial_send_prop(prop, val, val_len, arg);
}

/*
 * prop_send_generic(struct prop *, void *)
 * Send prop->arg as property's value.
 * Determine the val_len by looking at its type.
 * Can be used for ATLV_BOOL, ATLV_INT, ATLV_UTF8, and ATLV_CENTS.
 */
int prop_send_generic(struct prop *prop, void *arg)
{
	size_t val_len;

	if (!prop->arg) {
		return -1;
	}
	if (prop->type == ATLV_UTF8) {
		val_len = strlen(prop->arg);
	} else if (prop->val_len) {
		val_len = prop->val_len;
	} else {
		switch (prop->type) {
		case ATLV_BOOL:
			val_len = 1;
			break;
		case ATLV_INT:
		case ATLV_UINT:
		case ATLV_CENTS:
			val_len = 4;
			break;
		default:
			return -1;
		}
	}
	return prop_send(prop, prop->arg, val_len, arg);
}

/*
 * prop_request_value(const char *).
 * Request the value of the specified property from ADS.
 * If name is null, ask for all "to-device" properties.
 * If the name isn't in the prop table, it'll return -1.
 */
int prop_request_value(const char *name)
{
	struct prop *prop;

	if (name) {
		prop = prop_lookup(name);
		if (!prop) {
			return -1;
		}
		if (prop->name_len == 0) {
			prop->name_len = strlen(prop->name);
		}
		prop->get_val = 1;
	} else {
		prop_request_all = 1;
	}
	
	return 0;
}

/*
 * Byte-swap the property's value if appropriate.
 * For now, this is done only on integers.
 * Eventually floating point may need to be supported.
 */
void prop_swap(struct prop *prop, void *valp)
{
	switch (prop->type) {
	case ATLV_INT:
	case ATLV_CENTS:
	case ATLV_UINT:
		switch (prop->val_len) {
		case sizeof(s16):
			* (s16 *)valp = htons(* (s16 *)valp);
			break;
		case sizeof(u32):
			* (s32 *)valp = htonl(* (s32 *)valp);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

/*
 * Notify of failure to send or req prop from service
 */
void prop_notify_failure(u16 req_id, struct prop *prop, u8 dests)
{
	if (!prop && (prop_request_all_req_id == req_id)) {
		prop_request_all = 1;
	} else if (prop_active && (prop == prop_active) &&
	    req_id == prop_active->req_id && (dests & ADS_BIT)) {
		if (prop_active->curr_op == POST) {
			prop_active->send_mask |= ADS_BIT;
		} else if (prop_active->curr_op == ECHO) {
			prop_active->send_mask |= ADS_BIT;
			prop->echo = 1;
		} else {
			prop_active->get_val = 1;
		}
	}
	prop_active = NULL;
}

/*
 * Check whether property update has been delivered.
 */
int prop_send_done(struct prop *prop)
{
	if (prop_active == prop && serial_is_ads_busy()) {
		return -1;
	}
	if (prop->send_mask) {
		return -1;
	}
	return prop->send_err;
}

/*
 * Setup a retry for a prop_send during the next prop_poll
 */
void prop_setup_retry(struct prop *prop, void *arg, u8 is_resp)
{
	prop_retry = prop;
	prop_retry_arg = arg;
	prop_retry_is_resp = is_resp;
}

/*
 * Check if the prop state machine is ready to immediately respond to
 * a property request
 */
int prop_is_busy(void)
{
	if (prop_retry) {
		return 1;
	}
#ifdef AYLA_FILE_PROP
	if (prop_dp_active) {
		return 1;
	}
#endif
	return 0;
}

/*
 * prop_poll() - send properties with send_mask set.
 */
void prop_poll(void)
{
	struct prop *prop;
	void *prop_arg;
	u8 is_resp;
	u8 ads_up = (valid_dest_mask & ADS_BIT);
	u16 req_id;

	/* If a retry has been set up, first execute the retry */
	if (prop_retry) {
		prop = prop_retry;
		prop_arg = prop_retry_arg;
		is_resp = prop_retry_is_resp;
		prop_retry = NULL;
		prop_retry_arg = NULL;
		prop_retry_is_resp = 0;
		if (prop->send && (is_resp ||
		    (prop->send_mask & valid_dest_mask))) {
			if (prop->send(prop, prop_arg)) {
				return;
			}
			if (!is_resp) {
				prop->send_mask &= ~(NOT_ADS_BIT |
				    valid_dest_mask);
			}
			prop->echo = 0;
			prop_active = prop;
		}
	}
#ifdef AYLA_FILE_PROP
	/* if in the middle of a stream post, continue sending it */
	if (prop_dp_active) {
		if (prop_dp_active->prop->send_mask) {
			prop = prop_dp_active->prop;
			if (prop->send(prop, NULL)) {
				return;
			}
			prop->send_mask &= ~(NOT_ADS_BIT | valid_dest_mask);
			prop_active = prop;
		}
		return;
	}
#endif /* AYLA_FILE_PROP*/

	/* Send out property echoes */
	for (prop = prop_table; prop->name != NULL; prop++) {
		if (prop->send && ads_up && (prop->send_mask & ADS_BIT) &&
		    prop->echo) {
			if (prop->send(prop, NULL)) {
				return;
			}
			prop->send_mask = 0;
			prop_active = prop;
			prop->echo = 0;
		}
	}

	/* Enable receiving props/updates from service */
	if (send_ads_listen_cmd) {
		if (serial_tx_service_listen()) {
			return;
		}
		send_ads_listen_cmd = 0;
	}

	/* Send a GET all-to device props cmd to the service */
	if (prop_request_all && ads_up) {
		if (serial_request_prop(NULL, &req_id)) {
			return;
		}
		prop_request_all = 0;
		prop_request_all_req_id = req_id;
		for (prop = prop_table; prop->name != NULL; prop++) {
			prop->get_val = 0;
		}
	}

	/* Send props or request values */
	for (prop = prop_table; prop->name != NULL; prop++) {
		if (prop->get_val && ads_up) {
			if (serial_request_prop(prop, &req_id)) {
				return;
			}
			prop->get_val = 0;
			prop_active = prop;
		}
		if (prop->send && (prop->send_mask & valid_dest_mask)) {
			if (prop->send(prop, NULL)) {
				return;
			}
			prop->send_mask &= ~(NOT_ADS_BIT | valid_dest_mask);
			prop_active = prop;
		}
	}
}

int prop_pending(void)
{
	struct prop *prop;
	u8 ads_up = (valid_dest_mask & ADS_BIT);

	for (prop = prop_table; prop->name != NULL; prop++) {
		if (prop->get_val && ads_up) {
			return 1;
		}
		if (prop->send_mask & valid_dest_mask) {
			return 1;
		}
	}
	return 0;
}

/*
 * Update the current mask of valid destinations for prop updates.
 */
void prop_update_connectivity(u8 dests)
{
	if (!(valid_dest_mask & ADS_BIT) && (dests & ADS_BIT)) {
		/* 
		 * device can now reach the service.
		 * send opcode 13 (enable service listener) after
		 * all the echoes have been sent
		 */
		send_ads_listen_cmd = 1;
	}
	if (!(dests & ADS_BIT)) {
		send_ads_listen_cmd = 0;
	}
	valid_dest_mask = dests;
}

/*
 * Receive NAK (negative acknowlegement) for a datapoint operation.
 */
int prop_nak(struct ayla_cmd *cmd, void *buf, size_t len)
{
	struct prop *prop = prop_active;
	struct ayla_tlv *tlv;

#ifdef AYLA_FILE_PROP
	if (prop_dp_nak(cmd, buf, len) != -2) {
		return 0;
	}
#endif /* AYLA_FILE_PROP */

	if (prop) {
		tlv = buf;
		if (len < sizeof(*tlv) || len < sizeof(*tlv) + tlv->len) {
			return -1;		/* XXX invalid NAK */
		}
		if (tlv->type != ATLV_ERR) {
			return 0; 
		}
		prop->send_err = *(u8 *)(tlv + 1);
		if (prop->send_err) {
			prop->send_err_counter++;
		}
	}

	return 0;
}

/*
 * prop_lookup_error() - Lookup a property table entry with a send error
 */
struct prop *prop_lookup_error(void)
{
	struct prop *prop;

	for (prop = prop_table; prop->name != NULL; prop++) {
		if (prop->send_err != 0) {
			return prop;
		}
	}
	return NULL;
}
