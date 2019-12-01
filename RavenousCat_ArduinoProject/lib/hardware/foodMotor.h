#pragma once

#include "CheapStepper.h"

class FoodMotor
{

public:
	static void setup();
	static int protothreadDoSteps(struct pt *pt, int steps);
	static void blockingReverseFoodMotor();
private:
	static void setEnabled(bool enabled);
};