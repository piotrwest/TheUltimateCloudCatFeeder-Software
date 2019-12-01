#pragma once

//ESP32 Arduino defined pins: https://github.com/espressif/arduino-esp32/blob/master/variants/esp32/pins_arduino.h#L20
//ESP32 pinout: https://github.com/MHEtLive/arduino-esp32
#include <pins_arduino.h>

/* SD card connection
 * Connection Diagram: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 * SD Card Pin Number | SD SPI | SPI MEANING | ESP32 Pin
 * 1                  | CS     | CS, SS      | GPIO5
 * 2                  | DI     | MOSI        | GPIO23
 * 3                  | VSS1   | GND         | 
 * 4                  | VDD    | V+, 3.3V    | 
 * 5                  | SCLK   | SCK         | GPIO18
 * 6                  | VSS2   | GND         | 
 * 7                  | DO     | MISO        | GPIO19
 * 8                  | -      |             | 
 * 9                  | -      |             | 
 */
static const uint8_t UC_PIN_SD_CARD = SS; //GPIO5, SPI SS, IO5, Physical 34

/*
 * Lid Stepper (28BY-J48 + ULN2004)
 */
static const uint8_t UC_PIN_LID_STEPPER_1 = 4; //GPIO4, ADC2_0, IO4, Physical 24
static const uint8_t UC_PIN_LID_STEPPER_2 = 0; //GPIO0, ADC2_1, IO0, Physical 23
static const uint8_t UC_PIN_LID_STEPPER_3 = 2; //GPIO2, ADC2_2, IO2, Physical 22
static const uint8_t UC_PIN_LID_STEPPER_4 = 15; //GPIO15, ADC2_3, IO15, Physical 21

/*
 * Food Motor (A4988 + NEMA17)
 */
static const uint8_t UC_PIN_FOOD_DRIVER_STEP = A16; //GPIO14, ADC2_6, IO14, Physical 17
static const uint8_t UC_PIN_FOOD_DRIVER_DIR = 12; //GPIO12, ADC2_5, IO12, Physical 18
static const uint8_t UC_PIN_FOOD_DRIVER_EN = 13; //GPIO13

/*
 * Hall sensors
 * 3x hall sensors connected to analog inputs.
 */
static const uint8_t UC_PIN_HALL_SENSOR_1 = A0; //GPIO36, ADC1_0, SVP, Physical 5
static const uint8_t UC_PIN_HALL_SENSOR_2 = A3; //GPIO39, ADC1_3, SVN, Physical 8

/*
 * Voltage sensors
 * 3x voltage sensors, with voltage dividers.
 * 1: 3.3V or 5V - depending on the jumper
 * 2: 3.3V or 5V - depending on the jumper
 * 2: 5V or 9V..25V - depending on the jumper
 */
static const uint8_t UC_PIN_VOLTAGE_SENSOR_1 = A7; //GPIO35, ADC1_7, IO35, Physical 11
static const uint8_t UC_PIN_VOLTAGE_SENSOR_2 = A6; //GPIO34, ADC1_6, IO34, Physical 10
static const uint8_t UC_PIN_VOLTAGE_SENSOR_3 = A4; //GPIO32, ADC1_4, IO32, Physical 12

/*
 * Load scale sensors
 * 3 load scales - one for food, two for cat weight
 * Cat Load Scales are using single SCK line.
 */
static const uint8_t UC_PIN_CAT_SCALE_DT1 = A5; //GPIO33, ADC1_5, IO33, Physical 13
static const uint8_t UC_PIN_CAT_SCALE_DT2 = A19; //GPIO26, ADC2_9, IO26, Physical 15
static const uint8_t UC_PIN_CAT_SCALE_SCK = A18; //GPIO25, ADC2_8, IO25, Physical 14
static const uint8_t UC_PIN_FOOD_SCALE_DT = 17; //GPIO17, IO17, Physical 27
static const uint8_t UC_PIN_FOOD_SCALE_SCK = 16; //GPIO16, IO16, Physical 25

/*
 * Buzzer
 */
static const uint8_t BUZZER_CHANNEL = 0;
static const uint8_t BUZZER_RESOLUTION_BITS = 8;
static const int BUZZER_FREQUENCY = 2000;
static const uint8_t UC_PIN_BUZZER = 21; //GPIO21, IO21, Physical 42

/*
 * LED strip
 */
static const uint8_t UC_PIN_LED_STRIP = 22; //GPIO22, Physical 39
static const uint8_t LED_STRIP_LED_COUNT = 57;

/*
 * 1-wire temp sensor, external
 */
static const uint8_t UC_PIN_TEMP_DS18B20 = A17; //GPIO27, ADC2_7, IO27, Physical 16;


static const int FOOD_MOTOR_NUMBER_OF_STEPS = 400; //17HS2408 has 200 steps (1.8 degrees per step), but we are using halfstepping. 