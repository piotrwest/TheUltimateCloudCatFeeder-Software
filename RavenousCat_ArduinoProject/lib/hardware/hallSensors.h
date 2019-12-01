#pragma once

class HallSensors
{

public:
	static uint16_t getClosedLidReading();
	static uint16_t getOpenLidReading();
	static void setup();

private:
    HallSensors();
};