#ifndef	__SAC_PROTOCOL_I_H__
#define	__SAC_PROTOCOL_I_H__


struct tag_msg_101_000_build
{	/* 01 */
	u8	fan_speed_set			:1;	/* 0 = invalid,	1 =	avalid */
	u8	fan_speed				:7;
	/* 02 */
	u8	sleep_set				:1;
	u8	sleep					:7;	/* 0...8 */
	/* 03 */
	u8	fan_dir_set				:1;
	u8	fan_dir_ctrl			:1;	/* Not used	now. Replaced by Hor and Vet direction control */
	u8	power_set				:1;
	u8	power					:1;
	u8	work_mode_set			:1;
	u8	work_mode				:3;
	/* 04 */
	u8	temp_set				:1;
	u8	temp					:7;
	/* 05 */
	u8	humi_set_indoor_set		:1;
	u8	humi_set_indoor			:7;
	/* 06 */
	u8	temp_somato_indoor;
	/* 07 */
	u8	somato_comp_enable_set	:1;
	u8	somato_comp_enable		:1;
	u8	somato_comp_val_set		:1;
	u8	somato_comp_val			:5;
	/* 08 */
	u8	temptype_set			:1;
	u8	temptype				:1;
	u8	auto_manual				:1;
	u8	temp_comp_set			:1;
	u8	temp_comp				:4;
	/* 09 */
	u8	timer_set				:1;
	u8	timer_remain			:6;
	u8	timer_enable			:1;
	/* 10 */
	u8	rtc_hour_set			:1;
	u8							:2;
	u8	rtc_hour				:5;
	/* 11 */
	u8	rtc_minute_set			:1;
	u8							:1;
	u8	rtc_minute				:6;
	/* 12 */
	u8	rtc_on_enable_set		:1;
	u8	rtc_on_enable			:1;
	u8	rtc_on_hour_set			:1;
	u8	rtc_on_hour				:5;
	/* 13 */
	u8	rtc_on_minute_set		:1;
	u8							:1;
	u8	rtc_on_minute			:6;
	/* 14 */
	u8	rtc_off_enable_set		:1;
	u8	rtc_off_enable			:1;
	u8	rtc_off_hour_set		:1;
	u8	rtc_off_hour			:5;
	/* 15 */
	u8	rtc_off_minute_set		:1;
	u8							:1;
	u8	rtc_off_minute			:6;
	/* 16 */
	u8	fan_door_pos_set		:1;
	u8	fan_door_pos			:3;
	u8	humi_mode_set			:1;
	u8	humi_mode				:3;
	/* 17 */
	u8	e_heat_ctrl_set			:1;
	u8	e_heat_ctrl				:1;
	u8	natural_wind_ctrl_set	:1;
	u8	natural_wind_ctrl		:1;
	u8	fan_leftright_set		:1;
	u8	fan_leftright			:1;
	u8	fan_power_set			:1;
	u8	fan_power				:1;
	/* 18 */
	u8	run_mode_set			:1;
	u8	run_mode				:1;
	u8	temp_heatcold_set		:1;
	u8	temp_heatcold			:1;
	u8	eco_set					:1;
	u8	eco						:1;
	u8	energy_save_set			:1;
	u8	energy_save				:1;
	/* 19 */
	u8	clean_outdoor_set		:1;
	u8	clean_outdoor			:1;
	u8	clean_indoor_set		:1;
	u8	clean_indoor			:1;
	u8	change_wind_set			:1;
	u8	change_wind				:1;
	u8	fresh_ctrl_set			:1;
	u8	fresh_ctrl				:1;
	/* 20 */
	u8	smoke_remove_set		:1;
	u8	smoke_remove			:1;
	u8	voice_ctrl_set			:1;
	u8	voice_ctrl				:1;
	u8	fan_mute_set			:1;
	u8	fan_mute				:1;
	u8	smart_eye_set			:1;
	u8	smart_eye				:1;
	/* 21 */
	u8	display_in_out_ctrl_set	:1;
	u8	display_in_out_ctrl		:1;
	u8	LED_light_ctrl_set		:1;
	u8	LED_light_ctrl			:1;
	u8	screen_light_ctrl_set	:1;
	u8	screen_light_ctrl		:1;
	u8	backlight_set			:1;
	u8	backlight				:1;
	/* 22 */
	u8							:2;
	u8	right_fan_ctrl_set		:1;
	u8	right_fan_ctrl			:1;
	u8	left_fan_ctrl_set		:1;
	u8	left_fan_ctrl			:1;
	u8	filter_indoor_reset_set	:1;
	u8	filter_indoor_reset		:1;
	/* 23 */
	u8	reserve1;
	/* 24 */
	u8	screen_brightness_set	:1;
	u8	screen_brightness		:7;
	/* 25 */
	u8	reserve2;
	/* 26 */
	u8	reserve3;
	/* 27 */
	u8	reserve4;
	/* 28 */
	u8	reserve5;
	/* 29 */
	u8	reserve6;
	/* 30 */
	u8	reserve7;
} PACKED;

struct dooya_operation_4
{
	u8 current_present;
	u8 direction;
	u8 hand_pull;
	u8 status;
	u8 reserve5;
	u8 reserve6;
	u8 reserve7;
	u8 route_enable;
}PACKED;

struct dooya_error
{
	u8 error;
}PACKED;



struct tag_msg_101_000_parse
{	/* 01 */
	u8	speed_ctrl				:1;	/* 0 = manual, 1 = automatic */
	u8	fan_speed				:7;
	/* 02 */
	u8							:1;
	u8	sleep					:7;	/* 0...8 */
	/* 03 */
	u8							:1;
	u8	fan_dir_ctrl			:1;	/* Not used	now. Replaced by Hor and Vet direction control */
	u8							:1;
	u8	power					:1;
	u8	work_mode				:4;
	/* 04 ~	09 */
	u8	temp;
	u8	temp_in;
	u8	temp_coil_indoor;
	u8	humi_set_indoor;
	u8	humidity;
	u8	temp_somato_indoor;
	/* 10 */
	u8							:1;
	u8	somato_comp_enable		:1;
	u8							:1;
	u8	somato_comp_val			:5;
	/* 11 */
	u8							:1;
	u8	temptype				:1;
	u8							:2;
	u8	temp_comp				:4;
	/* 12 */
	u8							:1;
	u8	timer_remain			:6;	/* in 0.5 hour */
	u8	timer_enable			:1;
	/* 13 */
	u8	rtc_hour_avalid			:1;
	u8							:2;
	u8	rtc_hour				:5;
	/* 14 */
	u8	rtc_minute_avalid		:1;
	u8							:1;
	u8	rtc_minute				:6;
	/* 15 */
	u8	rtc_on_hour_avalid		:1;
	u8	rtc_on_enable			:1;
	u8							:1;
	u8	rtc_on_hour				:5;
	/* 16 */
	u8	rtc_on_minute_avalid	:1;
	u8							:1;
	u8	rtc_on_minute			:6;
	/* 17 */
	u8	rtc_off_hour_avalid		:1;
	u8	rtc_off_enable			:1;
	u8							:1;
	u8	rtc_off_hour			:5;
	/* 18 */
	u8	rtc_off_minute_avalid	:1;
	u8							:1;
	u8	rtc_off_minute			:6;
	/* 19 */
	u8							:1;
	u8	fan_door_pos			:3;
	u8							:1;
	u8	humi_mode				:3;
	/* 20 */
	u8	run_mode				:1;
	u8	temp_heatcold			:1;
	u8	eco						:1;
	u8	energy_save				:1;
	u8	e_heat_ctrl				:1;
	u8	natural_wind_ctrl		:1;
	u8	fan_leftright			:1;
	u8	fan_power				:1;
	/* 21 */
	u8	remove_smoke_ctrl		:1;
	u8	voice_ctrl				:1;
	u8	fan_mute				:1;
	u8	smart_eye_ctrl			:1;
	u8	clean_outdoor_ctrl		:1;
	u8	clean_indoor_ctrl		:1;
	u8	change_wind_ctrl		:1;
	u8	fresh_ctrl				:1;
	/* 22 */
	u8	pwr_meter_indoor_board	:1;
	u8	right_wind_ctrl			:1;
	u8	left_wind_ctrl			:1;
	u8	filter_indoor_status	:1;
	u8	display_temp_in_n_out	:1;
	u8	LED_ctrl				:1;
	u8	screen_light			:1;
	u8	backlight				:1;
	/* 23 */
	u8	eeprom_ota				:1;
	u8	es_or_mp				:1;
	u8	wifi_ctrl_before		:1;
	u8	irda_kb_ctrl_before		:1;
	u8							:4;
	/* 24 ~	25 */
	u8	indoor_warning_1;
	u8	indoor_warning_2;
	/* 26 */
	u8	compressor_freq_running;
	/* 27 */
	u8	compressor_freq_target;
	/* 28 */
	u8	driver_freq;
	/* 29 */
	u8	temp_env_outdoor;
	/* 30 */
	u8	temp_condenser_outdoor;
	/* 31 */
	u8	temp_compressor_exhaust_cur;
	/* 32 */
	u8	temp_compressor_exhaust_set;
	/* 33 */
	u8	outdoor_exp_valve_open_degree;
	/* 34 ~	45 */
	u8	power_supply_arg_1;			/* TODO	*/
	u8	power_supply_arg_2;
	u8	power_supply_arg_3;
	u8	power_supply_arg_4;
	u8	power_supply_arg_5;
	u8	power_supply_arg_6;
	u8	power_supply_arg_7;
	u8	power_supply_arg_8;
	u8	power_supply_arg_9;
	u8	power_supply_arg_10;
	u8	power_supply_arg_11;
	u8	power_supply_arg_12;
	/* 46 */
	u8	outdoor_fan_status		:3;
	u8	outdoor_unit_status		:1;
	u8	four_way_valve_status	:1;
	u8							:3;
	/* 47 ~	57 */
	u8	outdoor_status_1;
	u8	outdoor_status_2;
	u8	outdoor_warning_1;
	u8	outdoor_warning_2;
	u8	outdoor_warning_3;
	u8	outdoor_warning_4;
	u8	outdoor_warning_5;
	u8	outdoor_warning_6;
	u8	outdoor_warning_7;
	u8	outdoor_warning_8;
	u8	outdoor_warning_9;
	/* 58 */
	u8	fan_rpm_indoor;
	/* 59 */
	u8	fan_rpm_outdoor;
	/* 60 */
	u8	pm25_level				:4;
	u8							:3;
	u8	pm25_detect_avalid		:1;
	/* 61 */
	u8	pm25_percent;				/* 0 ~ 100 */
	/* 62 */
	u8	heat_8C_ctrl			:1;
	u8	display_brightness		:7;
} PACKED;

struct tag_msg_102_000_build
{
	u8	auto_manual				:1;	/* 0 = automatic, 1	= manual */
	u8							:7;
} PACKED;

#define	tag_msg_102_000_parse	tag_msg_101_000_parse

struct tag_msg_102_001_parse
{
	u8 frontlight_r;
	u8 frontlight_g;
	u8 frontlight_b;
	u8 backlight_r;
	u8 backlight_g;
	u8 backlight_b;
	u8 reserve1;
	u8 reserve2					:6;
	u8 wind_dir_follow			:2;
	u8 reserve3;
	u8 reserve4;
} PACKED;

#endif /* __SAC_PROTOCOL_I_H__ */
