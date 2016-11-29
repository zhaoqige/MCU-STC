/*
 * STD 1602 DRIVER/CONTROLS
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#include <stdio.h>

#include <intrins.h>
#include <reg51.h>

#include "stc_define.h"
#include "stc_uart.h"
#include "std_drv_1602.h"


//* lcd controls
#define LCD_LINE_LENGTH_MAX	16

#define LCD_BUSY_BIT_MASK	0x80
#define LCD_BASIC_SETTINGS	0x38

#define LCD_EDIT_NOMARK		0x0C
#define LCD_EDIT_MARK		0x0F

#define LCD_MOVE_RNEXT		0x06
#define LCD_ECHO_CLEAR		0x01

#define LCD_ECHO_LINE1		0x00
#define LCD_ECHO_LINE2		0x40



void lcd_delay(uint i);

//* data[7] = 1: busy
//* data[7] = 0: ready
bit  lcd_busy_now(void)
{
	bit is_busy = 0; //return is_busy;

	P0 = 0x00;

	LCD_RS = PIN_DISABLE;
	LCD_RW = PIN_ENABLE;
	LCD_EP = PIN_ENABLE;

	lcd_delay(4);

	//is_busy = (P0 & 0x80) ? 1 : 0; //0x80
	is_busy = (bit) (P0 & LCD_BUSY_BIT_MASK); //0x80
	LCD_EP = PIN_DISABLE;

	return is_busy;
}

void lcd_cmd(char cmd)
{
	while(lcd_busy_now());

	LCD_RS = PIN_DISABLE;
	LCD_RW = PIN_DISABLE;
	LCD_EP = PIN_DISABLE;
	lcd_delay(2);

	P0 = cmd;
	lcd_delay(4);

	LCD_EP = PIN_ENABLE;
	lcd_delay(4);

	LCD_EP = PIN_DISABLE;
}

void lcd_set_pos(char pos)
{
	lcd_cmd(pos | LCD_BUSY_BIT_MASK); //0x80
}

void lcd_data(char byte)
{
	while(lcd_busy_now());

	LCD_RS = PIN_ENABLE;
	LCD_RW = PIN_DISABLE;
	LCD_EP = PIN_DISABLE;
	P0 = byte;
	lcd_delay(4);

	LCD_EP = PIN_ENABLE;

	lcd_delay(4);

	LCD_EP = PIN_DISABLE;
}

void lcd_init(void)
{
	lcd_cmd(LCD_BASIC_SETTINGS); // 0x38: total 16*2, 5*7 each, 8 bit data
	lcd_delay(1);

	lcd_cmd(LCD_EDIT_NOMARK); // 0x0C: ECHO on, no CURSE
	lcd_delay(1);

	lcd_cmd(LCD_MOVE_RNEXT); // 0x06: move curse to RIGHT NEXT
	lcd_delay(1);

	lcd_cmd(LCD_ECHO_CLEAR); // 0x01: clear
	lcd_delay(1);
}


void lcd_print(uint x, uint y, char string[17])
{
	unsigned int i;

	//uart_send(string);

	lcd_cmd(LCD_MOVE_RNEXT); // 0x06

	if (y == 1) {
		lcd_set_pos(LCD_ECHO_LINE1 + x);
	} else {
		lcd_set_pos(LCD_ECHO_LINE2 + x);
	}

	i = 0;
	while(string[i] != '\0' && i < LCD_LINE_LENGTH_MAX) {
		lcd_data(string[i]);
		i ++;
	}
}

void lcd_edit_mark(uchar pos)
{
	lcd_cmd(LCD_EDIT_MARK);
	if (pos) lcd_set_pos(pos);
}

void lcd_edit_done(void)
{
	lcd_cmd(LCD_EDIT_NOMARK);
}


void lcd_delay(uint i)
{
	while(i--)
		_nop_();
}