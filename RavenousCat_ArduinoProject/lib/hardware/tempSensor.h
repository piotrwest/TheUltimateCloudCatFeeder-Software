#pragma once

#include <OneWire.h>
#include <DallasTemperature.h>

class TempSensor
{

public:
    static void setup();
	static int protothread();
	static float getTemp();
	
	static void soLookingForAddressYouAre();
private:
	static struct pt pt;
	static DeviceAddress sensorAddress;
	static OneWire oneWire;
	static DallasTemperature sensors;
};