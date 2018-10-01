// Copyright (c) 2018 Jared H. Hudson
// Copyright (c) 2016 Jared Gaillard https://github.com/jwarcd/CZII_to_MQTT
// Licensed under the MIT License
//
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <cassert>
#include "Frame.h"

using namespace std;

extern bool verbose;

Frame::~Frame() {
	// TODO Auto-generated destructor stub
}

uint16_t Frame::ModRTU_CRC(u_int16_t ringBuffer[], u_int8_t length) {
	uint16_t crc = 0x0000;

	for (int pos = 0; pos < length; pos++) {
		crc ^= (uint16_t) (ringBuffer[pos]); // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			} else
				// Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	// NOTE, this number has low and high bytes swapped, so use it accordingly
	return crc;
}

Frame::Frame() :
		dst(0), src(0), len(0), reserved(0), func(0), data(), checksum(0), framelen(
				0), state(0), checksum_valid(false) {
	memset(&buffer, 0, sizeof(buffer));
}

bool Frame::validChecksum() {
	return ModRTU_CRC(buffer, (2 + 2 + 1 + 2 + 1) + len) == checksum;
}

bool Frame::parseBuffer(u_int8_t input) {
		buffer[framelen++] = input;
		assert(framelen < Frame::maxframelen);

		switch (state) {
		case 0: // destination byte 1
			dst = 0;
			src = 0;
			len = 0;
			reserved = 0;
			func = 0;
			data = vector<u_int8_t>();
			checksum = 0;

			dst = input;
			dst <<= 8;
			state++;
			break;
		case 1: // destination byte 2
			dst |= input;
			dst = ntohs(dst);
			if (verbose) {
				cout << "dst=" << dst;
			}
			state++;
			break;
		case 2: // source byte 1
			src = input;
			src <<= 8;
			state++;
			break;
		case 3: // source byte 2
			src |= input;
			src = ntohs(src);
			if (verbose) {
				cout << " src=" << src;
			}
			state++;
			break;
		case 4: // length
			len = input;
			if (verbose) {
				cout << " len=" << (unsigned) len;
			}
			state++;
			break;
		case 5: // reserved
			reserved = input;
			reserved <<= 8;
			state++;
			break;
		case 6: // reserved
			reserved |= input;
			if (verbose) {
				cout << " reserved=" << (unsigned) reserved;
			}
			state++;
			break;
		case 7: // function
			func = input;
			if (verbose) {
				cout << " func=" << (unsigned) func;
				switch (func) {
				case 0x6:
					cout << " (response)";
					break;
				case 0xB:
					cout << " (read request)";
					break;
				case 0xC:
					cout << " (write request)";
					break;
				case 0x15:
					cout << " (error)";
					break;
				default:
					cout << " (unknown)";
					break;
				}
			}
			state++;
			if (verbose) {
			  cout << " data=";
			}
			break;
		case 8: // data
			data.push_back(input);
			if (verbose) {
				cout << hex << setiosflags(ios::uppercase)
						<< static_cast<unsigned>(input) << dec << flush;
			}
			if (data.size() == len) {
				state++;
			}
			break;
		case 9: // checksum
			if (verbose) {
				cout << " checksum=";
			}
			checksum = input;
			checksum <<= 8;
			state++;
			break;
		case 10: // checksum
			checksum |= input;
			checksum = ntohs(checksum);
			if (verbose) {
				cout << hex << setiosflags(ios::uppercase)
						<< static_cast<unsigned>(checksum) << dec << flush;
			}
			if (ModRTU_CRC(buffer, (2 + 2 + 1 + 2 + 1) + len) == checksum) {
				checksum_valid = true;
				if (verbose) {
					cout << " (valid)";
				}
			} else {
				if (verbose) {
					cout << " (NOT VALID)";
				}
			}
			state++;
			break;
		} // switch

		if (state == 11) {
			if (verbose) {
				cout << endl;
			}
			return true; // frame complete
		}
	return false; // frame incomplete
} // parseBuffer()

u_int8_t Frame::getState() {
	return state;
}

u_int8_t Frame::getFunc() {
	return func;
}

vector<u_int8_t> Frame::getData() {
	return data;
}


void Frame::empty() {
	dst = 0;
	src = 0;
	len = 0;
	reserved = 0;
	func = 0;
	data = vector<u_int8_t>();
	checksum = 0;

	state = 0;
	framelen = 0;
}
