// Copyright - Jared Gaillard - 2016
// Copyright (c) 2018 Jared H. Hudson
// Licensed under the MIT License
//

#pragma once
typedef u_int8_t byte;

const float FLOAT_MIN_VALUE = -1000.0;

class Zone {

public:
	Zone(byte number) :
			zoneNumber(number), modified(false), coolSetpoint(-1), humidity(-1) {
	}

	void setCoolSetpoint(byte value) {
		if (coolSetpoint != value)
			modified = true;
		coolSetpoint = value;
	}

	byte getCoolSetpoint() {
		return coolSetpoint;
	}

	void setHeatSetpoint(byte value) {
		if (heatSetpoint != value)
			modified = true;
		heatSetpoint = value;
	}

	byte getHeatSetpoint() {
		return heatSetpoint;
	}

	void setTemperature(float value) {
		if (temperature != value)
			modified = true;
		temperature = value;
	}

	float getTemperature() {
		return temperature;
	}

	void setHumidity(byte value) {
		if (humidity != value)
			modified = true;
		humidity = value;
	}

	byte getHumidity() {
		return humidity;
	}

	void setDamperPosition(byte value) {
		if (damperPosition != value)
			modified = true;
		damperPosition = value;
	}

	byte getDamperPosition() {
		return damperPosition;
	}

	bool isModified() {
		return modified;
	}

	bool setModified(bool value) {
		modified = value;
		return true;
	}

private:
	byte zoneNumber;
	bool modified;

	byte coolSetpoint;
	byte heatSetpoint = -1;
	float temperature = FLOAT_MIN_VALUE;
	byte humidity;
	byte damperPosition = -1;
};
