/*
 * STANDARD LOOP BACK BUFFER
 * by Qige from 6Harmoncs @ 2016.03.11
 */

#ifndef _STD_BUFFER_H
#define _STD_BUFFER_H


	void initBuffer(void);
	int  readDataLength();
	int  readDataFromBuffer(uchar * p_buffer, uint u_buffer_length, uint * p_data_length);
	void moveBuffer(uint x);
	int  saveByte2Buffer(uchar c);
	void freeBuffer(uint x);


#endif