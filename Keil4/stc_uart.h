/*
 * STC UART CONTROLS
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#ifndef _STC_UART_H_
#define _STC_UART_H_


	void uart_init();
	void uart_send_byte(uchar c);
	void uart_send_hex(uchar * p_data, uint u_data_length);
	void uart_send(uchar * p_data);


#endif