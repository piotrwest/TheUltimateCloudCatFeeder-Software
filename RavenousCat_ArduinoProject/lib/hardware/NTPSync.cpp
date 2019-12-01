#include "pt.h"

#include "Arduino.h"
#include "NTPSync.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "time.h"
#include "ledRing.h"
#include "wifiPublisher.h"
#include "wifiConnectionManager.h"

int64_t NTPSync::rootEspMicroSeconds;
pt NTPSync::pt;

void NTPSync::setup(int gmtOffsetHours, int daylightOffsetHours)
{
    const char *ntpServer1 = "pool.ntp.org";
    const char *ntpServer2 = "1.pool.ntp.org";
    const char *ntpServer3 = "3.pool.ntp.org";

    LOG("Entering NTPSync setup. GmtOffset: %d h, DaylightOffset: %d h", gmtOffsetHours, daylightOffsetHours);
    configTime(gmtOffsetHours * 3600, daylightOffsetHours * 3600, ntpServer1, ntpServer2, ntpServer3);
    PT_INIT(&pt);
    LOG("Finished NTPSync setup");
}

void NTPSync::blockinglyWaitForNetworkTime()
{
    LOG("Waiting for network time...");
    struct tm timeinfo;
    int progress = 100;
    int progressDelta = -1;
    while (!getLocalTime(&timeinfo, 0))
    {
        LedRing::showWaitingForNetworkTime(progress);
        WifiConnectionManager::protothread(); //connects to wifi
        progress += progressDelta;
        if (progress <= 0 || progress >= 100)
        {
            progressDelta *= -1;
        }
        delay(20);
    }
    LedRing::clear();
    LOG("Got the time from network: %s", asctime(&timeinfo));
    time_t now;
    time(&now);
    LOG("Epoch: %ld", (long)now);
    doMicroSecondsSync();
}

int NTPSync::getDecisecondPart()
{
    long result = (esp_timer_get_time() - rootEspMicroSeconds) % 1000000;
    if (result < 0)
    {
        result *= -1;
    }
    return result / 100000;
}

long standardize(long microseconds)
{
    if (microseconds < 0)
    { //brind to positive value
        microseconds *= -1;
    }
    if (microseconds > 500000)
    {                            //if it's greater than 500k, for example 999996
        microseconds *= -1;      //convert it to -999996
        microseconds += 1000000; //add 1M to bring below 500k
    }
    return microseconds % 1000000;
}

long NTPSync::doMicroSecondsSync()
{
    int64_t prevMicroSeconds = rootEspMicroSeconds;
    static time_t t1;
    static time_t t2;
    time(&t1);
    t1 += 2;
    while (true)
    {
        time(&t2);
        if (t1 - t2 == 0)
        {
            rootEspMicroSeconds = esp_timer_get_time(); //don't care about 2s, the ms/us part is important only
            break;
        }
    }
    long newMicroSeconds = rootEspMicroSeconds % 1000000;
    long oldMicroSeconds = prevMicroSeconds % 1000000;
    long result = standardize(newMicroSeconds) - standardize(oldMicroSeconds);
    LOG("Did microseconds sync! uS diff: %ld", result);
    return result;
}

int NTPSync::protothread()
{
    static long sleepSecondsUntilMidnight;
    static DeadSimpleTimer::deadTimer syncTimer;
    static struct tm timeinfo;

    PT_BEGIN(&pt);
    while (1)
    {
        if (getLocalTime(&timeinfo, 0))
        {
            LOG("Now is: %s", asctime(&timeinfo));
            sleepSecondsUntilMidnight = (23 - timeinfo.tm_hour) * 3600;
            LOG("Will sleep %ld seconds because of hours part until midnight", sleepSecondsUntilMidnight);
            sleepSecondsUntilMidnight += (59 - timeinfo.tm_min) * 60;
            LOG("Will sleep %ld seconds because of hours&minutes part until midnight", sleepSecondsUntilMidnight);
            sleepSecondsUntilMidnight += (59 - timeinfo.tm_sec);
            
            if (sleepSecondsUntilMidnight > 10 * 60)
            { //if more than 10 min
                sleepSecondsUntilMidnight = 10 * 60;
                LOG("Would sleep a lot... Instead sleeping %ld seconds and retrying...", sleepSecondsUntilMidnight);
                DeadSimpleTimer::setMs(&syncTimer, sleepSecondsUntilMidnight * 1000L);
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&syncTimer));
                continue;
            } else {
                //adding 7 min to be 7 min after midnight
                sleepSecondsUntilMidnight += 7 * 60;
                LOG("7min past midnight close! Sleeping %ld seconds...", sleepSecondsUntilMidnight);
                DeadSimpleTimer::setMs(&syncTimer, sleepSecondsUntilMidnight * 1000L);
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&syncTimer));

                getLocalTime(&timeinfo, 0);
                LOG("Syncing microseconds will happen now -> %s", asctime(&timeinfo));
                long diff = doMicroSecondsSync();
                InMemoryStore::putClockDrift(diff);
                LOG("Microsecond sync done. New root us: %lld", rootEspMicroSeconds);
            }
        }
        else
        {
            LOG("WARN! Couldn't get time. Will sleep 1min");
            DeadSimpleTimer::setMs(&syncTimer, 60 * 1000L);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&syncTimer));
        }
    }

    PT_END(&pt);
}