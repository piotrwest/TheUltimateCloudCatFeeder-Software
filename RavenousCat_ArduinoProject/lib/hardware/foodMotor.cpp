
#include "pt.h"

#include "foodMotor.h"
#include "hardwareConsts.h"
#include "hallSensors.h"
#include "log4arduino.h"
#include "deadSimpleTimer.h"
#include "inMemoryStore.h"
#include "putFoodIntoBowl.h"

#define GIVING_DIRECTION false //TODO: move to JSON config
#define TAKING_AWAY_DIRECTION !GIVING_DIRECTION

#define MICROSECONDS_STEP_DELAY 5000

void FoodMotor::setup()
{
    LOG("Entering FoodMotor setup");
    pinMode(UC_PIN_FOOD_DRIVER_EN, OUTPUT);
    pinMode(UC_PIN_FOOD_DRIVER_STEP, OUTPUT);
    pinMode(UC_PIN_FOOD_DRIVER_DIR, OUTPUT);
    setEnabled(false);
    LOG("Finished FoodMotor setup");
}

int FoodMotor::protothreadDoSteps(struct pt *pt, int steps)
{
    static int stepCount;
    static int stepsToPerformAdjusted; //unfortunately with protothreads you cannot modify the method parameter (it will be back unmodified with next invocation)
    static DeadSimpleTimer::deadTimer stepperTimer;

    PT_BEGIN(pt);
    LOG("FoodMotor moving: %d", steps);
    stepCount = 0;
    if (steps > 0) { //positive means give food
        digitalWrite(UC_PIN_FOOD_DRIVER_DIR, GIVING_DIRECTION);
        stepsToPerformAdjusted = steps;
    } else { //negative means... rollback!
        stepsToPerformAdjusted = steps * -1;
        digitalWrite(UC_PIN_FOOD_DRIVER_DIR, TAKING_AWAY_DIRECTION);
    }
    setEnabled(true);

    while (stepCount < stepsToPerformAdjusted)
    {
        stepCount++;
        
        digitalWrite(UC_PIN_FOOD_DRIVER_STEP, HIGH);
        DeadSimpleTimer::setUs(&stepperTimer, MICROSECONDS_STEP_DELAY);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&stepperTimer));

        digitalWrite(UC_PIN_FOOD_DRIVER_STEP, LOW);
        DeadSimpleTimer::setUs(&stepperTimer, MICROSECONDS_STEP_DELAY);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&stepperTimer));
    }

    setEnabled(false);
    LOG("FoodMotor finished moving: %d", stepsToPerformAdjusted);
    PT_END(pt);
}

void FoodMotor::blockingReverseFoodMotor() {
  pt PT_MoveFoodMotor;
  PT_INIT(&PT_MoveFoodMotor);
  LOG("Starting up procedure - moving the food motor back!");
  while(true) {
    if (PT_SCHEDULE(FoodMotor::protothreadDoSteps(&PT_MoveFoodMotor, PutFoodIntoBowl::FoodScrewBlockedReverseSteps)) == 0) //if exited
    {
      break;
    }
  }
}

void FoodMotor::setEnabled(bool enabled) {
    digitalWrite(UC_PIN_FOOD_DRIVER_EN, !enabled); //"When set to a logic high, the outputs are disabled."
}