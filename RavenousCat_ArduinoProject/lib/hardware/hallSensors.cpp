
#include <Arduino.h>

#include "log4arduino.h"
#include "hallSensors.h"
#include "hardwareConsts.h"

void HallSensors::setup()
{
    LOG("Entering HallSensors setup");

    //TODO: config to switch which one is for closed lid, which one for open lid
    pinMode(UC_PIN_HALL_SENSOR_1, INPUT);
    pinMode(UC_PIN_HALL_SENSOR_2, INPUT);

    LOG("Finished HallSensors setup");
}

/*
 * Returns value from 0 to 4095.
 * 2000...4095 means no magnet is present. 0 means presence of magnetic field.
 */
uint16_t HallSensors::getClosedLidReading()
{
    return analogRead(UC_PIN_HALL_SENSOR_1);
}

uint16_t HallSensors::getOpenLidReading()
{
    return analogRead(UC_PIN_HALL_SENSOR_2);
}