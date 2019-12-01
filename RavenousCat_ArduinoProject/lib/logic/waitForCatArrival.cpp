
#include "pt.h"

#include "waitForCatArrival.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "HX711.h"
#include "loadCells.h"
#include "foodMotor.h"
#include "CheapStepper.h"
#include "putFoodIntoBowl.h"

#define CAT_PRESENT_MINIMUM_READING 1000

int WaitForCatArrival::protothread(struct pt *pt, bool *catArrived, double catFoodStartAmount)
{
    static struct pt PT_GetCatWeight;
    static struct pt PT_GetFoodAmount;
    static DeadSimpleTimer::deadTimer catArrivalTimeout;
    static double *catWeight = new double;
    static bool timerExpired = false;
    static double *currentFoodAmount = new double;

    PT_BEGIN(pt);
    *currentFoodAmount = catFoodStartAmount;
    timerExpired = false;
    LOG("Waiting for cat arrival %ld ms. Cat food start amount: %f", WaitForCatArrival::catArrivalTimeoutInMs, catFoodStartAmount);
    DeadSimpleTimer::setMs(&catArrivalTimeout, WaitForCatArrival::catArrivalTimeoutInMs);
    PT_SPAWN(pt, &PT_GetCatWeight, LoadCells::protothreadGetCatWeight(&PT_GetCatWeight, catWeight));
    while (*catWeight < CAT_PRESENT_MINIMUM_READING &&
           fabs(catFoodStartAmount - *currentFoodAmount) < PutFoodIntoBowl::CatArrivedFoodAmountDiff &&
           !timerExpired)
    {
        timerExpired = DeadSimpleTimer::expired(&catArrivalTimeout);
        PT_SPAWN(pt, &PT_GetCatWeight, LoadCells::protothreadGetCatWeight(&PT_GetCatWeight, catWeight));
        PT_SPAWN(pt, &PT_GetFoodAmount, LoadCells::protothreadGetFoodAmount(&PT_GetFoodAmount, currentFoodAmount, 14, 3, true));
    }
    if (timerExpired)
    {
        LOG("Timer expired. Cat didn't arrive...");
        *catArrived = false;
    }
    else
    {
        LOG("Cat arrived! Bon Appetit :)");
        *catArrived = true;
    }
    PT_END(pt);
}