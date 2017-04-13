/*
 * Copyright 2013 Ayla Networks, Inc.  All rights reserved.
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
#ifndef __AYLA_FLASH_LAYOUT_H__
#define __AYLA_FLASH_LAYOUT_H__

/*
 * Flash layout:
 *  0x08000000 - bootloader
 *  0x08001c00 - copy_progress
 *  0x08002000 - active image
 *  0x08020000 - inactive image
 *  0x0803e000 - scratch area (when switching bw active/inactive)
 *  0x08040000 - end of flash
 */
#define MCU_IMG_PROG_BAR	0x08001c00
#define MCU_IMG_ACTIVE		0x08002000
#define MCU_IMG_INACTIVE	0x08010000	//0x08020000-->0x08010000
#define MCU_IMG_SCRATCH		0x0801e000	//0x0803e000-->0x0801e000

#define MCU_IMG_MAX_SZ		0xe000		//0x1e000-->0xe000
#define MCU_IMG_BLK_SZ      0x400		//0x800-->0x400
#define MCU_IMG_SCRATCH_SZ  0x2000
#define MCU_IMG_COPY_BLK	(MCU_IMG_MAX_SZ / MCU_IMG_SCRATCH_SZ)

/*
 * Image header as used by demo code. Network byte order.
 */
#define IMAGE_MAGIC 		0xbfa43642  /* Random */
#ifndef HAVE_UTYPES
#define HAVE_UTYPES
typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;

typedef signed char	s8;
typedef short		s16;
typedef long		s32;
#endif /* HAVE_UTYPES */

struct image_hdr {
	u32 ih_magic;		/* IMAGE_MAGIC */
	u16 ih_crc;		/* CCITT crc-16 over the image (ih_crc = 0) */
	u16 ih_len;		/* length */
};

struct image {
	struct image_hdr i_hdr;
	u8 i_vers[72];		/* version string */
};
#define i_magic	i_hdr.ih_magic
#define i_crc	i_hdr.ih_crc
#define i_len	i_hdr.ih_len

#define IMAGE_HDR_OFF		0x150	//0x188
#define IMAGE_HDR_CRC_OFF	(IMAGE_HDR_OFF + 4)

enum mcu_boot_state {
	MCU_BOOT_OK = 0,
	MCU_BOOT_INACTIVE = 1,
	MCU_BOOT_TEST = 2,
	MCU_BOOT_FALLBACK_START = 3,
	MCU_BOOT_FALLBACK = 4
};

/*
 * Progress bar is kept in a block of flash. This is updated to make
 * sure that image is updated properly even with power outages in
 * the middle. MCU should always have a way to boot to a valid image.
 *
 * Progress bar is split into 2 byte chunks, as this is the minimum
 * updateable block. Bar is initialized to all-ff's (erased flash).
 * Progress is indicated by writing non-ff value to that location.
 * Offset Purpose
 *   0     Image swap started (state MCU_BOOT_INACTIVE)
 *   1     1st flash block copied from inactive to scratch
 *   2     1st flash block copied from active to inactive
 *   3     1st flash block copied from scratch to active
 *   ...
 *   repeated in groups of 3 until all flash blocks have been moved
 *   43   Testing boot to new image (state MCU_BOOT_TEST)
 * Once we're here, and the image booted ok, we'll clear the progress
 * bar. Upgrade was a success.
 *   44    Boot not successful, bootloader marks that it tried to
 *         boot image, but failed. Swap the old image back.
 *         (state MCU_BOOT_FALLBACK_START)
 *   45    1st flash block copied from inactive to scratch
 *   ...
 *   86    Fallback to old image done (state MCU_BOOT_FALLBACK).
 * If we're here, upgrade was not a success. Image failed to boot.
 * 
 */
#define MCU_PBAR_OFF_INACTIVE		0
#define MCU_PBAR_OFF_TEST			(1 + (MCU_IMG_COPY_BLK * 3))
#define MCU_PBAR_OFF_FALLBACK_START	(MCU_PBAR_OFF_TEST + 1)
#define MCU_PBAR_OFF_FALLBACK		(MCU_PBAR_OFF_FALLBACK_START + 1 + (MCU_IMG_COPY_BLK * 3))
#define MCU_PBAR_OFF_MAX			(MCU_PBAR_OFF_FALLBACK + 1)

#define MCU_PBAR_ISSET(a) (((u16 *)MCU_IMG_PROG_BAR)[a] != 0xffff)

/*
 * Template version is reported to service. This makes it possible
 * for service to update template associated with the device when
 * upgrade happens.
 */
#define MCU_IMG_TMPL_MAXLEN			32
#define MCU_IMG_TMPL_VER_LOC		(MCU_IMG_ACTIVE - MCU_IMG_TMPL_MAXLEN)

#endif /* __AYLA_FLASH_LAYOUT_H__ */
