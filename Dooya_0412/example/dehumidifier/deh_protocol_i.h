#ifndef	__DEH_PROTOCOL_I_H__
#define	__DEH_PROTOCOL_I_H__

struct tag_msg_101_000_build
{	/* 01 */
	u8	fan_speed_set			:1;	/* 0 = invalid,	1 =	avalid */
	u8	fan_speed				:3;
	u8							:4;
	/* 02 */
	u8	work_mode_set			:1;
	u8	work_mode				:3;
	u8							:1;
	u8	temptype_set			:1;
	u8	temptype				:1;
	u8	auto_manual				:1;
	/* 03 */
	u8	timer_set				:1;
	u8	timer_remain			:6;
	u8	timer_enable			:1;
	/* 04 */
	u8	humidity_set			:1;
	u8	humidity				:7;
	/* 05 */
	u8	e_heat_temp_set			:1;
	u8	e_heat_temp				:7;
	/* 06 */
	u8	negative_ion_set		:1;
	u8	negative_ion			:1;
	u8	water_pump_set			:1;
	u8	water_pump				:1;
	u8	e_heat_set				:1;
	u8	e_heat					:1;
	u8	power_set				:1;
	u8	power					:1;
	/* 07 */
	u8	reserve1;
	/* 08 */
	u8	reserve2;
} PACKED;

struct tag_msg_101_000_parse
{	/* 01 */
	u8							:1;
	u8	fan_speed				:3;
	u8							:4;
	/* 02 */
	u8							:1;
	u8	work_mode				:3;
	u8							:2;
	u8	temptype				:1;
	u8							:1;
	/* 03 */
	u8							:1;
	u8	timer_remain			:6;
	u8	timer_enable			:1;
	/* 04 */
	u8	humidity;
	/* 05 */
	u8	e_heat_temp;
	/* 06 */
	u8	temp_indoor;
	/* 07 */
	u8							:2;
	u8	prev_wifi_ctrl			:1;
	u8	prev_ir_key_ctrl		:1;
	u8	negative_ion			:1;
	u8	water_pump				:1;
	u8	e_heat					:1;
	u8	power					:1;
	/* 08 */
	u8							:2;
	u8	e_water_pump			:1;
	u8	e_water_full			:1;
	u8	e_indoor_temp_sensor	:1;
	u8	e_pipe_temp_sensor		:1;
	u8	e_humidity_sensor		:1;
	u8	e_filter_clean			:1;
	/* 09 */
	u8	humidity_indoor;
} PACKED;

struct tag_msg_102_000_build
{
	u8	auto_manual				:1;	/* 0 = automatic, 1	= manual */
	u8							:7;
} PACKED;

#define	tag_msg_102_000_parse	tag_msg_101_000_parse

#endif /* __DEH_PROTOCOL_I_H__ */
