// Copyright (c) 2018 Jared H. Hudson
// Copyright (c) 2016 Jared Gaillard https://github.com/jwarcd/CZII_to_MQTT
// Licensed under the MIT License
//

#include <ctime>
#include <boost/log/trivial.hpp>
#include "HVAC.h"
#include "MQTT.h"

using namespace std;

const char* dayStr(uint8_t day) {
	switch (day) {
	case 1:
		return "Sunday";
		break;
	case 2:
		return "Monday";
		break;
	case 3:
		return "Tuesday";
		break;
	case 4:
		return "Wednesday";
		break;
	case 5:
		return "Thursday";
		break;
	case 6:
		return "Friday";
		break;
	case 7:
		return "Saturday";
		break;
	default:
		return "Unknown";
		break;
	}
}

HVAC::HVAC(string device, u_int32_t baud_rate, bool pty, u_int8_t numberZones, string mqtt_broker, int mqtt_port) :
		NUMBER_ZONES(numberZones), statusModified(false), ios(), sp(ios), f(), device(), wait_for_beginning(true), mqtt_broker(mqtt_broker), mqtt_port(mqtt_port) {
	try {
		sp.open(device);
	} catch (...) {
		cerr << "Error occurred opening " << device << endl;
		exit(1);
	}
	if (!pty) {
		sp.set_option(boost::asio::serial_port::baud_rate(baud_rate));
		sp.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
		sp.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
		sp.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));
		sp.set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
	}

	for (size_t i = 1; i <= NUMBER_ZONES; i++) {
		zones[i] = new Zone(i);
	}
	memset(&time, 0, sizeof(time));
}

void HVAC::listen(void) {
	MQTT *mqtt_handle;
	mqtt_handle = new MQTT("HVACmgr", mqtt_broker.c_str(), mqtt_port);

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
				update(f);
				if (isZoneModified()) {
					clearZoneModified();

					string output = toZoneJson();
					if (verbose) {
						cout << "hvac_mqtt_feed.publish: " << output.c_str() << endl;
					}
					int rc = mqtt_handle->publish(NULL, "hvac/zone", output.size()+1, output.c_str());
					if (rc != MOSQ_ERR_SUCCESS) {
						cout << "MQTT::publish rc=" << rc << endl;
					}

				}

				if (isStatusModified()) {
					clearStatusModified();

					string output = toStatusJson();
					if (verbose) {
						cout << "status_hvac_feed.publish: " << output.c_str() << endl;
					}
					//if (!status_hvac_feed.publish(output.c_str())) {     // Publish to MQTT server (openhab)
					//  cerr << "status_mqtt_feed.publish Failed" << endl;
					//}
				}
				f.empty();
			}
			if (f.getErrors() > 2) {
				wait_for_beginning = true;
				cerr << "3 errors received, waiting for next pause in transmission." << endl;
			}
		}
		int res = mqtt_handle->loop();						// Keep MQTT connection
			if (res) {
				cerr << "reconnecting to MQTT broker." << endl;
				mqtt_handle->reconnect();
			}
	}
}

HVAC::~HVAC() {
	sp.close();
}

float HVAC::getTemperatureF(u_int8_t highu_int8_t, u_int8_t lowu_int8_t) {
	return makeWord(highu_int8_t, lowu_int8_t) / 16.0;
}

u_int16_t HVAC::makeWord(u_int8_t h, u_int8_t l) {
	return (h << 8) | l;
}

bool HVAC::isStatusModified() {
	return statusModified;
}

void HVAC::clearStatusModified() {
	statusModified = false;
}

bool HVAC::isZoneModified() {
	for (u_int8_t i = 1; i <= NUMBER_ZONES; i++) {
		if (zones[i]->isModified()) {
			return true;
		}
	}
	return false;
}

void HVAC::clearZoneModified() {
	for (u_int8_t i = 1; i <= NUMBER_ZONES; i++) {
		zones[i]->setModified(false);
	}
}

Zone* HVAC::getZone(u_int8_t zoneIndex) {
	return zones[zoneIndex];
}

void HVAC::setControllerState(u_int8_t value) {
	if (controllerState != value)
		statusModified = true;

	controllerState = value;
}

void HVAC::setLatTemperature(u_int8_t value) {
	if (!isValidTemperature(value))
		return;

	if (lat_Temp_f != value)
		statusModified = true;

	lat_Temp_f = value;
}

void HVAC::setOutsideTemperature(float value) {
	if (!isValidTemperature(value))
		return;

	if (outside_Temp_f != value)
		statusModified = true;

	outside_Temp_f = value;
}

void HVAC::setOutsideTemperature2(float value) {
	if (!isValidTemperature(value))
		return;

	if (outside_Temp2_f != value)
		statusModified = true;

	outside_Temp2_f = value;
}

bool HVAC::isValidTemperature(float value) {
	return value < 200.0 && value > -50.0;
}

void HVAC::setDayTime(u_int8_t day, u_int8_t hour, u_int8_t minute, u_int8_t second) {
	if (day != time.Wday || hour != time.Hour || minute != time.Minute || second != time.Second) {
		statusModified = true;
	}

	time.Wday = day;
	cerr << "time.Wday=" << day << endl;
	time.Hour = hour;
	time.Minute = minute;
	time.Second = second;
}

//
//	Update cached data
//
bool HVAC::update(Frame &f) {
	vector<u_int8_t> data = f.getData();
	u_int8_t table = 0, row = 0;
	const u_int8_t dataLength = data.size();

	if (dataLength == 1) {
		return true;
	} else if (dataLength >= 3) {
		table = data.at(1);
		row = data.at(2);
	}

	const u_int8_t function = f.getFunc();

	switch (function) {
	case RESPONSE_FUNCTION:
		switch (table) {
		case 1:
			switch (row) {
			case 6:
				if (dataLength == 16) {
					updateZone1Info(data);
				} else {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 16" << endl;
				}
				break;
			case 16:
				if (dataLength == 19) {
					updateZoneSetpoints(data);
				} else {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 19" << endl;
				}

				break;
			case 18:
				if (dataLength == 7) {
					updateTime(data);
				} else {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 7" << endl;
				}
				break;
			default:
				cerr << "error: RES unrecognized table " << hex << (unsigned) table << " row " << hex << (unsigned) row << endl;
			}

			break;
		case 2:
			switch (row) {
			case 3:
				if (dataLength == 13) {
					updateZoneInfo(f.getSrc(), data);
				} else {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 13" << endl;
				}
				break;
			default:
				cerr << "error: RES unrecognized table " << hex << (unsigned) table << " row " << hex << (unsigned) row << endl;
				break;
			}
			break;
		case 9:
			switch (row) {
			case 1:
			case 2:
				if (data.size() != 11) {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " data.size() != 11" << endl;
				} else {
					for (size_t i = 3; i < data.size(); ++i) {
						if (data.at(i) != 0xFF) {
							cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " data.at(" << i << ") not 0xFF" << endl;
						}
					}
				}
				break;
			case 3:
				if (dataLength == 10) {
					updateOutsideTemp(data);
				} else {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 10" << endl;
				}
				break;
			case 5:
				if (dataLength == 4) {
					updateControllerState(data);
				} else {
					cerr << "error: RES" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 4" << endl;
				}
				break;
			default:
				cerr << "error: RES unrecognized table " << hex << (unsigned) table << " row " << hex << (unsigned) row << endl;
				break;
			}
			break;
		default:
			cerr << "error: RES unrecognized table " << hex << (unsigned) table << endl;
			break;
		}
		break;
	case WRITE_FUNCTION:
		switch (table) {
		case 2:
			switch (row) {
			case 1:
				if (dataLength == 13) {
					updateOutsideHumidityTemp(data);
				} else {
					cerr << "error: WR" << hex << (unsigned) table << "," << hex << (unsigned) row << " datalen != 13" << endl;
				}
				break;
			case 2:
				if (verbose) {
					cout << "src=" << hex << (unsigned) f.getSrc() << " dst=" << (unsigned) f.getDst() << endl;
					cout << "hold:      " << dec << (unsigned) data.at(3) << endl;
					cout << "out:       " << dec << (unsigned) data.at(5) << endl;
					cout << "Heat goal: " << dec << (unsigned) data.at(7) << "F" << endl;
					cout << "Cool goal: " << dec << (unsigned) data.at(8) << "F" << endl;
					cout << "Save:      " << dec << (unsigned) data.at(9) << endl;
				}
				break;
			default:
				cerr << "error: WR unrecognized table " << hex << (unsigned) table << " row " << hex << (unsigned) row << endl;
				break;
			}
			break;
		case 9:
			if (row == 4 && dataLength == 11) {
				updateDamperPositions(data);
			} else if (row == 5 && dataLength == 4) {
				updateControllerState(data);
			} else {
				cerr << "error: WR unrecognized table " << hex << (unsigned) table << " row " << hex << (unsigned) row << endl;
			}
			break;
		default:
			cerr << "error: WR unrecognized table " << hex << (unsigned) table << endl;
			break;
		}

		break;
	case READ_FUNCTION:
		if (verbose) {
			cout << "src=" << dec << (unsigned) f.getSrc() << " requesting table " << (unsigned) table << " row " << (unsigned) row << " from device id "
					<< (unsigned) f.getDst() << endl;
		}
		break;

	default:
		cerr << "error: function " << hex << (unsigned) function << " not recognized." << endl;
		break;
	}
	return true;
}

//
//   FRAME: 9.0  1.0  11  0.0.12  0.9.4.15.15.0.0.0.0.0.0.                 136.181
//
void HVAC::updateDamperPositions(RingBuffer& ringBuffer) {
	for (u_int8_t i = 1; i <= NUMBER_ZONES; i++) {
		zones[i]->setDamperPosition(ringBuffer.at(3 + i));
	}
}

//
//  FRAME: 8.0  1.0  16  0.0.6   0.1.6.0.0.4.64.60.0.0.0.0.0.0.17.50.      74.114
//
void HVAC::updateZone1Info(RingBuffer& ringBuffer) {
	zones[1]->setTemperature(getTemperatureF(ringBuffer.at(5), ringBuffer.at(6)));
	zones[1]->setHumidity(ringBuffer.at(7));
}

//
//  FRAME: 99.0  1.0  19  0.0.6   0.1.16. 78.77.76.76.76.76.76.76. 68.67.68.68.68.68.68.68.        177.133
//                                        [     cooling          ] [        heating       ]
//
void HVAC::updateZoneSetpoints(RingBuffer& ringBuffer) {
	for (u_int8_t i = 1; i <= NUMBER_ZONES; i++) {
		zones[i]->setCoolSetpoint(ringBuffer.at(2 + i));
		zones[i]->setHeatSetpoint(ringBuffer.at(10 + i));
	}
}

//
//  FRAME: 8.0  1.0  7   0.0.6   0.1.18.3.22.33.47
//
//
void HVAC::updateTime(RingBuffer& ringBuffer) {
	u_int8_t day = ringBuffer.at(3);
	u_int8_t hour = ringBuffer.at(4);
	u_int8_t minute = ringBuffer.at(5);
	u_int8_t second = ringBuffer.at(6);

	day++;
	setDayTime(day, hour, minute, second);
}

//
//  FRAME: 1.0  9.0  10  0.0.6   0.9.3.195.3.136.72.255.0.0.
//
void HVAC::updateOutsideTemp(RingBuffer& ringBuffer) {
	setOutsideTemperature(getTemperatureF(ringBuffer.at(4), ringBuffer.at(5)));
	setLatTemperature(ringBuffer.at(6));
}

//
//  FRAME: 9.0  1.0  4   0.0.12  0.9.5.128.
//
void HVAC::updateControllerState(RingBuffer& ringBuffer) {
	setControllerState(ringBuffer.at(3));
}

//
//  FRAME: 2.0  1.0  13  0.0.12  0.2.1.0.57.3.145.3.0.0.0.2.0.
//
void HVAC::updateOutsideHumidityTemp(RingBuffer& ringBuffer) {
	setOutsideTemperature2(getTemperatureF(ringBuffer.at(5), ringBuffer.at(6)));
	zones[1]->setHumidity(ringBuffer.at(4));
}

//
//  FRAME: 1.0  2.0  13  0.0.6   0.2.3.1.0.0.0.4.160.74.67.77.0.
//  FRAME: 1.0  2.0  13  0.0.6   0.2.3.0.0.0.0.4.122.71.66.78.0.
//
void HVAC::updateZoneInfo(u_int8_t zoneIndex, RingBuffer& ringBuffer) {
	if (verbose) {
		cerr << "updateZoneInfo(" << dec << (unsigned) zoneIndex << ")" << endl;
	}

	if (zoneIndex == 0 || zoneIndex > NUMBER_ZONES) {
		cerr << "error in updateZoneInfo(), zoneIndex(" << dec << (unsigned) zoneIndex << ") == 0 or >= " << dec << (unsigned) NUMBER_ZONES << endl;
		return;
	}

	zones[zoneIndex]->setTemperature(getTemperatureF(ringBuffer.at(7), ringBuffer.at(8)));
	zones[zoneIndex]->setHeatSetpoint(ringBuffer.at(10));
	zones[zoneIndex]->setCoolSetpoint(ringBuffer.at(11));
}

//
//   To Zone JSON string
//
string HVAC::toZoneJson() {
	boost::property_tree::ptree root;
	for (u_int8_t i = 1; i <= NUMBER_ZONES; i++) {
		boost::property_tree::ptree child;

		child.put("cool", zones[i]->getCoolSetpoint());
		child.put("heat", zones[i]->getHeatSetpoint());
		child.put("temp", zones[i]->getTemperature());
		child.put("hum", zones[i]->getHumidity());
		child.put("damp", zones[i]->getDamperPosition());
		root.add_child("z" + to_string(i), child);
	}

	ostringstream output;
	boost::property_tree::write_json(output, root);
	return output.str();
}

//
//   To Status JSON string
//
string HVAC::toStatusJson() {
	boost::property_tree::ptree root;

	char timestring[10];
	sprintf(timestring, "%02d:%02d:%02d", time.Hour, time.Minute, time.Second);
	root.put("time", timestring);

	root.put("day", dayStr(time.Wday));

	root.put("lat", lat_Temp_f);
	root.put("out", outside_Temp_f);
	root.put("out2", outside_Temp2_f);

	if (controllerState != (u_int8_t) -1) {
		boost::property_tree::ptree state;

		state.put("dehum", (int) (bool) (controllerState & DeHumidify));
		state.put("hum", (int) (bool) (controllerState & Humidify));
		state.put("fan", (int) (bool) (controllerState & Fan_G));
		state.put("rev", (int) (bool) (controllerState & ReversingValve));
		state.put("aux2", (int) (bool) (controllerState & AuxHeat2_W2));
		state.put("aux1", (int) (bool) (controllerState & AuxHeat1_W1));
		state.put("comp2", (int) (bool) (controllerState & CompressorStage2_Y2));
		state.put("comp1", (int) (bool) (controllerState & CompressorStage1_Y1));
		root.add_child("state", state);
	}

	ostringstream output;
	boost::property_tree::write_json(output, root);
	return output.str();
}

//
//   Add to json if value is not the default value
//
void HVAC::addJson(boost::property_tree::ptree& obj, string key, u_int8_t value) {
	if (value == (u_int8_t) -1)
		return;
	obj.put(key, value);
}

//
//    Add to json if value is not the default value
//
void HVAC::addJson(boost::property_tree::ptree& obj, string key, float value) {
	if (value > FLOAT_MIN_VALUE) {
		obj.put(key, value);
	}
}

