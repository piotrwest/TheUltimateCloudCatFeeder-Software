
#include "pt.h"

#include "CheapStepper.h"
#include "lidMotor.h"
#include "hardwareConsts.h"
#include "hallSensors.h"
#include "log4arduino.h"
#include "deadSimpleTimer.h"
#include "inMemoryStore.h"

CheapStepper *LidMotor::stepper;
pt LidMotor::pt;
int LidMotor::closeLidHallOffsetMotorSteps;
int LidMotor::openLidHallOffsetMotorSteps;
bool LidMotor::doOpenLid;
bool LidMotor::doCloseLid;
bool LidMotor::actionInProgress;

#define CLOSING_DIRECTION true
#define OPENING_DIRECTION !CLOSING_DIRECTION

#define LID_STEP_MOTOR_STEP_DELAY_US 2000

void LidMotor::setup(int closeOffsetSteps, int openOffsetSteps)
{
    LOG("Entering LidMotor setup");
    closeLidHallOffsetMotorSteps = closeOffsetSteps;
    openLidHallOffsetMotorSteps = openOffsetSteps;

    pinMode(UC_PIN_LID_STEPPER_1, OUTPUT);
    pinMode(UC_PIN_LID_STEPPER_2, OUTPUT);
    pinMode(UC_PIN_LID_STEPPER_3, OUTPUT);
    pinMode(UC_PIN_LID_STEPPER_4, OUTPUT);
    turnOffLidStepperMotor(); //set all to low - no need to keep the current flowing

    stepper = new CheapStepper(UC_PIN_LID_STEPPER_1, UC_PIN_LID_STEPPER_2, UC_PIN_LID_STEPPER_3, UC_PIN_LID_STEPPER_4);
    stepper -> setTotalSteps(6000000);
    stepper -> setRpm(10); //doesn't matter, as we are "cheating" and setting the delay as 1us in the stepper library through "setTotalSteps".
    //This is to avoid third party library modification. We can't use delay mechanism in the library, as it would block protothreads.
    //Note: setRpm needs to be invoked after setting total steps.

    PT_INIT(&pt);
    LOG("Finished LidMotor setup");
}

void LidMotor::reset()
{
    LOG("Resetting lid position...");
    if (lidIsClosed()) //if lid is closed, lift up until it's not closed
    {
        LOG("Lid is closed, lifting up a little...");
        while (lidIsClosed())
        {
            stepper->step(OPENING_DIRECTION);
            delayMicroseconds(LID_STEP_MOTOR_STEP_DELAY_US);
        }
        LOG("Moved up enough. Moving up just a little bit more.");
        for (int i = 0; i < 800; i++) { //move even a little bit more, just to be sure
            stepper->step(OPENING_DIRECTION);
            delayMicroseconds(LID_STEP_MOTOR_STEP_DELAY_US);
        }
    }

    //now close
    LOG("Closing the lid now.");
    while (!lidIsClosed())
    {
        stepper->step(CLOSING_DIRECTION);
        delayMicroseconds(LID_STEP_MOTOR_STEP_DELAY_US);
    }
    LOG("Lid closed, but stepping offset (%d steps) now.", closeLidHallOffsetMotorSteps);
    for (int i = 0; i < closeLidHallOffsetMotorSteps; i++) {
        stepper->step(CLOSING_DIRECTION);
        delayMicroseconds(LID_STEP_MOTOR_STEP_DELAY_US);
    }
    LOG("Lid fully closed.");
    turnOffLidStepperMotor();
}

void LidMotor::openLid()
{
    if (!doOpenLid && !doCloseLid && !actionInProgress)
    {
        doOpenLid = true;
    }
}

void LidMotor::closeLid()
{
    if (!doOpenLid && !doCloseLid && !actionInProgress)
    {
        doCloseLid = true;
    }
}

int LidMotor::protothread()
{
    static bool currentDirection;
    static bool (*checkLidFunc)();
    static int stepCountSum;
    static int offsetStepCount;
    static DeadSimpleTimer::deadTimer stepperTimer;
    PT_BEGIN(&pt);

    while (1)
    {
        PT_WAIT_UNTIL(&pt, doOpenLid || doCloseLid);

        if (doOpenLid)
        {
            currentDirection = OPENING_DIRECTION;
            checkLidFunc = lidIsOpen;
            offsetStepCount = openLidHallOffsetMotorSteps;
        }
        else if (doCloseLid)
        {
            currentDirection = CLOSING_DIRECTION;
            checkLidFunc = lidIsClosed;
            offsetStepCount = closeLidHallOffsetMotorSteps;
        }
        actionInProgress = true;
        doOpenLid = doCloseLid = false;
        stepCountSum = 0;

        while (!checkLidFunc())
        {
            stepper->step(currentDirection);
            stepCountSum++;
            DeadSimpleTimer::setUs(&stepperTimer, LID_STEP_MOTOR_STEP_DELAY_US);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&stepperTimer));
        }
        LOG("LidMotor - steps until hall sensor triggered: %d", stepCountSum);

        if (stepCountSum > 10) //if moved the lid a lot
        {
            //moving additional steps count. These are not counted towards currentStepCount.
            for (; offsetStepCount >= 0; offsetStepCount--)
            {
                stepper->step(currentDirection);
                DeadSimpleTimer::setUs(&stepperTimer, LID_STEP_MOTOR_STEP_DELAY_US);
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&stepperTimer));
            }

            LOG("LidMotor - finished the action");
            InMemoryStore::putStepsPerLidOperation(stepCountSum);
        }
        else
        {
            LOG("LidMotor - in fact, didn't have to move the lid. Requested state was already there.");
        }

        turnOffLidStepperMotor();
        actionInProgress = false;
    }

    PT_END(&pt);
}

bool LidMotor::lidIsClosed()
{
    return HallSensors::getClosedLidReading() < 2000;
}

bool LidMotor::lidIsOpen()
{
    return HallSensors::getOpenLidReading() < 2000;
}

void LidMotor::turnOffLidStepperMotor()
{
    digitalWrite(UC_PIN_LID_STEPPER_1, 0);
    digitalWrite(UC_PIN_LID_STEPPER_2, 0);
    digitalWrite(UC_PIN_LID_STEPPER_3, 0);
    digitalWrite(UC_PIN_LID_STEPPER_4, 0);
}