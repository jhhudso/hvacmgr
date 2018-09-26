/*
 * Frame.h
 *
 *  Created on: Sep 25, 2018
 *      Author: Jared H. Hudson
 */

#ifndef FRAME_H_
#define FRAME_H_
#include <vector>
#include <iomanip>

extern bool verbose;

class Frame {
public:
	Frame();
	virtual ~Frame();
	bool parseBuffer(u_int8_t tmp[64], size_t length);
	bool validChecksum();
private:
	u_int16_t dst;
	u_int16_t src;
	u_int8_t len;
	u_int16_t reserved;
	u_int8_t func;
	std::vector<u_int8_t> data;
	u_int16_t checksum;

	size_t framelen;
	int state;
	const static int maxframelen = 2 + 2 + 1 + 2 + 1 + 255 + 2;
	u_int16_t buffer[maxframelen];
	uint16_t ModRTU_CRC(u_int16_t ringBuffer[], u_int8_t length);
};

#endif /* FRAME_H_ */
