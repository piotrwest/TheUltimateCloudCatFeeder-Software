#pragma once

#include "HX711.h"
#include "spline.h"

class LoadCells
{

public:
  static void setup();
  static void calibrateFoodScale();
  static void calibrateCatScale();
  static void initFromSDCard();
  static int protothreadGetFoodAmount(struct pt *pt, double *result, int supersamplingCount, int trimCount, bool limitReadingToCalibrationRange);
  static int protothreadGetCatWeight(struct pt *pt, double *result);
  static void test_getFoodAmount(int supersamplingCount, int trimCount);
  static void test_getCatWeight();

private:
  static void calibrateAScale(const char *pointsPath, const char *outValuesPath, long (*readScaleFunc)());

  static long readCatScaleSum();
  static long readFoodScale();

  static long minCatScaleReading; 
  static long maxCatScaleReading;
  static long minFoodScaleReading; 
  static long maxFoodScaleReading;
  static long trimFoodScaleReadingToCalibrationLimit(long reading);
  static long trimCatScaleReadingToCalibrationLimit(long reading);

  static HX711 *catScale1;
  static HX711 *catScale2;
  static HX711 *foodScale;
  static tk::spline *foodScaleSpline;
  static tk::spline *catScaleSpline;
};