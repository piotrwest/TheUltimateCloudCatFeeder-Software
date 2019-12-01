#pragma once

#include "time.h"

enum DispenserPhase {
	Unscheduled = 1,
    FoodAmountPreparation = 2,
    FoodAmountPrepared = 3,
	FoodComingReallySoon = 4,
	LidOpening = 5,
	ConsumptionMeasurement = 6,
	LidClosing = 7
};

class FoodPortionDispenser
{
public:
	static void setup();
	static int protothread();
	static void scheduleFood(time_t t, double grams);
private:
	static time_t atTime;
	static double foodPortionInGrams;
	static int foodDispensedCount;

	static DispenserPhase phase;
	static struct pt pt;
};