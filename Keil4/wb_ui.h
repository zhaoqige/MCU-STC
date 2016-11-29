/*
 * WB UI CONTROL
 * by Qige from 6Harmoncs @ 2016.03.14
 */

#ifndef _WB_UI_H_
#define _WB_UI_H_

	void ui_calc_ui(void);
	static uint ui_detect_btn(void);
	static void ui_calc_pages(uint btn);
	static void ui_echo_page(void);

	static void ui_calc_screen_echo_loading(uint btn);
	static void ui_calc_scrren_echo_sts(uint btn);
	static void ui_calc_scrren_echo_gws(uint btn);
	static void ui_calc_screen_edit_txpwr(uint btn);
	static void ui_calc_screen_edit_channel(uint btn);
	static void ui_calc_screen_edit_chanbw(uint btn);
	static void ui_calc_screen_edit_region(uint btn);
	static void ui_calc_screen_edit_mode(uint btn);


	static void gws_ui_init(void);
	static uchar * gws_mode_get(void);

	void wbp_parser(uchar * p_data, uint u_data_length, uint * p_move);

#endif