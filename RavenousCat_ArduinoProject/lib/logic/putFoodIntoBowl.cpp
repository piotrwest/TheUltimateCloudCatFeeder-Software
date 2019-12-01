
#include "pt.h"

#include "putFoodIntoBowl.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "HX711.h"
#include "loadCells.h"
#include "foodMotor.h"
#include "CheapStepper.h"
#include "ledRing.h"

#define FOOD_STEPS_MOTOR_AT_TIME FOOD_MOTOR_NUMBER_OF_STEPS/2

int PutFoodIntoBowl::protothread(struct pt *pt, double foodPortionInGrams)
{
    static struct pt PT_GetFoodAmount;
    static struct pt PT_MoveFoodMotor;
    static double *foodAmount = new double;
    static double startFoodAmount;
    static double blockedFoodScrewCheckAmount;
    static double endFoodAmount;
    static double tmpFoodDiffVariable;
    static long totalStepsOfFoodMotor;
    static long blockedFoodScrewCheckSteps;
    static DeadSimpleTimer::deadTimer waitForTheFoodToFallDownTimer;

    PT_BEGIN(pt);
    LOG("Dispensing food amount: %f g STARTED", foodPortionInGrams);
    totalStepsOfFoodMotor = 0;
    //if you want to dispense absolute amount of food (on top of what was already in bowl, initialize startFoodAmount accordingly)
    PT_SPAWN(pt, &PT_GetFoodAmount, LoadCells::protothreadGetFoodAmount(&PT_GetFoodAmount, foodAmount, 20, 5, true));
    startFoodAmount = (*foodAmount) * 1.0;
    //or otherwise, if you want to assume on the beginning the start food amount was 0g
    //startFoodAmount = 0;
    LOG("Start amount: %f g", startFoodAmount);
    blockedFoodScrewCheckAmount = startFoodAmount;
    blockedFoodScrewCheckSteps = 0;
    while (!(fabs(foodPortionInGrams - *foodAmount) < PutFoodIntoBowl::CloseEnoughFoodAmount || *foodAmount > foodPortionInGrams))
    {
        LedRing::setAnimation(CurrentAnimation::PuttingFoodIntoBowl);
        LOG("Moving the motor then! Total steps so far: %ld, blocked food screw check steps: %ld", totalStepsOfFoodMotor, blockedFoodScrewCheckSteps);
        PT_SPAWN(pt, &PT_MoveFoodMotor, FoodMotor::protothreadDoSteps(&PT_MoveFoodMotor, FOOD_STEPS_MOTOR_AT_TIME));
        totalStepsOfFoodMotor += FOOD_STEPS_MOTOR_AT_TIME;
        blockedFoodScrewCheckSteps += FOOD_STEPS_MOTOR_AT_TIME;

        DeadSimpleTimer::setMs(&waitForTheFoodToFallDownTimer, 1500);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&waitForTheFoodToFallDownTimer));
        
        PT_SPAWN(pt, &PT_GetFoodAmount, LoadCells::protothreadGetFoodAmount(&PT_GetFoodAmount, foodAmount, 14, 3, true));

        if (blockedFoodScrewCheckSteps > PutFoodIntoBowl::FoodScrewBlockedStepsMade) { //time for blocked screw check
            tmpFoodDiffVariable = fabs(blockedFoodScrewCheckAmount - *foodAmount);
            LOG("Blocked food screw check! Steps made from last check: %ld, started from food at %f g, ended: %f g, diff: %f g",
                blockedFoodScrewCheckSteps, blockedFoodScrewCheckAmount, *foodAmount, tmpFoodDiffVariable);
            if (tmpFoodDiffVariable < PutFoodIntoBowl::FoodScrewBlockedAmountNotDispensed) {
                LOG("That means the food screw is probably blocked! Doing: %ld steps.", PutFoodIntoBowl::FoodScrewBlockedReverseSteps);
                PT_SPAWN(pt, &PT_MoveFoodMotor, FoodMotor::protothreadDoSteps(&PT_MoveFoodMotor, PutFoodIntoBowl::FoodScrewBlockedReverseSteps));
                InMemoryStore::putReverseFoodMotorSteps(PutFoodIntoBowl::FoodScrewBlockedReverseSteps);
                LOG("Resetting blocked food screw check with minus steps, as the food has to travel through tunel again.");
                blockedFoodScrewCheckAmount = *foodAmount;
                blockedFoodScrewCheckSteps = PutFoodIntoBowl::FoodScrewBlockedStepsMade * -1;
            } else {
                LOG("Resetting blocked food screw check.");
                blockedFoodScrewCheckAmount = *foodAmount;
                blockedFoodScrewCheckSteps = 0;
            }
        }
    }
    endFoodAmount = *foodAmount;
    LOG("Done dispensing food. Start: %f g, End: %f g, Delta: %f g", startFoodAmount, endFoodAmount,  fabs(startFoodAmount - endFoodAmount));
    LOG("Wanted-measured diff: %f g, which theoretically should be smaller than CloseEnoughFoodAmount: %f",
               fabs(endFoodAmount - foodPortionInGrams), PutFoodIntoBowl::CloseEnoughFoodAmount);
    if (endFoodAmount > startFoodAmount && (endFoodAmount - startFoodAmount) > PutFoodIntoBowl::CloseEnoughFoodAmount && (endFoodAmount - startFoodAmount) != 0)
    {
        int stepsPerGram = totalStepsOfFoodMotor / (endFoodAmount - startFoodAmount);
        LOG("Dispensed considerable food amount. Steps per gram: %d", stepsPerGram);
        InMemoryStore::putStepsPerFoodGram(stepsPerGram);
    }
    LedRing::setAnimation(CurrentAnimation::Clear);
    PT_END(pt);
}