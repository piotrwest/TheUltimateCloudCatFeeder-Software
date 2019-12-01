#include <vector>
#include "FS.h"
#include "SD.h"

#include "simpleSdCard.h"
#include <ArduinoJson.h>
#include "ledRing.h"
#include "log4arduino.h"
#include "hardwareConsts.h"
#include "inMemoryStore.h"

#define CONFIG_FILE_NAME "/conf.json"

const size_t calibrationFileBufferSize = 2 * JSON_ARRAY_SIZE(60) + JSON_OBJECT_SIZE(2) + 460;

bool SimpleSdCard::mount()
{
    if (!SD.begin(UC_PIN_SD_CARD))
    {
        LOG("ERROR! Card Mount Failed");
        LedRing::showFatalError(Error::NoSdCard);
        return true;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        LOG("ERROR! No SD card attached");
        LedRing::showFatalError(Error::NoSdCard);
        return true;
    }

    LOG("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        LOG("MMC");
    }
    else if (cardType == CARD_SD)
    {
        LOG("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        LOG("SDHC");
    }
    else
    {
        LOG("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    LOG("SD Card Size: %lluMB", cardSize);
    return false;
}

void readConfigFile(fs::FS &fs, const char *path)
{
    LOG("Reading file: %s", path);

    File file = fs.open(path);
    if (!file)
    {
        LOG("ERROR! Failed to open file for reading");
        LedRing::showFatalError(Error::NoConfigFile);
        return;
    }

    LOG("Reading from file.");
    const size_t confBufferSize = 24 * JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(24) + JSON_ARRAY_SIZE(6) + 6 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(7) + 360;
    DynamicJsonBuffer jsonBuffer(confBufferSize);
    JsonObject &root = jsonBuffer.parseObject(file);
    JsonArray &wifis = root["wifis"];

    if (wifis.size() <= 0)
    {
        LedRing::showFatalError(Error::NoWifisInConfigFile);
        return;
    }

    InMemoryStore::sdConfig.wifis = new InMemoryStore::WifiConfig[wifis.size()];
    InMemoryStore::sdConfig.wifisSize = wifis.size();

    for (int i = 0; i < wifis.size(); i++)
    {
        LOG("Getting wifi number %d:", i);
        LOG("Wifi ID: %s", wifis[i]["id"].as<char *>());
        int idLength = strlen(wifis[i]["id"].as<char *>()) + 1;
        InMemoryStore::sdConfig.wifis[i].id = new char[idLength];
        strlcpy(InMemoryStore::sdConfig.wifis[i].id, wifis[i]["id"].as<char *>(), idLength);

        int passLength = strlen(wifis[i]["pass"].as<char *>()) + 1;
        InMemoryStore::sdConfig.wifis[i].pass = new char[passLength];
        strlcpy(InMemoryStore::sdConfig.wifis[i].pass, wifis[i]["pass"].as<char *>(), passLength);
    }

    InMemoryStore::sdConfig.gmtOffsetHours = root["gmtOffsetHours"];
    InMemoryStore::sdConfig.daylightOffsetHours = root["daylightOffsetHours"];

    InMemoryStore::sdConfig.closeLidHallOffsetMotorSteps = root["closeLidHallOffsetMotorSteps"];
    InMemoryStore::sdConfig.openLidHallOffsetMotorSteps = root["openLidHallOffsetMotorSteps"];

    //AWS IOT config
    int awsHostAddressLength = strlen(root["awsHostAddress"].as<char *>()) + 1;
    InMemoryStore::sdConfig.awsHostAddress = new char[awsHostAddressLength];
    strlcpy(InMemoryStore::sdConfig.awsHostAddress, root["awsHostAddress"].as<char *>(), awsHostAddressLength);

    int awsClientIdLength = strlen(root["awsClientId"].as<char *>()) + 1;
    InMemoryStore::sdConfig.awsClientId = new char[awsClientIdLength];
    strlcpy(InMemoryStore::sdConfig.awsClientId, root["awsClientId"].as<char *>(), awsClientIdLength);

    //foodSchedule
    JsonArray &foodSchedule = root["foodSchedule"];

    if (foodSchedule.size() <= 0)
    {
        LedRing::showFatalError(Error::NoFoodScheduleInConfigFile);
        return;
    }

    InMemoryStore::sdConfig.foodSchedule = new InMemoryStore::FoodPortionSchedule[foodSchedule.size()];
    InMemoryStore::sdConfig.foodScheduleSize = foodSchedule.size();

    for (int i = 0; i < foodSchedule.size(); i++)
    {
        LOG("Schedule number %i at %d:%d with  mass: %f g",
                   i,
                   foodSchedule[i]["hour"].as<int>(),
                   foodSchedule[i]["minute"].as<int>(),
                   foodSchedule[i]["grams"].as<float>());
        InMemoryStore::sdConfig.foodSchedule[i].hour = foodSchedule[i]["hour"].as<int>();
        InMemoryStore::sdConfig.foodSchedule[i].minute = foodSchedule[i]["minute"].as<int>();
        InMemoryStore::sdConfig.foodSchedule[i].grams = foodSchedule[i]["grams"].as<float>();
    }
    file.close();
}

void SimpleSdCard::saveCalibrationValuesFile(const std::vector<float> &scaleValues, const char *path)
{
    if (SD.exists(path))
    {
        if (SD.remove(path))
        {
            LOG("File deleted");
        }
        else
        {
            LOG("ERROR! Delete failed");
        }
    }

    LOG("Saving calibration file: %s", path);

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        LOG("ERROR! Failed to open file for writing: %s", path);
        LedRing::showFatalError(Error::CantWriteCalibrationFile);
        return;
    }

    DynamicJsonBuffer jsonBuffer(calibrationFileBufferSize);
    JsonObject &root = jsonBuffer.createObject();

    JsonArray &scaleCalibrationValues = root.createNestedArray("calibrationValues");
    for (int i = 0; i < scaleValues.size(); i++)
    {
        scaleCalibrationValues.add((float)scaleValues.at(i));
    }

    root.printTo(file);

    LOG("Wrote calibration file.");
    file.close();
}

void SimpleSdCard::readCalibrationPoints(std::vector<float> *&scalePoints, const char *path)
{
    LOG("Reading file: %s", path);

    File file = SD.open(path);
    if (!file)
    {
        LOG("ERROR! Failed to open calibration points file for reading");
        LedRing::showFatalError(Error::NoCalibrationPointsFile);
    }

    LOG("Reading from calibration points file.");
    DynamicJsonBuffer jsonBuffer(calibrationFileBufferSize);
    JsonObject &root = jsonBuffer.parseObject(file);

    JsonArray &calibrationPoints = root["calibrationPoints"];
    LOG("Discovered %d calibrationPoints", calibrationPoints.size());
    scalePoints = new std::vector<float>(calibrationPoints.size());
    for (int i = 0; i < calibrationPoints.size(); i++)
    {
        (*scalePoints)[i] = calibrationPoints[i].as<float>();
    }

    file.close();
}

bool SimpleSdCard::hasCalibrationValuesFile(const char *path)
{
    return SD.exists(path);
}

//TODO: scaleValues can be just returned, it wasn't as this function returned two values
void SimpleSdCard::readCalibrationValuesFile(std::vector<float> *&scaleValues, const char *path)
{
    LOG("Reading file: %s", path);

    File file = SD.open(path);
    if (!file)
    {
        LOG("ERROR! Failed to open calibration file for reading");
        LedRing::showFatalError(Error::NoCalibrationFile);
    }

    LOG("Reading from calibration file.");

    DynamicJsonBuffer jsonBuffer(calibrationFileBufferSize);
    JsonObject &root = jsonBuffer.parseObject(file);

    JsonArray &scaleArray = root["calibrationValues"];
    scaleValues = new std::vector<float>(scaleArray.size());
    for (int i = 0; i < scaleArray.size(); i++)
    {
        (*scaleValues)[i] = scaleArray[i].as<float>();
    }

    file.close();
}

void SimpleSdCard::initConfig()
{
    readConfigFile(SD, CONFIG_FILE_NAME);
}

void SimpleSdCard::util_readFile(const char *path)
{
    LOG("Reading file: %s", path);

    File file = SD.open(path);
    if (!file)
    {
        LOG("ERROR! Failed to open file for reading");
        return;
    }

    LOG("Read from file: ");
    while (file.available())
    {
        LOG(file.read());
    }
    file.close();
}

void SimpleSdCard::util_writeFile(const char *path, const char *message)
{
    LOG("Writing file: %s", path);

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        LOG("ERROR! Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        LOG("File written");
    }
    else
    {
        LOG("Write failed");
    }
    file.close();
}

void SimpleSdCard::util_deleteFile(const char *path)
{
    if (SD.exists(path))
    {
        if (SD.remove(path))
        {
            LOG("File deleted");
        }
        else
        {
            LOG("ERROR! Delete failed");
        }
    }
}