/*
 * LCD GWS CONTROLS
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reg51.h>
#include <intrins.h>

#include "stc_define.h"
#include "stc_uart.h"
#include "std_drv_1602.h"

#include "wb_ui.h"



void delay(uint x);

void main(void)
{
	uint i;
	P0 = 0x00;
	P1 = 0x00;
	P2 = 0xFF;

	lcd_init();
	uart_init();

	for(i = 0;; i ++) {
		ui_calc_ui();
		delay(100);
	}
}



void delay(uint x)
{
	uint j;
	for(x; x > 0; x --)
		for(j = 100; j > 0; j --);
}