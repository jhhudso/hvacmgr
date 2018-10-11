/*
 * MQTT.cpp
 *
 *  Created on: Oct 7, 2018
 *      Author: jhhudso
 */
#include <iostream>
#include "MQTT.h"

using namespace std;

MQTT::MQTT(const char *id, const char *host, int port) {
	mosqpp::lib_init();			// Initialize libmosquitto

	int keepalive = 120; // seconds
	connect(host, port, keepalive);		// Connect to MQTT Broker
}

MQTT::~MQTT() {
	// TODO Auto-generated destructor stub
}

void MQTT::on_connect(int rc)
{
	cout << "Connected with code " << rc << "." << endl;

	if (rc == 0)
	{
		subscribe(NULL, "command/IGot");
	}
}

void MQTT::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	printf("Subscription succeeded. \n");
}

void MQTT::on_message(const struct mosquitto_message *message)
{
	if (message && message->topic) {
		cout << "on_message" << message->topic << endl;
	}
}
