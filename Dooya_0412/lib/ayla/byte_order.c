
/*
 * Copyright 2011-2014 Ayla Networks, Inc.  All rights reserved.
 */
#include <ayla/mcu_platform.h>
#include <ayla/byte_order.h>

int get_ua_with_len(void *src, u8 len, u32 *dest)
{
	switch (len) {
	case 1:
		*dest = *(u8 *)src;
		break;
	case 2:
		*dest = get_ua_be16(src);
		break;
	case 4:
		*dest = get_ua_be32(src);
		break;
	default:
		return -1;
	}
	return 0;
}

