
#include "pt.h"

#include "wifiConnectionManager.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "WiFi.h"

pt WifiConnectionManager::pt;

void WifiConnectionManager::setup()
{
    LOG("Entering WifiConnectionManager setup");
    PT_INIT(&pt);
    LOG("Finished WifiConnectionManager setup");
}

int WifiConnectionManager::protothread()
{
    static int currentWifiIndex;
    static InMemoryStore::WifiConfig currentWifiConfig = InMemoryStore::sdConfig.wifis[currentWifiIndex];
    static DeadSimpleTimer::deadTimer wifiTimer;

    PT_BEGIN(&pt);

    while (1)
    {
        if (WiFi.isConnected())
        {
            LOG("Currently connected to: %s", currentWifiConfig.id);

            DeadSimpleTimer::setMs(&wifiTimer, 10000);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&wifiTimer));
            continue;
        }

        LOG("Attempting to connect to WiFi: %s", currentWifiConfig.id);

        WiFi.begin(currentWifiConfig.id, currentWifiConfig.pass);

        DeadSimpleTimer::setMs(&wifiTimer, 30000);
        PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&wifiTimer));

        if (!WiFi.isConnected())
        {
            LOG("Couldnt connect to: %s", currentWifiConfig.id);
            currentWifiIndex++;
            if (currentWifiIndex >= InMemoryStore::sdConfig.wifisSize)
            {
                currentWifiIndex = 0;
            }
            currentWifiConfig = InMemoryStore::sdConfig.wifis[currentWifiIndex];
        }
    }

    PT_END(&pt);
}