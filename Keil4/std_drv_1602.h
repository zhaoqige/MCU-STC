/*
 * STD 1602 DRIVER/CONTROLS
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#ifndef _STD_DRV_1602_H_
#define _STD_DRV_1602_H_


	  bit  lcd_busy_now(void);
	  void lcd_cmd(char cmd);
	  void lcd_set_pos(char pos);
	  void lcd_data(char byte);
	  void lcd_init(void);
	  void lcd_print(uint x, uint y, char string[17]);
	  void lcd_edit_mark(uchar pos);
	  void lcd_edit_done(void);


#endif