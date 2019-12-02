#pragma once

#include "CheapStepper.h"

class LidMotor
{

public:
	static void setup(int closeOffsetSteps, int openOffsetSteps);
	static void reset();
	static void openLid();
	static void closeLid();
	static int protothread();

private:
	static int closeLidHallOffsetMotorSteps;
	static int openLidHallOffsetMotorSteps;
	static CheapStepper *stepper;
	static bool lidIsClosed();
	static bool lidIsOpen();
	static void turnOffLidStepperMotor();
	
	static struct pt pt;
	static bool doOpenLid;
	static bool doCloseLid;
	static bool actionInProgress;
};