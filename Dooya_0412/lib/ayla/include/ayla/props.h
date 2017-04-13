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
#ifndef __AYLA_PROPS_H__
#define __AYLA_PROPS_H__

#define PROP_NAME_LEN	28
#define PROP_LOC_LEN	64

/* Operations for a property */
enum prop_ops {
	POST,
	GET,
	ECHO
} PACKED;


struct prop {
	const char *name;
	u8 beep;
	enum ayla_tlv_type type;
	/* callback to handle an incoming value */
	void (*set)(struct prop *prop, void *arg, void *val, size_t val_len);
	/* callback to handle an outgoing value */
	int (*send)(struct prop *prop, void *arg);
	void *arg;		/* argument to pass the the set and send callbacks */
	size_t val_len;		/* size of property data in bytes, or 0 if not fixed size */
	u8 fmt_flags;		/* formatting flags to aid in generating interfaces */
	size_t name_len;	/* length of name string */
	u8 send_mask;		/* send is requested/required for this mask */
	u8 send_err;		/* error reported in NAK for send */
	u16 send_err_counter;	/* counter of number of send failures */
	enum prop_ops curr_op;	/* current operation for this property */
	u16 req_id;		/* current request ID */
	unsigned int get_val:1;	/* value from ADS is needed for this prop */
	unsigned int echo:1;	/* the prop update needs to be echoed to ADS */
};

/*
 * For properties with large data points, prop_set_blob() and 
 * prop_send_blob() functions can be used.
 * This state structure is needed for each such property.
 */
struct prop_dp {
	enum prop_dp_state {
		DS_IDLE = 0,	/* nothing to do */
		DS_CREATE,	/* send datapoint create */
		DS_CREATE_RESP,	/* wait for create response */
		DS_SEND,	/* send datapoint value */
		DS_REQUEST,	/* request datapoint */
		DS_RECV,	/* receive datapoint value */
		DS_FETCHED,	/* send datapoint fetched opcode */
		DS_ABORTING	/* aborting the datapoint operation */
	} state;
	struct prop *prop;	/* associated property entry */
	u32 next_off;		/* next offset to transfer */
	u32 tot_len;		/* total length expected (if known) */
	u16 req_id;		/* current request ID */
	u8 aborted;		/* 1 if last op for this prop was aborted */
	u8 loc_len;		/* length of location string */
	char loc[PROP_LOC_LEN];	/* location / data point key */
	/* callback to handle incoming data */
	int (*prop_get)(struct prop *prop, size_t off, void *buf, size_t len);
	/* callback to handle outgoing data */
	int (*prop_set)(struct prop *prop, size_t off, void *buf, size_t len, u8 eof);
};

/*
 * Argument to send_prop calls.
 */
struct send_arg {
	u32 cont;	/* continuation token, if non-zero */
	u32 offset;	/* offset to start transfer from */
	u16 req_id;	/* host-order request ID */
	unsigned int resp:1;	/* 1 if its a response */
};

/*
 * prop_lookup() - Lookup a property table entry by name.
 *
 * Returns NULL if not found.
 */
struct prop *prop_lookup(const char *name);

/*
 * prop_lookup_len() - Lookup a property table entry by name and name len.
 *
 * Returns NULL if not found.
 */
struct prop *prop_lookup_len(const char *name, size_t name_len);

/*
 * prop_send(struct prop *, const void *, size_t, void *)
 * Returns non-zero error if property wasn't sent.
 */
int prop_send(struct prop *, const void *val, size_t val_len, void *arg);

/*
 * prop_send_generic(struct prop *, void *)
 * Send prop->arg as property's value.
 * Determine the val_len by looking at its type.
 * Can be used for ATLV_BOOL, ATLV_INT, ATLV_UTF8, and ATLV_CENTS.
 */
int prop_send_generic(struct prop *prop, void *arg);

/*
 * Internal routine to swap bytes for sending or after receiving.
 * On a big-endian micro, this would be a no-op.
 */
void prop_swap(struct prop *prop, void *valp);

/*
 * Schedule property to be sent to module.
 */
void prop_send_req(const char *name);

/*
 * Schedule property to be sent only to service
 */
void prop_send_req_to_ads_only(const char *name);

/*
 * prop_poll() - send properties with send_req set.
 */
void prop_poll(void);

/*
 * Check if there are any properties with pending request.
 */
int prop_pending(void);


/*
 * Check whether a particular property update has been sent.
 * Returns -1 if transmission is still in progress,
 * 0 if it completed ok and > 0 if module NAKed.
 */
int prop_send_done(struct prop *prop);

/*
 * Receive NAK (negative acknowlegement) for a datapoint operation.
 */
int prop_nak(struct ayla_cmd *cmd, void *buf, size_t len);

/*
 * prop_lookup_error() - Lookup a property table entry with a send error
 */
struct prop *prop_lookup_error(void);

/*
 * Update the current mask of valid destinations for prop updates.
 */
void prop_update_connectivity(u8 dests);

/*
 * prop_request_value(const char *).
 * Request the value of the specified property from ADS.
 * If name is null, ask for all "to-device" properties.
 * If the name isn't in the prop table, it'll return -1.
 */
int prop_request_value(const char *name);

/*
 * Notify of failure to send or req prop from service
 */
void prop_notify_failure(u16 req_id, struct prop *prop, u8 dests);

/*
 * Setup a retry for a prop_send during the next prop_poll
 */
void prop_setup_retry(struct prop *prop, void *arg, u8 is_resp);

/*
 * Check if the prop state machine is ready to immediately respond to
 * a property request
 */
int prop_is_busy(void);

/*
 * The property table.
 * This is supplied and initialized by the application.
 */
extern struct prop prop_table[];

extern u8 prop_count;
extern u8 valid_dest_mask;

extern struct prop_dp *prop_dp_active;

#endif /*  __AYLA_PROPS_H__ */
