/*
 * MQTT.h
 *
 *  Created on: Oct 7, 2018
 *      Author: jhhudso
 */

#ifndef MQTT_H_
#define MQTT_H_
#include <mosquittopp.h>

class MQTT : public mosqpp::mosquittopp {
public:
	MQTT(const char *id, const char *host, int port);
	virtual ~MQTT();

	void on_connect(int rc);
	void on_message(const struct mosquitto_message *message);
	void on_subscribe(int mid, int qos_count, const int *granted_qos);
};

#endif /* MQTT_H_ */
