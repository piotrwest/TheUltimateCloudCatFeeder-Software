#pragma once

#include "pt.h"

class NTPSync
{

public:
	static void setup(int gmtOffsetHours, int daylightOffsetHours);
	static void blockinglyWaitForNetworkTime();
	static int protothread();
	static long doMicroSecondsSync();
	static int getDecisecondPart();

	static int64_t rootEspMicroSeconds;
private:
	NTPSync();
	static struct pt pt;
};