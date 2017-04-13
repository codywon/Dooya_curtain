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
#ifndef __AYLA_UART_H__
#define __AYLA_UART_H__

void uart_init(void);

/*
 * Put the received packets in a buffer. Once we see a whole frame,
 * call the process function to parse/interpret the frame.
 */
void uart_recv(u8 dr);

/*
 * Transmit packet if needed
 */
u8 *uart_tx(void);

/*
 * Return a buffer if available. The data going into this packet
 * SHOULD NOT be a property update or any data thats being
 * transmitted out. This function can be used for transmits like
 * configuration changes and error messages.
 */
void *uart_tx_buf_get(size_t len, void (*cb)(void));

/*
 * Return a buffer if available. Wait for confirmation from module
 * before making this buffer available for anything else that requires
 * confirmation.
 */
void *uart_tx_buf_get_for_props(size_t len, void (*cb)(void));

/*
 * Returns 1 if a prop update hasn't finished sending/receiving confirmation
 */
int uart_is_tx_busy(void);

/*
 * Resize the length of the tx data packet
 */
void uart_tx_buf_trim(size_t len);

/*
 * Transmit data over uart if tx buffer is available.
 */
int uart_tx_buf_send(int confirm_needed, u16 req_id);

/*
 * Received confirm or nak packet from module. See if its the confirmation
 * we are looking for.
 */
void uart_confirm_or_nak_recvd(u16 req_id);

#ifdef PLATFORM_LINUX
/*
 * Show UART statistics
 */
void uart_print_stats(void);
#endif

/******************* Platform-specific function headers *******************/
/*
 * Set the UART/GPIO settings
 */
void uart_platform_init(void);

#ifdef PLATFORM_LINUX
/*
 * Cleanup the serial device and restore original port configuration
 */
void uart_platform_cleanup(void);
#endif

/*
 * Poll for data. If RXNE, receive the packet.
 */
void uart_platform_poll(void);

#endif /* __AYLA_UART_H__ */
