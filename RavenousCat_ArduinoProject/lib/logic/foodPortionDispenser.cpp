
#include "pt.h"

#include "foodPortionDispenser.h"
#include "loadCells.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "putFoodIntoBowl.h"
#include "waitForCatArrival.h"
#include "measureConsumption.h"
#include "waitForTheCatToLeave.h"
#include "lidMotor.h"
#include "buzzer.h"
#include <FastLED.h>
#include "ledRing.h"
#include "wifiPublisher.h"

pt FoodPortionDispenser::pt;
DispenserPhase FoodPortionDispenser::phase = DispenserPhase::Unscheduled;
time_t FoodPortionDispenser::atTime;
double FoodPortionDispenser::foodPortionInGrams = 0;
int FoodPortionDispenser::foodDispensedCount = 0;

void FoodPortionDispenser::setup()
{
    LOG("Entering FoodPortionDispenser setup");
    PT_INIT(&pt);
    LOG("Finished FoodPortionDispenser setup");
}

void FoodPortionDispenser::scheduleFood(time_t t, double grams)
{
    if (phase == DispenserPhase::FoodAmountPreparation || phase == DispenserPhase::FoodAmountPrepared || phase == DispenserPhase::Unscheduled)
    {
        if (fabs(foodPortionInGrams - grams) > 0.1 || abs(atTime - t) > 10)
        {
            //if difference is significant (more than 0.1g and more than 10s difference), reset foodDispensedCount
            LOG("Food scheduling: difference between amounts in grams: %f", fabs(foodPortionInGrams - grams));
            LOG("Times: scheduled: %ld previously scheduled: %ld", (long)t, (long)atTime);
            foodDispensedCount = 0;

            if (phase == DispenserPhase::FoodAmountPrepared)
            {
                phase = DispenserPhase::FoodAmountPreparation;
            }
        }
        atTime = t;
        foodPortionInGrams = grams;
        if (phase == DispenserPhase::Unscheduled)
        {
            phase = DispenserPhase::FoodAmountPreparation;
        }
    }
}

int FoodPortionDispenser::protothread()
{
    static struct pt PT_PutFoodIntoBowl;
    static struct pt PT_CheckFoodAmount;
    static struct pt PT_WaitForCatArrival;
    static struct pt PT_MeasureConsumption;
    static struct pt PT_WaitForTheCatToLeave;
    static double diffTime;
    static double *checkFoodAmount = new double;
    static time_t timeOpeningTheLid;
    static time_t timeCatArrived;
    static time_t now;
    static bool *catArrived = new bool;
    static DeadSimpleTimer::deadTimer justAboutToOpenLidTimer;

    PT_BEGIN(&pt);
    while (1)
    {
        PT_WAIT_UNTIL(&pt, phase != DispenserPhase::Unscheduled);

        if (phase == DispenserPhase::FoodAmountPreparation)
        {
            time(&now);
            if (now > atTime)
            {
                LOG("It seems the food was scheduled in the past. Setting phase to Unscheduled");
                phase = DispenserPhase::Unscheduled;
                foodDispensedCount = 0;
                LedRing::setAnimation(CurrentAnimation::Clear);
            }
            else
            {
                if (foodDispensedCount > 3)
                {
                    LOG("It seems the food was properly dispensed to the food bowl. Moving over to FoodAmountPrepared state");
                    phase = DispenserPhase::FoodAmountPrepared;
                    foodDispensedCount = 0;
                    LedRing::setAnimation(CurrentAnimation::Clear);
                }
                else
                {
                    PT_SPAWN(&pt, &PT_PutFoodIntoBowl, PutFoodIntoBowl::protothread(&PT_PutFoodIntoBowl, foodPortionInGrams));
                    foodDispensedCount++;
                }
            }
        }
        else if (phase == DispenserPhase::FoodAmountPrepared)
        {
            time(&now);
            diffTime = fabs(difftime(atTime, now));
            LOG("Food will be given in %f seconds", diffTime);
            LedRing::setAnimation(CurrentAnimation::FoodDispensedWaitingAndChillingOut, (int)diffTime);

            DeadSimpleTimer::setMs(&justAboutToOpenLidTimer, 7000); //reusing justAboutToOpenLidTimer
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&justAboutToOpenLidTimer));

            time(&now);
            diffTime = fabs(difftime(atTime, now));
            if (now > atTime) //this is handled above as well, but handling here as it may get up to this point
            {
                LOG("It seems the food was scheduled in the past!! Setting phase to Unscheduled");
                phase = DispenserPhase::Unscheduled;
                foodDispensedCount = 0;
                LedRing::setAnimation(CurrentAnimation::Clear);
            }
            else if (diffTime < 3 * 60 && now <= atTime) //less than 3min difference, but in future?
            {
                LOG("FoodPortionDispenser is switching to FoodComingReallySoon, as food has to be given within 3 min.");
                phase = DispenserPhase::FoodComingReallySoon; //next phase.
            }
        }
        else if (phase == DispenserPhase::FoodComingReallySoon)
        {
            do
            {
                time(&now);
                diffTime = difftime(atTime, now);
                LedRing::setAnimation(CurrentAnimation::JustAboutToOpenLid, (int)fabs(diffTime));
                LOG("DiffTime is %f, sleeping 1 second if it is greater than 0", diffTime);
                if (diffTime > 0)
                {
                    DeadSimpleTimer::setMs(&justAboutToOpenLidTimer, 1 * 1000);
                    PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&justAboutToOpenLidTimer));
                }
            } while (diffTime > 0);
            phase = DispenserPhase::LidOpening;
            LedRing::setAnimation(CurrentAnimation::Clear);
        }
        else if (phase == DispenserPhase::LidOpening)
        {
            LOG("DispenserPhase::LidOpening");
            LOG("Will open lid in a moment, but first gathering the stats");
            PT_SPAWN(&pt, &PT_CheckFoodAmount, LoadCells::protothreadGetFoodAmount(&PT_CheckFoodAmount, checkFoodAmount, 16, 4, true));
            LOG("Wanted food amount: %f, dispensed: %f", foodPortionInGrams, *checkFoodAmount);
            InMemoryStore::putBeforeLidOpeningStats(*checkFoodAmount, foodPortionInGrams);
            LOG("FoodPortionDispenser is opening lid.");
            time(&timeOpeningTheLid);
            LidMotor::openLid();
            Buzzer::playFoodIntro();
            LedRing::setAnimation(CurrentAnimation::WaitForCatArrival);
            PT_SPAWN(&pt, &PT_WaitForCatArrival, WaitForCatArrival::protothread(&PT_WaitForCatArrival, catArrived, *checkFoodAmount));
            LedRing::setAnimation(CurrentAnimation::Clear);
            time(&timeCatArrived);
            double timeToBowl = fabs(difftime(timeOpeningTheLid, timeCatArrived));
            if (*catArrived)
            {
                LOG("Cat arrived within %f seconds", timeToBowl);
                InMemoryStore::putCatToBowlTime((int)round(timeToBowl));
                phase = DispenserPhase::ConsumptionMeasurement;
            }
            else
            {
                LOG("Cat didn't arrive within timeout (waited %f seconds)", timeToBowl);
                InMemoryStore::putCatToBowlTime(-1 * (int)round(timeToBowl));
                phase = DispenserPhase::LidClosing;
            }
        }
        else if (phase == DispenserPhase::ConsumptionMeasurement)
        {
            LOG("DispenserPhase::ConsumptionMeasurement");
            LedRing::setAnimation(CurrentAnimation::GivingFood);
            PT_SPAWN(&pt, &PT_MeasureConsumption, MeasureConsumption::protothread(&PT_MeasureConsumption, *checkFoodAmount));
            phase = DispenserPhase::LidClosing;
            LedRing::setAnimation(CurrentAnimation::Clear);
        }
        else if (phase == DispenserPhase::LidClosing)
        {
            LOG("DispenserPhase::LidClosing");
            PT_SPAWN(&pt, &PT_WaitForTheCatToLeave, WaitForTheCatToLeave::protothread(&PT_WaitForTheCatToLeave));
            LOG("Closing the lid now");
            LidMotor::closeLid();
            WifiPublisher::publishNow();
            phase = DispenserPhase::Unscheduled;
            LedRing::setAnimation(CurrentAnimation::Clear);
            LOG("Resting 15s after giving the food, so the next portion can be scheduled without any race conditions");
            DeadSimpleTimer::setMs(&justAboutToOpenLidTimer, 15000); //reusing justAboutToOpenLidTimer
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&justAboutToOpenLidTimer));
            //if at last time cat arrived, get after eating stats
            if (*catArrived)
            {
                PT_SPAWN(&pt, &PT_CheckFoodAmount, LoadCells::protothreadGetFoodAmount(&PT_CheckFoodAmount, checkFoodAmount, 16, 4, false));
                LOG("After eating food amount: %f", *checkFoodAmount);
                InMemoryStore::putAfterEatingFoodAmount(*checkFoodAmount);
            }
        }
    }
    PT_END(&pt);
}