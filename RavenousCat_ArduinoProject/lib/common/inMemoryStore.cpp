
#include "pt.h"

#include "log4arduino.h"
#include "inMemoryStore.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "NTPSync.h"

InMemoryStore::SdConfig InMemoryStore::sdConfig;
char InMemoryStore::singleLineBuffer[];
char InMemoryStore::messagePayload[];
int InMemoryStore::messagePosition;
int InMemoryStore::mqttPublishedSuccessfully;
int InMemoryStore::mqttPublishError;
time_t InMemoryStore::messageRootTime;

#define put_to_message_payload(format, ...)                                                                                             \
    {                                                                                                                                   \
        if (messagePosition > BUFFER_OVERFLOW)                                                                                          \
        {                                                                                                                               \
            return;                                                                                                                     \
        }                                                                                                                               \
        time_t now;                                                                                                                     \
        time(&now);                                                                                                                     \
        int lineLength = sprintf(singleLineBuffer, format, (long)(now - messageRootTime), NTPSync::getDecisecondPart(), ##__VA_ARGS__); \
        strncpy(messagePayload + messagePosition, singleLineBuffer, lineLength);                                                        \
        messagePosition += lineLength;                                                                                                  \
        messagePayload[messagePosition] = '\0';                                                                                         \
        LOG("MessagePayload size: %d bytes", messagePosition);                                                                \
    }

void InMemoryStore::setup()
{
    initMessagePayload();
    putHardwareReset();
}

void InMemoryStore::initMessagePayload()
{
    messagePosition = 0;
    messagePayload[messagePosition] = '\0';
    mqttPublishedSuccessfully = 0;
    mqttPublishError = 0;
    time(&messageRootTime);
    LOG("Initing message payload. Root time: %ld.%d", (long)messageRootTime, NTPSync::getDecisecondPart());
    int lineLength = sprintf(singleLineBuffer, "root@%ld%d#", (long)(messageRootTime), NTPSync::getDecisecondPart());
    strncpy(messagePayload + messagePosition, singleLineBuffer, lineLength);
    messagePosition += lineLength;
    messagePayload[messagePosition] = '\0';
}

//TODO: remove last measure - not used
void InMemoryStore::putVoltageLevels(float r1, float r2, float r3, float r4)
{
    put_to_message_payload("V@%ld%d@%.2f|%.2f|%.2f|%.2f#", r1, r2, r3, r4);
}

void InMemoryStore::putTemp(float t)
{
    put_to_message_payload("T@%ld%d@%.4f#", t);
}

void InMemoryStore::putStepsPerLidOperation(int steps)
{
    put_to_message_payload("SL@%ld%d@%d#", steps);
}

void InMemoryStore::putStepsPerFoodGram(int steps)
{
    put_to_message_payload("SF@%ld%d@%d#", steps);
}

void InMemoryStore::putCatToBowlTime(int seconds)
{
    put_to_message_payload("CTB@%ld%d@%d#", seconds);
}

void InMemoryStore::putBeforeLidOpeningStats(double actualFoodAmount, double wantedFoodAmount)
{
    put_to_message_payload("FBO@%ld%d@%.2f|%.2f#", actualFoodAmount, wantedFoodAmount);
}

void InMemoryStore::putFoodAmount(double foodAmount)
{
    put_to_message_payload("FA@%ld%d@%.2f#", foodAmount);
}

void InMemoryStore::putCatWeight(double catWeight)
{
    put_to_message_payload("CW@%ld%d@%.0f#", catWeight);
}

void InMemoryStore::putEatingStats(double eatingTimeInSeconds, double startFoodAmount, double endFoodAmount)
{
    put_to_message_payload("ES@%ld%d@%.2f|%.2f|%.2f#", eatingTimeInSeconds, startFoodAmount, endFoodAmount);
}

void InMemoryStore::putMinFreeHeapSize(uint32_t minFreeHeapSize)
{
    put_to_message_payload("HS@%ld%d@%u#", minFreeHeapSize);
}

void InMemoryStore::putProtothreadLoopLengths(int64_t timers[], int timersSize)
{
    if (timersSize != 14) {
        LOG("ERROR! EXPECTED protothread loop length size of 14 (dirty hack for speed), but found %d. Modify code below!", timersSize);
    } else {
        put_to_message_payload("PLL@%ld%d@%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld#",
            timers[0],
            timers[1],
            timers[2],
            timers[3],
            timers[4],
            timers[5],
            timers[6],
            timers[7],
            timers[8],
            timers[9],
            timers[10],
            timers[11],
            timers[12],
            timers[13]);
    }
    //Serial.printf("PAYLOAD: %s", messagePayload);
}

void InMemoryStore::putClockDrift(long us)
{
    put_to_message_payload("CD@%ld%d@%ld#", us);
}

void InMemoryStore::putMessageBufferUtilization()
{
    put_to_message_payload("MBU@%ld%d@%d#", messagePosition);
}

bool InMemoryStore::isTimeToPublish()
{
    return messagePosition > PUBLISH_TRESHOLD;
}

char *InMemoryStore::getPublishPayload()
{
    return messagePayload;
}

void InMemoryStore::putMqttPublishedSuccessfully()
{
    mqttPublishedSuccessfully++;
    put_to_message_payload("MQTTS@%ld%d@%d#", mqttPublishedSuccessfully);
}

void InMemoryStore::putMqttPublishError()
{
    mqttPublishError++;
    put_to_message_payload("MQTTE@%ld%d@%d#", mqttPublishError);
}

void InMemoryStore::putAWSIoTClientState(int state)
{
    put_to_message_payload("AWSIOTS@%ld%d@%d#", state);
}

void InMemoryStore::putHardwareReset()
{
    put_to_message_payload("HR@%ld%d@%d#", 1);
}

void InMemoryStore::putWifiConnected(int wifiOn)
{
    put_to_message_payload("WON@%ld%d@%d#", wifiOn);
}

void InMemoryStore::putReverseFoodMotorSteps(long steps)
{
    put_to_message_payload("RS@%ld%d@%ld#", steps);
}

void InMemoryStore::putAfterEatingFoodAmount(double afterEatingFoodAmount)
{
    put_to_message_payload("AEF@%ld%d@%.2f#", afterEatingFoodAmount);
}
