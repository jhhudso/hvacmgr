/*
 * Frame.cpp
 *
 *  Created on: Sep 25, 2018
 *      Author: Jared H. Hudson
 */
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <cassert>
#include "Frame.h"

using namespace std;

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
				0), state(0) {
	memset(&buffer, 0, sizeof(buffer));
}

bool Frame::validChecksum() {
	return ModRTU_CRC(buffer, (2 + 2 + 1 + 2 + 1) + len) == checksum;
}

bool Frame::parseBuffer(u_int8_t tmp[64], size_t length) {
	for (size_t i = 0; i < length; i++) {
		buffer[framelen++] = tmp[i];
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

			dst = tmp[i];
			dst <<= 8;
			state++;
			break;
		case 1: // destination byte 2
			dst |= tmp[i];
			dst = ntohs(dst);
			if (verbose) {
				cout << "dst=" << dst;
			}
			state++;
			break;
		case 2: // source byte 1
			src = tmp[i];
			src <<= 8;
			state++;
			break;
		case 3: // source byte 2
			src |= tmp[i];
			src = ntohs(src);
			if (verbose) {
				cout << " src=" << src;
			}
			state++;
			break;
		case 4: // length
			len = tmp[i];
			if (verbose) {
				cout << " len=" << (unsigned) len;
			}
			state++;
			break;
		case 5: // reserved
			reserved = tmp[i];
			reserved <<= 8;
			state++;
			break;
		case 6: // reserved
			reserved |= tmp[i];
			if (verbose) {
				cout << " reserved=" << (unsigned) reserved;
			}
			state++;
			break;
		case 7: // function
			func = tmp[i];
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
			break;
		case 8: // data
			data.push_back(tmp[i]);
			if (verbose) {
				cout << hex << setiosflags(ios::uppercase)
						<< static_cast<unsigned>(tmp[i]) << dec << flush;
			}
			if (data.size() == len) {
				state++;
			}
			if (data.size() == 1) {
				if (verbose) {
					cout << " data=";
				}
			}
			break;
		case 9: // checksum
			if (verbose) {
				cout << " checksum=";
			}
			checksum = tmp[i];
			checksum <<= 8;
			state++;
			break;
		case 10: // checksum
			checksum |= tmp[i];
			checksum = ntohs(checksum);
			if (verbose) {
				cout << hex << setiosflags(ios::uppercase)
						<< static_cast<unsigned>(checksum) << dec << flush;
			}
			if (ModRTU_CRC(buffer, (2 + 2 + 1 + 2 + 1) + len) == checksum) {
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
		}

		if (state == 11) {
			if (verbose) {
				cout << endl;
			}
			state = 0;
			framelen = 0;
		}
	}
	return false;
} // parseBuffer()
