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
#ifndef __AYLA_SPI_MCU_H__
#define __AYLA_SPI_MCU_H__

/*
 * Module slave status byte.
 */
#define ASPI_MSTAT_INVAL_BIT 7	/* if set, indicates invalid status */
#define ASPI_MSTAT_DOWN_BIT 4	/* data service not available */
#define ASPI_MSTAT_ADS_BUSY_BIT 3 /* ADS send not available */
#define ASPI_MSTAT_ERR_BIT 2	/* error on last command */
#define ASPI_MSTAT_ATTN_BIT 1	/* pending message to master */
#define ASPI_MSTAT_BUSY_BIT 0	/* busy processing command */

#define ASPI_MSTAT_NONE 0		/* no status */
#define ASPI_MSTAT_INVAL (1 << ASPI_MSTAT_INVAL_BIT)
#define ASPI_MSTAT_DOWN (1 << ASPI_MSTAT_DOWN_BIT)
#define ASPI_MSTAT_ADS_BUSY (1 << ASPI_MSTAT_ADS_BUSY_BIT)
#define ASPI_MSTAT_ERR	(1 << ASPI_MSTAT_ERR_BIT)
#define ASPI_MSTAT_ATTN	(1 << ASPI_MSTAT_ATTN_BIT)
#define ASPI_MSTAT_BUSY	(1 << ASPI_MSTAT_BUSY_BIT)

/*
 * Ayla SPI frame definitions.
 * Frame is start byte followed by two-byte length.
 */
#define ASPI_NO_CMD	0	/* non-start value */

#define ASPI_CMD_MASK	0xc0	/* mask for MO commands */
#define ASPI_CMD_MO	0x80	/* master to slave transfer */
#define ASPI_CMD_MI	0xf1	/* slave to master transfer */
#define ASPI_CMD_MI_RETRY 0xf2	/* repeat last slave to master transfer */
#define ASPI_LEN_MASK	0x3f	/* length mask */
#define ASPI_LEN_MULT	8	/* length multiplier */

#define ASPI_LEN(x)	(((x) + ASPI_LEN_MULT - 1) / ASPI_LEN_MULT)

#define ASPI_XTRA_CMDS	40	/* number of repeated commands allowed */

#endif /* __AYLA_SPI_MCU_H__ */
