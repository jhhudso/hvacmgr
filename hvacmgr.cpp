// Copyright (c) 2018 Jared H. Hudson
// Copyright (c) 2016 jwarcd https://github.com/jwarcd/CZII_to_MQTT
// Licensed under the MIT License
//
#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <cstdio>
#include <sys/time.h>
#include <vector>
#include <unistd.h>
#include "Frame.h"

using namespace std;

bool verbose = false;


int main(int argc, char **argv) {
	int opt;
	auto pty = false;
	auto baud_rate = 9600;
	auto device = "/dev/ttyUSB0";

	while ((opt = getopt(argc, argv, "pb:f:v")) != -1) {
		switch (opt) {
		case 'p':
			pty = true;
			break;
		case 'b':
			pty = false;
			baud_rate = atoi(optarg);
			break;
		case 'f':
			device = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		default: /* '?' */
			cerr << "Usage: " << argv[0] << "[-b baud rate] [-p] [-f file]"
					<< endl;
			cerr << "-p file is a pty" << endl;
			exit(EXIT_FAILURE);
		}
	}

	static boost::asio::io_service ios;
	boost::asio::serial_port sp(ios, device);

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

	Frame f;
	u_int8_t tmp[64];
	bool wait_for_beginning = true;

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
				if (verbose) {
					cout << "new frame" << endl;
				}
				f = Frame();
			} else {
				continue;
			}
		}
		f.parseBuffer(tmp, length);
	}
	sp.close();

	return 0;
}
