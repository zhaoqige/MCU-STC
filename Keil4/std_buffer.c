/*
 * STANDARD LOOP BACK BUFFER
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#include <string.h>

#include <reg51.h>
#include <intrins.h>

#include "stc_define.h"
#include "stc_uart.h"
#include "std_buffer.h"


#ifndef STD_BUFFER_LENGTH
	#define STD_BUFFER_LENGTH	33
#endif

uint  std_buffer_pos2write = 0;
uchar std_buffer[STD_BUFFER_LENGTH] = {""};//{"+wbsts:1,0,33,24\n"};



void initBuffer(void)
{
	std_buffer_pos2write = 0; // data2write index
	memset(std_buffer, 0, STD_BUFFER_LENGTH);
}
int readDataLength(void)
{
	return std_buffer_pos2write;
}
/*int  readDataFromBuffer(uchar * p_buffer, uint u_buffer_length, uint * p_data_length)
{
	uint data_length = MIN(std_buffer_pos2write, u_buffer_length);
	if (std_buffer_pos2write) {
		memcpy(p_buffer, std_buffer, data_length);
		(*p_data_length) = data_length;
	} else {
		(*p_data_length) = 0;
		return -1; // no data
	}
	return 0;
}*/
void moveBuffer(uint x)
{
	uint i, data2move = 0, std_buffer_length = STD_BUFFER_LENGTH;
	if (std_buffer_length <= x) {
		initBuffer();
	} else {
		std_buffer_pos2write -= x; // left data counter
		//data2move = MIN(std_buffer_pos2write, std_buffer_length);
		data2move = std_buffer_pos2write;
		for(i = 0; i < data2move; i ++) {
			std_buffer[i] = std_buffer[i+x];
		}
		for(i = std_buffer_pos2write; i < std_buffer_length; i ++) {
			std_buffer[i] = '\0';
		}
	}	
}
int  saveByte2Buffer(uchar c)
{
	uint std_buffer_length = STD_BUFFER_LENGTH;
	//* keep last byte empty
	if (std_buffer_pos2write + 1 >= std_buffer_length) {
		//uart_send('|');
		return -1;
	}
	std_buffer[std_buffer_pos2write] = c;
	std_buffer_pos2write += 1;		

	return 0;
}
