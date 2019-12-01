#pragma once

#include <vector>

#define CALIBRATION_FOOD_POINTS_FILE_NAME "/calibFoodPoints.json"
#define CALIBRATION_FOOD_VALUES_FILE_NAME "/calibFoodVals.json"
#define CALIBRATION_CAT_POINTS_FILE_NAME "/calibCatPoints.json"
#define CALIBRATION_CAT_VALUES_FILE_NAME "/calibCatVals.json"

class SimpleSdCard
{
 public:
   static bool mount();
   static void initConfig();
   static bool hasCalibrationValuesFile(const char *path);
   static void readCalibrationPoints(std::vector<float> *&scalePoints, const char *path);
   static void saveCalibrationValuesFile(const std::vector<float> &scaleValues, const char *path);
   static void readCalibrationValuesFile(std::vector<float> *&scaleValues, const char *path);
   static void util_readFile(const char *path);
   static void util_writeFile(const char *path, const char *message);
   static void util_deleteFile(const char * path);
};