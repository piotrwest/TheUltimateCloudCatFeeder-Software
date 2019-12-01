#pragma once

#include "time.h"
#include "Arduino.h"

#define SINGLE_MESSAGE_BUFFER 256
#define MESSAGE_BUFFER 20*1024 //remember to change platformio.ini -> build_flags when changing this value
#define PUBLISH_TRESHOLD 4*1024 + 512 //AWS IOT bills every 5kb, so let's publish at 4.5kb to be safe
#define BUFFER_OVERFLOW 19*1024 + 512

class InMemoryStore
{

  public:
	static void setup();
	static void putVoltageLevels(float r1, float r2, float r3, float r4);
	static void putTemp(float temp);
	static void putStepsPerLidOperation(int steps);
	static void putStepsPerFoodGram(int steps);
	static void putCatToBowlTime(int seconds);
	static void putBeforeLidOpeningStats(double actualFoodAmount, double wantedFoodAmount);
	static void putFoodAmount(double foodAmount);
	static void putCatWeight(double catWeight);
	static void putEatingStats(double eatingTimeInSeconds, double startFoodAmount, double endFoodAmount);
	static void putMinFreeHeapSize(uint32_t minFreeHeapSize);
	static void putProtothreadLoopLengths(int64_t timers[], int timersSize);
	static void initMessagePayload();
	static void putClockDrift(long us);
	static bool isTimeToPublish();
	static char *getPublishPayload();
	static void putMessageBufferUtilization();
	static void putMqttPublishedSuccessfully();
	static void putMqttPublishError();
	static void putAWSIoTClientState(int state);
	static void putWifiConnected(int wifiOn);
	static void putReverseFoodMotorSteps(long steps);
	static void putAfterEatingFoodAmount(double afterEatingFoodAmount);

	struct WifiConfig
	{
		WifiConfig(){};
		char *id;
		char *pass;
	};

	struct FoodPortionSchedule
	{
		int hour;
		int minute;
		float grams;
	};

	struct SdConfig
	{
		WifiConfig *wifis;
		int wifisSize;

		int gmtOffsetHours;
		int daylightOffsetHours;

		int closeLidHallOffsetMotorSteps;
		int openLidHallOffsetMotorSteps;

		char *awsHostAddress;
		char *awsClientId;

		FoodPortionSchedule *foodSchedule;
		int foodScheduleSize;
	};

	static SdConfig sdConfig;

  private:
	InMemoryStore();
	static void putHardwareReset();

	static char singleLineBuffer[SINGLE_MESSAGE_BUFFER];
	static char messagePayload[MESSAGE_BUFFER];
	static int messagePosition;
	static time_t messageRootTime;
	
	static int mqttPublishedSuccessfully;
	static int mqttPublishError;
};