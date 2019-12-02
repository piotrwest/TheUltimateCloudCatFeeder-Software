#include <Arduino.h>
#include <ArduinoJson.h>
#include "log4arduino.h"
#include "SD.h"
#include "Buzzer.h"
#include "simpleSdCard.h"
#include "pt.h"
#include "deadSimpleTimer.cpp"
#include "hallSensors.h"
#include "ledRing.h"
#include "hardwareConsts.h"
#include "inMemoryStore.h"
#include "HX711.h"
#include "CheapStepper.h"
#include <WiFi.h>
#include "time.h"
#include "NTPSync.h"
#include "wifiPublisher.h"
#include "voltageSensors.h"
#include "lidMotor.h"
#include "tempSensor.h"
#include "loadCells.h"
#include "foodPortionDispenser.h"
#include "foodPortionScheduler.h"
#include "foodMotor.h"
#include "healthMonitor.h"
#include "wifiConnectionManager.h"
#include "putFoodIntoBowl.h"
#include <vector>
#include "spline.h"

//#define WRITE_LOGS_TO_SDCARD

#ifdef WRITE_LOGS_TO_SDCARD
  File logFile;
#endif

void setup()
{
  Serial.begin(57600);
  LOG_INIT(&Serial);

  /*LOG(ESP.getFreeHeap());
    while(true) {
        InMemoryStore::putVoltageLevels(0.1F, 0.2F, 0.3F, 0.4F);
        tick++;
        if(tick % 100 == 0) {
            LOG(tick);
            LOG(ESP.getFreeHeap());
        }
    }*/

  LedRing::setup(); //needed to display errors
  Buzzer::setup();  //just in case to say something :)

  if (SimpleSdCard::mount())
  {
    LOG("Error occured when initializing SD card. Cannot proceed!");
    while (1)
    {
      delay(100);
    };
  }
  //SimpleSdCard::util_deleteFile("/calibCatVals.json");
  SimpleSdCard::initConfig();
  if (InMemoryStore::sdConfig.wifis == NULL)
  {
    LOG("Config was not initialized properly - wifis is NULL");
    while (1)
    {
      delay(100);
    };
  }

  if (InMemoryStore::sdConfig.wifisSize == 0)
  {
    LOG("Config was read, but wifis size is 0");
    while (1)
    {
      delay(100);
    };
  }
  
  LoadCells::setup();
  //SimpleSdCard::util_deleteFile("/calibPoints.jsn");
  //SimpleSdCard::util_writeFile("/calibPoints.jsn", "{\"foodScaleCalibrationPoints\":[0.0,1.0,2.0,3.0,4.0,5.0,6.0],\"catScaleCalibrationPoints\":[0.0,1.0]}");
  LOG("Checking if there is food calibration file on SD card...");
  if (!SimpleSdCard::hasCalibrationValuesFile(CALIBRATION_FOOD_VALUES_FILE_NAME))
  {
    LOG("There is NO food calibration file on SD card...");
    LoadCells::calibrateFoodScale();
  }
  LOG("Checking if there is cat calibration file on SD card...");
  if (!SimpleSdCard::hasCalibrationValuesFile(CALIBRATION_CAT_VALUES_FILE_NAME))
  {
    LOG("There is NO cat calibration file on SD card...");
    LoadCells::calibrateCatScale();
  }
  LOG("Memory FreeHeap before init from SD: %u bytes", ESP.getFreeHeap());
  LoadCells::initFromSDCard();
  LOG("Memory FreeHeap after init from SD: %u bytes", ESP.getFreeHeap());

  /* Some code to test food scale
  LidMotor::setup(InMemoryStore::sdConfig.closeLidHallOffsetMotorSteps, InMemoryStore::sdConfig.openLidHallOffsetMotorSteps);
  LidMotor::openLid();
  for (int i = 0; i < 5000; i++) {
    LidMotor::protothread();
    delay(1);
  }
  while(true) {
    LoadCells::test_getCatWeight();
    LoadCells::test_getFoodAmount(16, 3);
  } 
  */

  #ifdef WRITE_LOGS_TO_SDCARD
    LOG("Redirecting log to SD CARD");
    logFile = SD.open("/loggo.doggo", FILE_APPEND);
    LOG_INIT(&logFile);
    LOG("Logs redirected to SD CARD");
  #endif

  WifiConnectionManager::setup();

  //threads:
  NTPSync::setup(InMemoryStore::sdConfig.gmtOffsetHours, InMemoryStore::sdConfig.daylightOffsetHours); //fetches time (native ESP32 mechanism)
  HallSensors::setup(); //allows to read HallSensors                                                                               
  LidMotor::setup(InMemoryStore::sdConfig.closeLidHallOffsetMotorSteps, InMemoryStore::sdConfig.openLidHallOffsetMotorSteps);
  LidMotor::reset(); //uses HallSensors inside
  FoodMotor::setup();
  VoltageSensors::setup();
  TempSensor::setup();
  HealthMonitor::setup();
  FoodPortionDispenser::setup();

  NTPSync::blockinglyWaitForNetworkTime();
  WifiPublisher::setupAndConnectBlockinglyToAWSIOT();
  FoodPortionScheduler::setup();

  InMemoryStore::setup(); //get ready for readings!

  //after setup, publish the message, so number of hardware restarts are published
  //it might happen that the device will restart before next message will be published.
  LOG("Publishing the metrics to be able to capture hardware resets more precisely.");
  WifiPublisher::publishNow();

  //after setup, reverse food motor, as it might be blocked already
  //I've seen restarts because of blocked food motor (due to temp rising too high with blocked motor).
  FoodMotor::blockingReverseFoodMotor();

  Buzzer::shortBeep();
  delay(200);
  Buzzer::shortBeep();

  /* Some code to schedule 1.2g of food in next 5 min
  time_t now;
  time(&now);
  time_t newTime = now + (5 * 3600);
  FoodPortionDispenser::scheduleFood(newTime, 1.2);*/
}

void calculateMaxLatencies(int64_t maxLatencies[], int64_t timers[], int timersSize) {
  for (int i = 0; i < timersSize - 1; i++) {
    int64_t diff = timers[i + 1] - timers[i];
    maxLatencies[i] = _max(maxLatencies[i], diff);
  }

  //total diff:
  maxLatencies[timersSize - 1] = _max(maxLatencies[timersSize - 1], timers[timersSize - 1] - timers[0]);
}

void cleanTimersArray(int64_t array[], int timersSize) {
  for (int i = 0; i < timersSize; i++) {
    array[i] = 0LL;
  }
}

void loop()
{
  static const int MAX_LOOP_LENGTH_GAP_BETWEEN_MEASURES_IN_MS = 60000;
  int timerIndex = 0;
  int timersSize = 14;
  int64_t timers[timersSize];
  int64_t maxLatencies[timersSize];
  cleanTimersArray(timers, timersSize);
  cleanTimersArray(maxLatencies, timersSize);
  DeadSimpleTimer::deadTimer maxLoopProtothreadDelayTimer;
  DeadSimpleTimer::setMs(&maxLoopProtothreadDelayTimer, MAX_LOOP_LENGTH_GAP_BETWEEN_MEASURES_IN_MS);

  #ifdef WRITE_LOGS_TO_SDCARD
    static const int LOG_FILE_FLASH_IN_MS = 50;
    DeadSimpleTimer::deadTimer logFileFlashTimer;
    DeadSimpleTimer::setMs(&logFileFlashTimer, LOG_FILE_FLASH_IN_MS);
  #endif

  while (1)
  {
    timerIndex = 0;
    timers[timerIndex++] = esp_timer_get_time();
    WifiConnectionManager::protothread(); //connects to wifi

    timers[timerIndex++] = esp_timer_get_time();
    WifiPublisher::protothread();         //pushes the data out to AWS

    timers[timerIndex++] = esp_timer_get_time();
    WifiPublisher::protothreadKeepAliveAndReceive(); //reads the messages from AWS and keeps the connection alive

    timers[timerIndex++] = esp_timer_get_time();
    Buzzer::protothread();                //plays melody, if semaphore is set

    //TODO: remove, not used anymore
    timers[timerIndex++] = esp_timer_get_time();
    //FoodStorage::protothread();           //fetches food level sensor status, every minute stores to InMemoryStore

    timers[timerIndex++] = esp_timer_get_time();
    VoltageSensors::protothread();        //fetches voltages values, every minute stores to InMemoryStore

    timers[timerIndex++] = esp_timer_get_time();
    TempSensor::protothread();            //fetches temperature, every minute stores to InMemoryStore

    timers[timerIndex++] = esp_timer_get_time();
    LidMotor::protothread();              //openes and closes food lid, according to LidMotor::openLid/LidMotor::closeLid

    timers[timerIndex++] = esp_timer_get_time();
    HealthMonitor::protothread();         //monitors minFreeHeapSize

    timers[timerIndex++] = esp_timer_get_time();
    LedRing::protothread();               //displays colors on led ring

    timers[timerIndex++] = esp_timer_get_time();
    FoodPortionDispenser::protothread();  //manages food dispension workflow

    timers[timerIndex++] = esp_timer_get_time();
    NTPSync::protothread();               //sync microsecond time every midnight

    timers[timerIndex++] = esp_timer_get_time();
    FoodPortionScheduler::protothread();  //invoke food portion dispenser according to schedule

    timers[timerIndex++] = esp_timer_get_time();

    calculateMaxLatencies(maxLatencies, timers, timersSize);
    if (DeadSimpleTimer::expired(&maxLoopProtothreadDelayTimer))
    {
      InMemoryStore::putProtothreadLoopLengths(maxLatencies, timersSize);
      cleanTimersArray(maxLatencies, timersSize);
      DeadSimpleTimer::setMs(&maxLoopProtothreadDelayTimer, MAX_LOOP_LENGTH_GAP_BETWEEN_MEASURES_IN_MS);
    }
    #ifdef WRITE_LOGS_TO_SDCARD
      if (DeadSimpleTimer::expired(&logFileFlashTimer))
      {
        logFile.flush();
        logFile.close();
        logFile = SD.open("/loggo.doggo", FILE_APPEND);
        LOG_INIT(&logFile);
        DeadSimpleTimer::setMs(&logFileFlashTimer, LOG_FILE_FLASH_IN_MS);
      }
    #endif
  }
}