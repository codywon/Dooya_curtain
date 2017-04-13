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
#ifndef __AYLA_CRC_H__
#define __AYLA_CRC_H__

#define CRC8_POLY	7
#define CRC8_INIT	0xffU

u8 crc8(const void *buf, size_t len, u8 init_crc);

#define CRC8_DALLAS_POLY 0x8cU
#define CRC8_DALLAS_INIT 0xffU

u8 crc8_dallas(const void *buf, size_t len, u8 init_crc);

#define	CRC16_POLY	0x1021U		/* CCITT polynomial */
#define	CRC16_INIT	0xffffU

u16 crc16(const void *buf, size_t len, u16 init_crc);

#define	CRC32_POLY	0xedb88320U	/* polynomial for sending LSB first */
#define	CRC32_INIT	0xffffffffU

u32 crc32(const void *buf, size_t len, u32 init_crc);

void crc32_init(void);			/* possibly needed for h/w accel */

#endif /* __AYLA_CRC_H__ */

