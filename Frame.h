// Copyright (c) 2018 Jared H. Hudson
// Copyright (c) 2016 Jared Gaillard https://github.com/jwarcd/CZII_to_MQTT
// Licensed under the MIT License
//
#ifndef FRAME_H_
#define FRAME_H_
#include <vector>
#include <iomanip>

extern bool verbose;

class Frame {
public:
	Frame();
	virtual ~Frame();
	bool parseBuffer(u_int8_t input);
	bool validChecksum();
	u_int8_t getState();
	u_int8_t getFunc(void);
	void empty();
	u_int8_t getSrc();
	u_int8_t getDst();
	std::vector<u_int8_t> getData();
	const static size_t maxframelen = 2 + 2 + 1 + 2 + 1 + 255 + 2;
	const size_t getErrors();
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
	u_int16_t buffer[maxframelen];
	uint16_t ModRTU_CRC(u_int16_t ringBuffer[], u_int8_t length);
	bool checksum_valid;
	size_t errors;
};

#endif /* FRAME_H_ */
