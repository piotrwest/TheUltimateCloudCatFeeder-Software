
#include "tempSensor.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "pt.h"
#include "log4arduino.h"
#include "inMemoryStore.h"

pt TempSensor::pt;
//TODO: move to JSON config, allow configuration
DeviceAddress TempSensor::sensorAddress  = { 0x28, 0xFF, 0xCA, 0x5, 0x24, 0x17, 0x3, 0x3C };
OneWire TempSensor::oneWire = OneWire(UC_PIN_TEMP_DS18B20);
DallasTemperature TempSensor::sensors = DallasTemperature(&TempSensor::oneWire);

void TempSensor::setup()
{
    LOG("Entering TempSensor setup");
    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.setResolution(12);

    PT_INIT(&pt);
    LOG("Finished TempSensor setup");
}

void TempSensor::soLookingForAddressYouAre() {
    while(true) {   
        byte i;
        byte addr[8];
        
        if (!oneWire.search(addr)) {
            Serial.println(F(" No more addresses."));
            Serial.println();
            oneWire.reset_search();
            delay(250);
        }
        Serial.print(" ROM =");
        for (i = 0; i < 8; i++) {
            Serial.write(' ');
            Serial.print(addr[i], HEX);
        }
    }
}

float TempSensor::getTemp(){
    sensors.requestTemperatures(); // Send the command to get temperatures
    return sensors.getTempC(sensorAddress);
}

int TempSensor::protothread()
{
    static const int NUMBER_OF_MEASURES = 30;
    static const int GAP_BETWEEN_MEASURES_IN_MS = 1000; //1000ms*30 measures = 60s
    static int measureNumber;
    static float measureSum;
    static DeadSimpleTimer::deadTimer measureTimer;

    PT_BEGIN(&pt);

    while(1) {
        measureSum = 0;
        measureNumber = 0;

        for (measureNumber = 0; measureNumber < NUMBER_OF_MEASURES; measureNumber++) {
            sensors.requestTemperatures(); // Send the command to get temperatures
            DeadSimpleTimer::setMs(&measureTimer, GAP_BETWEEN_MEASURES_IN_MS); 
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&measureTimer));

            float result = sensors.getTempC(sensorAddress);
            if (result != DEVICE_DISCONNECTED_C) {
                measureSum += result;
            } else {
                LOG("WARN! Got device disconnected from the TempSensor!");
                measureNumber--;
            }
        }

        float finalResult;
        finalResult = measureSum/NUMBER_OF_MEASURES;
        LOG("TempSensor - result is: %f *C", finalResult);

        InMemoryStore::putTemp(finalResult);
    }

    PT_END(&pt);
}