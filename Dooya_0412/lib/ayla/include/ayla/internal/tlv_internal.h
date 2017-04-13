/*
 * Copyright 2014 Ayla Networks, Inc.  All rights reserved.
 */
#ifndef __AYLA_TLV_INTERNAL_H__
#define __AYLA_TLV_INTERNAL_H__

/*
 * Get the first TLV of the specified type from a received packet.
 */
struct ayla_tlv *tlv_get(enum ayla_tlv_type type, void *buf, size_t len);

/*
 * Place TLV of a given type to a buffer.
 */
size_t tlv_put(void *buf, enum ayla_tlv_type type, const void *val, size_t len);

#endif /* __AYLA_TLV_INTERNAL_H__ */
