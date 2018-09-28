/*
 * HVAC.h
 *
 *  Created on: Sep 26, 2018
 *      Author: jhhudso
 */

#ifndef HVAC_H_
#define HVAC_H_
#include <iomanip>
#include <iostream>
#include <boost/asio.hpp>
#include "Frame.h"

class HVAC {
public:
	HVAC(std::string device, u_int32_t baud_rate, bool pty, u_int8_t zones);
	bool parseFrame(Frame);
	void listen(void);
	virtual ~HVAC();
private:
	boost::asio::io_service ios;
	boost::asio::serial_port sp;
	Frame f;
	std::string device;
	u_int8_t maxzones;
	u_int8_t tmp[64];
	bool wait_for_beginning;

};

#endif /* HVAC_H_ */
