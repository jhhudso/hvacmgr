/*
 * HVAC.cpp
 *
 *  Created on: Sep 26, 2018
 *      Author: jhhudso
 */

#include "HVAC.h"

using namespace std;

HVAC::HVAC(string device, u_int32_t baud_rate, bool pty, u_int8_t zones) :
		ios(), sp(ios, device), f(), device(), maxzones(zones), wait_for_beginning(
				true) {
	if (!pty) {
		sp.set_option(boost::asio::serial_port::baud_rate(baud_rate));
		sp.set_option(
				boost::asio::serial_port::flow_control(
						boost::asio::serial_port::flow_control::none));
		sp.set_option(
				boost::asio::serial_port::parity(
						boost::asio::serial_port::parity::none));
		sp.set_option(
				boost::asio::serial_port::stop_bits(
						boost::asio::serial_port::stop_bits::one));
		sp.set_option(
				boost::asio::serial_port::character_size(
						boost::asio::serial_port::character_size(8)));
	}

}

void HVAC::listen(void) {
	while (true) {
		timeval diff, before, after;
		memset(&tmp, 0, 64);
		gettimeofday(&before, NULL);
		size_t length = sp.read_some(boost::asio::buffer(tmp));
		gettimeofday(&after, NULL);
		diff.tv_sec = after.tv_sec - before.tv_sec;
		diff.tv_usec = after.tv_usec - before.tv_usec;

		if (wait_for_beginning) {
			if (diff.tv_sec > 2) {
				wait_for_beginning = false;
				f = Frame();
			} else {
				continue;
			}
		}
		for (size_t i = 0; i < length; i++) {
			if (f.parseBuffer(tmp[i]) == true) {
				parseFrame(f);
				f.empty();
			}
		}
	}
}

HVAC::~HVAC() {
	// TODO Auto-generated destructor stub
	sp.close();
}

bool HVAC::parseFrame(Frame f) {
	u_int8_t func = f.getFunc();

	switch (func) {
	case 0x6:
		break;
	case 0xB:
		break;
	case 0xC:
		break;
	case 0x15:
		break;
	default:
		break;
	}

	return false;
}
