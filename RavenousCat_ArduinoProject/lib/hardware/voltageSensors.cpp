
#include <Arduino.h>

#include "voltageSensors.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "pt.h"
#include "log4arduino.h"
#include "inMemoryStore.h"

pt VoltageSensors::pt;

void VoltageSensors::setup()
{
    LOG("Entering VoltageSensors setup");

    //voltage sensors
    pinMode(UC_PIN_VOLTAGE_SENSOR_1, INPUT);
    pinMode(UC_PIN_VOLTAGE_SENSOR_2, INPUT);
    pinMode(UC_PIN_VOLTAGE_SENSOR_3, INPUT);
    PT_INIT(&pt);

    LOG("Finished VoltageSensors setup");
}

int VoltageSensors::protothread()
{
    static const int NUMBER_OF_MEASURES = 100;
    static const int GAP_BETWEEN_MEASURES_IN_MS = 600; //600ms*100 measures = 60s
    static int measureNumber;
    static uint32_t measureSum1;
    static uint32_t measureSum2;
    static uint32_t measureSum3;
    static DeadSimpleTimer::deadTimer measureTimer;

    PT_BEGIN(&pt);

    while (1)
    {
        measureSum1 = measureSum2 = measureSum3 = 0;
        measureNumber = 0;

        for (measureNumber = 0; measureNumber < NUMBER_OF_MEASURES; measureNumber++)
        {
            //LOG(F("VoltageSensors - measure number: ");
            //LOG(measureNumber);
            //Max is 12bit (4,095)
            //Ref point is 3.3V

            measureSum1 += analogRead(UC_PIN_VOLTAGE_SENSOR_1);
            measureSum2 += analogRead(UC_PIN_VOLTAGE_SENSOR_2);
            measureSum3 += analogRead(UC_PIN_VOLTAGE_SENSOR_3);

            DeadSimpleTimer::setMs(&measureTimer, GAP_BETWEEN_MEASURES_IN_MS);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&measureTimer));
        }

        float r1, r2, r3;
        //TODO: move the numbers below to JSON config, allow calibration procedure
        r1 = ((measureSum1 * 0.00165375F) / NUMBER_OF_MEASURES) + 0.30221375;
        r2 = ((measureSum2 * 0.00164552F) / NUMBER_OF_MEASURES) + 0.30569402;
        r3 = ((measureSum3 * 0.00900596F) / NUMBER_OF_MEASURES) + 1.75809145;

        LOG("VoltageSensors - results are: V1: %f, V2: %f, V3: %f",
                   r1, r2, r3);

        InMemoryStore::putVoltageLevels(r1, r2, r3, 0);
    }

    PT_END(&pt);
}