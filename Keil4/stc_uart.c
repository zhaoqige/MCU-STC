/*
 * STC UART CONTROLS
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#include <string.h>

#include <reg51.h>
#include <intrins.h>

#include "stc_define.h"
#include "std_buffer.h"
#include "stc_uart.h"


extern uint  std_buffer_pos2write;
extern uchar std_buffer[];


// BASIC RS232 PARAMs
//#define UART_FOSC 			11059200L
//#define UART_BAUD 			9600
//#define UART_SEND_DOUBLE_CMD 
#define UART_RECV_COUNT_MAX	64


uchar	c_uartRecvByte = 0;
bit		b_uartRecvByte = 0;


void uart_init() {

	SCON = 0x50;
	TMOD |= 0x20;
	PCON |= 0x80;

	//TH1 = TL1 = -(SP_FOSC/12/32/SP_BAUD);
	//TH1 = 0xF3;	TL1 = 0xF3;	// baudrate: 0xf3,0xf3 4800@12M
	TL1 = 0xFA; // baudrate: 0xfa, 0xfa 9600@11.0592M
	TH1 = 0xFA;
	
	TR1 = 1;
	ES = 1;
	EA = 1;
}


void uart_send_byte(uchar c)
{
	SBUF = c;
	while(! TI);
	TI = 0;
}

void uart_send_hex(uchar * p_data, uint u_data_length)
{
	uint i = 0;
	for(i = 0; i < u_data_length; i ++) {
		uart_send_byte(p_data[i]);
	}
}

void uart_send(uchar * p_data)
{
	uart_send_hex(p_data, strlen(p_data));
}



//* INTERRUPT for RS232 receiving
void serv_intterrupt(void) interrupt 4 using 1 {
	ES = 0;
	if(RI == 1) {
		RI = 0;
		c_uartRecvByte = SBUF;
		b_uartRecvByte = 1;

		saveByte2Buffer(c_uartRecvByte);
		//uart_send_byte(c_uartRecvByte);
	}
	ES = 1;
}

  