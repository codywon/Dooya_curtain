/*
 * Copyright 2011-2013 Ayla Networks, Inc.  All rights reserved.
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
#include <string.h>
#include <ayla/mcu_platform.h>
#include <mcu_io.h>
#include <stm32.h>

#ifdef DEMO_CONF
#include <ayla/conf_token.h>
#include <ayla/conf_access.h>
#endif

#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/serial_msg.h>

#ifdef DEMO_UART
#include <ayla/uart.h>
#else
#include <spi_platform_arch.h>
#endif /* DEMO_UART */

#ifdef DEMO_FILE_PROP
#include <ayla/prop_dp.h>
#include <demo_stream.h>
#endif /* DEMO_FILE_PROP */

#ifdef DEMO_FILE_PROP
#define VERSION "Dooya 1.0 "
#else
#define VERSION "ayla_smart 1.0"
#endif /* DEMO_FILE_PROP */

#ifdef DEMO_SCHED_LIB
#include <ayla/schedeval.h>
#include <ayla/sched.h>
#include <ayla/cmp.h>
#endif /* DEMO_SCHED_LIB */

#ifdef DEMO_IMG_MGMT
#include <flash_layout.h>
#define BUILD_DATE  "@"  __DATE__ " " __TIME__
#ifdef AYLA_KEIL

#include "app_property.h"

/*
 * Image header location is fixed.
 */
#define IMG_HDR_LOC			(MCU_IMG_ACTIVE + IMAGE_HDR_OFF)
#define IMG_HDR_VER_LOC		(IMG_HDR_LOC + sizeof(struct image_hdr))

const struct image_hdr __img_hdr
			__attribute__((used))
			__attribute((at(IMG_HDR_LOC)));
const char version[72] __attribute((at(IMG_HDR_VER_LOC))) =
	 VERSION " " BUILD_DATE;
#else
const char version[] __attribute__((section(".version"))) =
	VERSION " " BUILD_DATE;
#endif /* AYLA_KEIL */
#else
const char version[] = VERSION;
#endif /* DEMO_IMG_MGMT || AYLA_BUILD_VERSION */

static u8 factory_reset;

#ifdef DEMO_IMG_MGMT
extern u8 boot2inactive;
extern u8 template_req;
void mcu_img_mgmt_init(void);
int send_inactive_version(struct prop *, void *arg);
void set_boot2inactive(struct prop *, void *arg, void *valp, size_t len);
void template_version_sent(void);
#endif

#ifdef DEMO_SCHED_LIB
struct sched_prop sched_table[10];

void set_schedule(struct prop *prop, void *arg, void *valp, size_t len)
{
	struct sched_prop *p_sched;

	p_sched = (struct sched_prop *)prop->arg;

	if (len > sizeof(p_sched->tlvs))
	{
		len = sizeof(p_sched->tlvs);
	}

	memcpy(p_sched->tlvs, valp, len);
	p_sched->len = len;

	sched_run_all(NULL);
}
#endif /* DEMO_SCHED_LIB */

int send_version(struct prop *prop, void *arg)
{
	return prop_send(prop, version, strlen(version), arg);
}

/*
 * Blue button push observed by interrupt handler.
 * Callers are in stm32.c
 */
void demo_set_button_state(u8 button_value)
{
#if 0
	blue_button = button_value;
	prop_table[PROP_BUTTON].send_mask = valid_dest_mask;
#endif
}

void app_init(void);
void app_poll(void);

int main(int argc, char **argv)
{
	struct prop *prop;

#ifdef RTT
	SEGGER_RTT_Init();
	log((0, "\n\n\n\n\nSEGGER RTT is ready!\r\n"));
#endif
	feature_mask |= MCU_LAN_SUPPORT;
#ifdef DEMO_IMG_MGMT
	mcu_img_mgmt_init();
	feature_mask |= MCU_OTA_SUPPORT;
#endif
#ifdef DEMO_SCHED_LIB
	feature_mask |= MCU_TIME_SUBSCRIPTION;
#endif
	mcu_io_init();
#ifdef DEMO_UART
	feature_mask |= MCU_DATAPOINT_CONFIRM;
	uart_init();
#else
	spi_platform_init();
#endif
	stm32_reset_module();
	stm32_init();
	factory_reset = stm32_factory_reset_detect();
#ifdef DEMO_FILE_PROP
	demo_stream_init();
#endif /* DEMO_FILE_PROP */
	app_init();
	
	while( 1 )
	{
		if (stm32_ready())
		{
			if (factory_reset && !serial_tx_cmd(ACMD_LOAD_FACTORY, NULL, 0))
			{
				factory_reset = 0;
				stm32_set_factory_rst_led(0);
				while (stm32_ready())
				{
					serial_poll();
				}
			}
#ifdef DEMO_CONF
			conf_poll();
#endif
			prop_poll();
			serial_poll();
#ifdef DEMO_SCHED_LIB
			if (sched_next_event_tick && (tick == sched_next_event_tick || cmp_gt(tick, sched_next_event_tick)))
			{
				sched_run_all(&sched_next_event_tick);
			}
#endif
		}

#ifdef DEMO_IMG_MGMT
		if (template_req && prop_send_done(prop_lookup(PN_OEM_HOST_VERSION)) == 0)
		{
			/*
			 * Template version number has been sent.
			 */
			template_version_sent();
		}
#endif
		prop = prop_lookup_error();
		if (prop != NULL)
		{
			/*
			 * Property send has failed with error code.
			 * Error code is available in prop->send_err
			 *
			 * Insert logic here to handle the failure.
			 */
			prop->send_err = 0;
		}

		app_poll();
	}
}
