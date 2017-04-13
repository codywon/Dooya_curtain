/*
 * Copyright 2014 Ayla Networks, Inc.  All rights reserved.
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
#ifndef __AYLA_CONF_ACCESS_H__
#define __AYLA_CONF_ACCESS_H__

/*
 * In conf_access.c.
 */
extern void (*conf_ota_cb)(int type, void *buf, size_t len);
extern char conf_dsn[];
extern char conf_model[];

#ifdef AYLA_REG_TOKEN
extern char conf_reg_token[];
#endif

/*
 * Receive a configuration item.
 */
void conf_rx(void *buf, size_t len);

/*
 * enum conf_token conf_tokens[] = {CT_token0, CT_token1};
 * int ntokens = sizeof (conf_tokens) / sizeof (conf_tokens[0];
 * int val_type;
 * int val;
 * int val_sz = sizeof (val);
 *
 * conf_write(conf_tokens, ntokens, val_type, val, val_sz, ...);
 *
 * Arguments to conf_write are repeated in a set of 5,
 * 1. config tokens,
 * 2. number of config tokens,
 * 3. config variable type id
 * 4. pointer to value conf variable should be set to
 * 5. size of the conf variable value
 */
int conf_write(int cnt, ...);

/*
 * Send GET_CONF request.
 */
int conf_read(const enum conf_token * const tokens, unsigned int ntokens,
    void (*cb)(void *buf, size_t len));

void conf_set_cb(void (*cb)(void *buf, size_t len));
void conf_poll(void);

#ifdef AYLA_WIFI_JOIN

extern char conf_wifi_ssid[];
extern s8 conf_wifi_bars;
extern u8 conf_wifi_ap_mode;

/*
 * Ask module for wifi information every 'poll_secs'
 */
u8 conf_wifi_poll(u8 poll_secs);

/*
 * Sends a wifi join command over Serial. This'll get module to attempt to
 * join the specified network.
 */
int conf_wifi_join(char *ssid, int ssid_len, char *key, int klen, u8 sec_type);

/*
 * Sends a wifi delete command over Serial. This'll get module to leave and
 * forget the specified network.
 */
int conf_wifi_leave(char *ssid, int slen);

/*
 * This routine gets module to start it's wifi accesspoint. AP mode
 * will stay in effect until next restart, or until there is a wifi
 * configuration change (e.g. new network configuration gets added).
 */
int conf_wifi_start_ap(void);
#endif

#endif /* __AYLA_CONF_ACCESS_H__ */
