// Copyright (c) 2018 Jared H. Hudson
// Copyright (c) 2016 jwarcd https://github.com/jwarcd/CZII_to_MQTT
// Licensed under the MIT License
//
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <sys/time.h>
#include <vector>
#include <unistd.h>
#include "Frame.h"
#include "HVAC.h"

using namespace std;

bool verbose = false;

int main(int argc, char **argv) {
	int opt;
	bool pty = false;
	u_int32_t baud_rate = 9600;
	string device = "/dev/ttyUSB0";

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

	HVAC hvac(device, baud_rate, pty, 4);
	hvac.listen();

	return 0;
}
