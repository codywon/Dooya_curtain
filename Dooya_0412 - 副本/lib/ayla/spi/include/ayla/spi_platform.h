/*
 * Copyright 2012 Ayla Networks, Inc.  All rights reserved.
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
#ifndef __SPI_PLATFORM_H__
#define __SPI_PLATFORM_H__

/*
 * Headers for platform-specific SPI Master functions
 */

/*
 * Send out 'byte' and return the incoming byte
 */
u8 spi_platform_io(u8);

/*
 * Send and receive the last byte of a SPI message, followed by the CRC byte.
 *
 * NB:  The setting of CRCNEXT must happen immediately after the last
 * byte is sent.  See RM0008.
 */
u8 spi_platform_io_crc(u8);

/*
 * Select slave
 */
void spi_platform_slave_select(void);

/*
 * Deselect slave
 */
void spi_platform_slave_deselect(void);

/*
 * Enable CRC
 */
void spi_platform_crc_en(void);

/*
 * Clear CRC status and return if error
 */
int spi_platform_crc_err(void);
			 
/*
 * Check if there is a receive pending
 */
int spi_platform_rx_pending(void);             

#endif /* __SPI_PLATFORM_H__ */
