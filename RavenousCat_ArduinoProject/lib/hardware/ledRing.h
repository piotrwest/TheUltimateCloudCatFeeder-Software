#pragma once

#include <FastLED.h>
#include "hardwareConsts.h"

enum Error {
    NoSdCard = 1,
    NoConfigFile,
    NoWifisInConfigFile,
    CantWriteCalibrationFile,
    NoCalibrationFile,
    NoCalibrationPointsFile,
    NoFoodScheduleInConfigFile
};

enum CurrentAnimation {
    Clear = 1, //display.. nothing
    GivingFood, //display rainbow (when cat is eating the food)
    PuttingFoodIntoBowl, //display animation when dispensing food into bowl
    JustAboutToOpenLid, //animation when waiting for lid to be opened
    FoodDispensedWaitingAndChillingOut, //food was dispensed, now waiting and chilling out
    WaitForCatArrival //when the cat feeder opened, and waiting for cat arrival to eat the food
};

class LedRing
{

public:
    static void showFatalError(Error error);
    static void showWaitingForNetworkTime(int progress);
	static void setup();
	static void clear();
    static int protothread();
    static void setAnimation(CurrentAnimation animation, int speed = 20);

private:
    static CRGB leds[LED_STRIP_LED_COUNT];

    static struct pt pt;
    static CRGB wheel(byte WheelPos);
    static CurrentAnimation currentAnimation;
    static int currentAnimationSpeed;
};