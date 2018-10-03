// Copyright (c) 2018 Jared H. Hudson
// Copyright (c) 2016 Jared Gaillard https://github.com/jwarcd/CZII_to_MQTT
// Licensed under the MIT License
//
#ifndef HVAC_H_
#define HVAC_H_
#include <iomanip>
#include <iostream>
#include <boost/asio.hpp>
#include "Frame.h"
#include "Zone.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

typedef std::vector<u_int8_t> RingBuffer;
#define TABLE1_TEMPS_ROW   		16
#define TABLE1_TIME_ROW   		18

typedef struct  {
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;
}       tmElements_t, TimeElements, *tmElementsPtr_t;

class HVAC {
public:
	HVAC(std::string device, u_int32_t baud_rate, bool pty, u_int8_t zones);

	void listen(void);
	virtual ~HVAC();
	u_int16_t makeWord(u_int8_t h, u_int8_t l);
	Zone* getZone(byte zoneIndex);
	bool isZoneModified();
	void clearZoneModified();
	bool isStatusModified();
	void clearStatusModified();
	std::string toZoneJson();
	std::string toStatusJson();

    // CZII frame
    static const byte DEST_ADDRESS_POS     = 0;
    static const byte SOURCE_ADDRESS_POS   = 2;
    static const byte DATA_LENGTH_POS      = 4;
    static const byte FUNCTION_POS         = 7;
    static const byte DATA_START_POS       = 8;
    static const byte MIN_MESSAGE_SIZE     = 11;

    // CZII Function Codes
    static const byte RESPONSE_FUNCTION    = 6;
    static const byte READ_FUNCTION        = 11;
    static const byte WRITE_FUNCTION       = 12;

  private:

    Zone* zones[8];
    u_int8_t NUMBER_ZONES;
    bool statusModified;

    byte controllerState = -1;
    byte lat_Temp_f = -1;
    float outside_Temp_f = FLOAT_MIN_VALUE;
    float outside_Temp2_f = FLOAT_MIN_VALUE;
    TimeElements time;

	bool isValidTemperature(float value);
    float getTemperatureF(byte highByte, byte lowByte);
    void updateZoneInfo(u_int8_t zone, RingBuffer& ringBuffer);
    void updateOutsideHumidityTemp(RingBuffer& ringBuffer);
    void updateOutsideTemp(RingBuffer& ringBuffer);
    void updateControllerState(RingBuffer& ringBuffer);
    void updateZoneSetpoints(RingBuffer& ringBuffer);
    void updateTime(RingBuffer& ringBuffer);
    void updateZone1Info(RingBuffer& ringBuffer);
    void updateDamperPositions(RingBuffer& ringBuffer);

    void addJson(boost::property_tree::ptree& obj, std::string key, byte value);
    void addJson(boost::property_tree::ptree& obj, std::string key, float value);

    void setControllerState(byte value);
    void setLatTemperature(byte value);
    void setOutsideTemperature(float value);
    void setOutsideTemperature2(float value);
	void setDayTime(byte day, byte hour, byte minute, byte second);
	bool update(Frame &);

    enum ControllerStateFlags {
      DeHumidify 				  = 1 << 7,
      Humidify 				    = 1 << 6,
      Fan_G					      = 1 << 5,
      ReversingValve			= 1 << 4,
      AuxHeat2_W2				  = 1 << 3,
      AuxHeat1_W1				  = 1 << 2,
      CompressorStage2_Y2	= 1 << 1,
      CompressorStage1_Y1	= 1 << 0
    };

	bool decodeTable(std::vector<u_int8_t>);
	boost::asio::io_service ios;
	boost::asio::serial_port sp;
	Frame f;
	std::string device;
	u_int8_t tmp[64];
	bool wait_for_beginning;
};

#endif /* HVAC_H_ */
