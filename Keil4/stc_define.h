/*
 * COMMON DEFINE
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#ifndef _STC_DEFINE_H_
#define _STC_DEFINE_H_


	typedef unsigned int	uint;
	typedef unsigned char	uchar;
	
	
	#define MIN(x, y) (x < y ? x : y)
	#define MAX(x, y) (x > y ? x : y)


	#define PIN_ENABLE		1
	#define PIN_DISABLE		0
	#define BTN_ENABLE		0
	#define BTN_DISABLE		1

	sbit LCD_RS = P2^7;
	sbit LCD_RW = P2^6;
	sbit LCD_EP = P2^5;

	sbit btn_1 = P2^1;
	sbit btn_2 = P2^2;
	sbit btn_3 = P2^3;
	sbit btn_4 = P2^4;
	
	sbit beep = P2^0;

	#define STD_BUFFER_LENGTH 65


#endif