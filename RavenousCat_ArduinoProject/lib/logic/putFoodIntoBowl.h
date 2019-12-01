#pragma once

#include "hardwareConsts.h"

class PutFoodIntoBowl
{
public:
	static int protothread(struct pt *pt, double foodPortionInGrams);
	static constexpr double CloseEnoughFoodAmount = 0.25;
	static constexpr double CatArrivedFoodAmountDiff = 0.3;
	static constexpr double EatingFinishedRemainingFoodAmount = 0.75;

	static constexpr double FoodScrewBlockedAmountNotDispensed = 0.75; //if at least this amount of food was not dispensed
	static constexpr long FoodScrewBlockedStepsMade = 17 * FOOD_MOTOR_NUMBER_OF_STEPS; //during this many steps
	static constexpr long FoodScrewBlockedReverseSteps = -3 * FOOD_MOTOR_NUMBER_OF_STEPS; //reverse this many steps
private:
};