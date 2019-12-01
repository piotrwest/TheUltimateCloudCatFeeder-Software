
#include "pt.h"

#include "foodPortionScheduler.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "WiFi.h"
#include "foodPortionDispenser.h"

pt FoodPortionScheduler::pt;

void FoodPortionScheduler::setup()
{
    LOG("Entering FoodPortionScheduler setup");
    PT_INIT(&pt);
    LOG("Finished FoodPortionScheduler setup");
}

int FoodPortionScheduler::protothread()
{
    static DeadSimpleTimer::deadTimer timer;

    PT_BEGIN(&pt);

    while (1)
    {
        DeadSimpleTimer::setMs(&timer, 10000);
        PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&timer));

        struct tm timeinfo;
        getLocalTime(&timeinfo, 0);

        int minimumSecondsDistance = 25 * 3600;
        int scheduleIndex = -1;
        //look for closest schedule, in future
        //example: schedules: 11:20, 13:40
        //now is:                 12:10
        for (int i = 0; i < InMemoryStore::sdConfig.foodScheduleSize; i++)
        {
            InMemoryStore::FoodPortionSchedule currentSchedule = InMemoryStore::sdConfig.foodSchedule[i];
            //LOG("Evaluating schedule: %d:%d", currentSchedule.hour, currentSchedule.minute);
            //LOG("Time now: %d:%d", timeinfo.tm_hour, timeinfo.tm_min);
            if (currentSchedule.hour > timeinfo.tm_hour ||
                (currentSchedule.hour == timeinfo.tm_hour && currentSchedule.minute > timeinfo.tm_min))
            {
                int distance = (currentSchedule.hour - timeinfo.tm_hour) * 3600 + (currentSchedule.minute - timeinfo.tm_min) * 60 - timeinfo.tm_sec;
                //LOG("Seconds distance: %d", distance);
                if (distance < minimumSecondsDistance)
                {
                    minimumSecondsDistance = distance;
                    scheduleIndex = i;
                }
            }
        }

        if (scheduleIndex == -1)
        { //assume schedule is next day, at index 0
            LOG("Closest food schedule the same day wasn't found, therefore assuming next schedule is the first item..");
            minimumSecondsDistance = (23 - timeinfo.tm_hour) * 3600;
            minimumSecondsDistance += (59 - timeinfo.tm_min) * 60;
            minimumSecondsDistance += (59 - timeinfo.tm_sec);
            minimumSecondsDistance += InMemoryStore::sdConfig.foodSchedule[0].hour * 3600;
            minimumSecondsDistance += InMemoryStore::sdConfig.foodSchedule[0].minute * 60;
            scheduleIndex = 0;
        }

        if (minimumSecondsDistance < 3 * 60)
        {
            LOG("Seconds until next schedule: %d, which is smaller than 3min, therefore not scheduling now.", minimumSecondsDistance);
        }
        else
        {
            InMemoryStore::FoodPortionSchedule nextSchedule = InMemoryStore::sdConfig.foodSchedule[scheduleIndex];
            time_t at;
            time(&at);
            at += minimumSecondsDistance;
            LOG("Next closest schedule is at: %d:%d (epoch: %ld) with %f grams of food",
                       nextSchedule.hour, nextSchedule.minute, (long)at, nextSchedule.grams);
            FoodPortionDispenser::scheduleFood(at, nextSchedule.grams);
        }
    }

    PT_END(&pt);
}