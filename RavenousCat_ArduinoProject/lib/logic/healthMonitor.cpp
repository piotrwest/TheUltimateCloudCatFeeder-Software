
#include "pt.h"

#include "healthMonitor.h"
#include "log4arduino.h"
#include "deadSimpleTimer.h"
#include "inMemoryStore.h"
#include "WiFi.h"

pt HealthMonitor::pt;

void HealthMonitor::setup()
{
    LOG("Entering HealthMonitor setup");
    PT_INIT(&pt);
    LOG("Finished HealthMonitor setup");
}

int HealthMonitor::protothread()
{
    static const int NUMBER_OF_MEASURES = 60;
    static const int GAP_BETWEEN_MEASURES_IN_MS = 1000; //1000ms*60 measures = 60s
    static int measureNumber;
    static int wifiConnected;
    static uint32_t minHeapSize;
    static DeadSimpleTimer::deadTimer measureTimer;

    PT_BEGIN(&pt);
    while (1)
    {
        minHeapSize = 9999999;
        wifiConnected = 1;
        measureNumber = 0;

        for (measureNumber = 0; measureNumber < NUMBER_OF_MEASURES; measureNumber++)
        {
            DeadSimpleTimer::setMs(&measureTimer, GAP_BETWEEN_MEASURES_IN_MS);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&measureTimer));

            minHeapSize = _min(ESP.getFreeHeap(), minHeapSize);
            if (!WiFi.isConnected())
            {
                wifiConnected = 0;
            }
        }

        LOG("HealthMonitor - min FreeHeap size is: %u bytes, wifiConnected: %d", minHeapSize, wifiConnected);
        InMemoryStore::putMinFreeHeapSize(minHeapSize);
        InMemoryStore::putWifiConnected(wifiConnected);
    }

    PT_END(&pt);
}