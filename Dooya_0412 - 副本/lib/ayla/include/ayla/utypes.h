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
#ifndef __AYLA_UTYPES_H__
#define __AYLA_UTYPES_H__

#ifndef HAVE_UTYPES
typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
#if 0
typedef unsigned long long u64;
#endif

typedef signed char	s8;
typedef short		s16;
typedef long	    s32;
#if 0
typedef long long	s64;
#endif
#endif

typedef u16		be16;
typedef u32		be32;
#if 0
typedef u64		be64;
#endif 

#define MAX_U8		0xffU
#define	MAX_U16		0xffffU
#define MAX_U32		0xffffffffU

#define MAX_S8		0x7f
#define MAX_S16		0x7fff
#define MAX_S32		0x7fffffff

#define MIN_S8		((s8)0x80)
#define MIN_S16		((s16)0x8000)
#define MIN_S32		((s32)0x80000000)

/* XXX assumes little-endian system */
static /* inline */ be16 htons(short x)
{
    return (be16)((x << 8) | ((x >> 8) & 0xff));
}

static /* inline */ u16 ntohs(be16 x)
{
    return ((u16)x << 8) | (((u16)x >> 8) & 0xff);
}

static /* inline */ be32 htonl(u32 x)
{
    return (be32)((x << 24) |
        ((x << 8) & 0xff0000) |
        ((x >> 8) & 0xff00) |
        ((x >> 24) & 0xff));
}

static /* inline */ u32 ntohl(be32 x)
{
     return (u32)(((u32)x << 24) |
        (((u32)x << 8) & 0xff0000) |
        (((u32)x >> 8) & 0xff00) |
        (((u32)x >> 24) & 0xff));
}

#ifndef PACKED
#define PACKED __attribute__((__packed__))
#endif /* PACKED */

#ifndef ARRAY_LEN
#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#endif /* ARRAY_LEN */

#define TSTAMP_CMP(cmp, a, b)   ((s32)((a) - (b)) cmp 0)

#define TSTAMP_LT(a, b)		((s32)((a) - (b)) < 0)
#define TSTAMP_GT(a, b)		((s32)((a) - (b)) > 0)
#define TSTAMP_LEQ(a, b)	((s32)((a) - (b)) <= 0)
#define TSTAMP_GEQ(a, b)	((s32)((a) - (b)) >= 0)

#ifndef ABS
#define	ABS(a)			(((s32)(a) < 0) ? -(a) : (a))
#endif /* ABS */
 
#endif /* __AYLA_UTYPES_H__ */
