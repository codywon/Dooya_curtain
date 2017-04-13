/*
 * Copyright 2014 Ayla Networks, Inc.  All rights reserved.
 */
#ifndef __AYLA_UTF8_INTERNAL_H__
#define __AYLA_UTF8_INTERNAL_H__

/*
 * Get multiple UTF-8 wide characters from a buffer.
 * Returns the number of characters put in result, or -1 on error.
 */
int utf8_gets(u32 *result, int rcount, u8 *buf, size_t len);

#endif /* __AYLA_UTF8_INTERNAL_H__ */
