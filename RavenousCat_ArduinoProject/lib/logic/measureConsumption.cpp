
#include "pt.h"

#include "measureConsumption.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "HX711.h"
#include "loadCells.h"
#include "foodMotor.h"
#include "putFoodIntoBowl.h"
#include "CheapStepper.h"

#define FOOD_CONSUMPTION_TIMEOUT_IN_MS 1000L * 30L * 60L
#define ZEROED_FOOD_READINGS_TO_FINISHED_EATING 4
#define MINIMUM_GAPS_BETWEEN_READINGS_IN_MS 5

int MeasureConsumption::protothread(struct pt *pt, double startFoodAmount)
{
    static struct pt PT_GetCatWeight;
    static struct pt PT_GetFoodAmount;
    static DeadSimpleTimer::deadTimer consumptionTimeout;
    static DeadSimpleTimer::deadTimer maximumMeasureFrequencySafetyTimer;
    static double *foodAmount = new double;
    static double *catWeight = new double;
    static time_t eatingStartTime;
    static time_t eatingEndTime;
    static int zeroedFoodAmountReadingCounter;

    PT_BEGIN(pt);
    LOG("Measuring food consumption...");
    time(&eatingStartTime);
    zeroedFoodAmountReadingCounter = 0;
    DeadSimpleTimer::setMs(&consumptionTimeout, FOOD_CONSUMPTION_TIMEOUT_IN_MS);
    DeadSimpleTimer::setMs(&maximumMeasureFrequencySafetyTimer, MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
    PT_INIT(&PT_GetFoodAmount);
    PT_INIT(&PT_GetCatWeight);

    while (!DeadSimpleTimer::expired(&consumptionTimeout))
    {
        if (PT_SCHEDULE(LoadCells::protothreadGetFoodAmount(&PT_GetFoodAmount, foodAmount, 12, 3, true)) == 0) //if exited
        {
            InMemoryStore::putFoodAmount(*foodAmount);
            PT_INIT(&PT_GetFoodAmount);
            if (*foodAmount < PutFoodIntoBowl::EatingFinishedRemainingFoodAmount)
            {
                LOG("Seems like eating finished? Certainty: %d", zeroedFoodAmountReadingCounter);
                zeroedFoodAmountReadingCounter++;
            } else {
                LOG("Nah, eating not finished! Certainty: %d reseted to 0", zeroedFoodAmountReadingCounter);
                zeroedFoodAmountReadingCounter = 0;
            }
            if (zeroedFoodAmountReadingCounter > ZEROED_FOOD_READINGS_TO_FINISHED_EATING)
            {
                LOG("Yup, finished. Certainty: %d", zeroedFoodAmountReadingCounter);
                break;
            }
        }
        if (PT_SCHEDULE(LoadCells::protothreadGetCatWeight(&PT_GetCatWeight, catWeight)) == 0)
        {
            PT_INIT(&PT_GetCatWeight);
            InMemoryStore::putCatWeight(*catWeight);
        }
        DeadSimpleTimer::setMs(&maximumMeasureFrequencySafetyTimer, MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&maximumMeasureFrequencySafetyTimer));
    }
    time(&eatingEndTime);
    double eatingTimeInSeconds = fabs(difftime(eatingStartTime, eatingEndTime));
    LOG("Measuring food consumption done. Eating start time: %ld, end: %ld, eating time: %f seconds",
               (long)eatingStartTime, (long)eatingEndTime, eatingTimeInSeconds);
    //TODO: end food amount will be slightly lower than in reality. Up to PutFoodIntoBowl::EatingFinishedRemainingFoodAmount.
    //Consider moving it after closing the lid.
    InMemoryStore::putEatingStats(eatingTimeInSeconds, startFoodAmount, *foodAmount);
    PT_END(pt);
}