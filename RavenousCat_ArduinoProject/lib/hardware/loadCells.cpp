
#include <Arduino.h>

#include "hardwareConsts.h"
#include "buzzer.h"
#include "log4arduino.h"
#include "simpleSdCard.h"
#include "deadSimpleTimer.h"
#include <ArduinoJson.h>
#include <pt.h>
#include "loadCells.h"

HX711 *LoadCells::foodScale;
HX711 *LoadCells::catScale1;
HX711 *LoadCells::catScale2;
tk::spline *LoadCells::foodScaleSpline;
tk::spline *LoadCells::catScaleSpline;
long LoadCells::minCatScaleReading;
long LoadCells::maxCatScaleReading;
long LoadCells::minFoodScaleReading;
long LoadCells::maxFoodScaleReading;

//HX711 supports 80Hz maximum frequency, meaning each result will be available in 12.5ms. Setting 5ms to be sure 
//we get the maximum possible frequency, but in case of hardware failure (faulty result available immediately)
//we need a slow-down mechanism - that's this 5ms.
#define MINIMUM_GAPS_BETWEEN_READINGS_IN_MS 5

void LoadCells::setup()
{
    LOG("Entering LoadCells setup");

    foodScale = new HX711();
    foodScale->begin(UC_PIN_FOOD_SCALE_DT, UC_PIN_FOOD_SCALE_SCK, 128);

    catScale1 = new HX711();
    catScale1->begin(UC_PIN_CAT_SCALE_DT1, UC_PIN_CAT_SCALE_SCK, 128);
    catScale2 = new HX711();
    catScale2->begin(UC_PIN_CAT_SCALE_DT2, UC_PIN_CAT_SCALE_SCK, 128);

    LOG("Finished LoadCells setup");
}

float trimAndCalculateAverage(std::vector<long> v, int trimTimes)
{
    if (v.size() == 0) {
        LOG("ERROR! trimAndCalculateAverage vector size 0!!");
    }
    if (v.size() - trimTimes*2 <= 0) {
        LOG("ERROR! trimAndCalculateAverage vector size 0 after trimming!!");
    }

    std::sort(v.begin(), v.end());
    /*LOGf("TRIMING BEFORE %d:\n", trimTimes);
    for (int i = 0; i < v.size(); ++i)
    {
        LOGf2("%d\t%d\n", i, v.at(i));
    }*/
    for (int trim = 0; trim < trimTimes; trim++)
    {
        v.pop_back();
    }
    
    std::reverse(v.begin(), v.end());
    for (int trim = 0; trim < trimTimes; trim++)
    {
        v.pop_back();
    }
    /*
    LOGf("TRIMING AFTER %d:\n", trimTimes);
    for (int i = 0; i < v.size(); ++i)
    {
        LOGf2("%d\t%d\n", i, v.at(i));
    }*/
    long long sumOfWeights = 0;
    std::for_each(v.begin(), v.end(), [&](long n) {
        sumOfWeights += n;
    });
    if (v.size() == 0) {
        LOG("WARN! Received 0 elements for trimAndCalculateAverage!");
        return 0;
    }
    return (float)(((double)sumOfWeights) / v.size());
}

std::vector<float> *getMeasures(std::vector<float> *scalePoints, long (*readScaleFunc)(), int numberOfMeasuresPerWeight, int trimOutliersRepeatCount)
{
    std::vector<float> *measures = new std::vector<float>((*scalePoints).size());

    for (int i = 0; i < (*scalePoints).size(); i++)
    {
        LOG("Doing weight number: %d with mass: %f g", i, (*scalePoints)[i]);
        delay(15000);
        Buzzer::longBeep();
        delay(1000);

        Buzzer::shortBeep();
        std::vector<long> measuresThisWeight;
        for (int measure = 0; measure < numberOfMeasuresPerWeight; measure++)
        {
            measuresThisWeight.push_back(readScaleFunc());
        }
        Buzzer::shortBeep();

        float avg = trimAndCalculateAverage(measuresThisWeight, trimOutliersRepeatCount);
        (*measures)[i] = avg;

        LOG("Avg: %f", avg);
    }
    LOG("Measures done, have: %d values", measures->size());

    return measures;
}

void LoadCells::calibrateAScale(const char *pointsPath, const char *outValuesPath, long (*readScaleFunc)())
{
    int numberOfMeasuresPerWeight = 60;
    int trimOutliersRepeatCount = 10;

    Buzzer::longBeep();
    Buzzer::longBeep();
    LOG("Calibrating. Reading the points from %s.", pointsPath);
    std::vector<float> *scalePoints = nullptr;
    SimpleSdCard::readCalibrationPoints(scalePoints, pointsPath);
    LOG("File: %s. Number of points to calibrate: %d", pointsPath, scalePoints->size());

    std::vector<float> *scaleValues = getMeasures(scalePoints, readScaleFunc, numberOfMeasuresPerWeight, trimOutliersRepeatCount);

    Buzzer::longBeep();
    Buzzer::longBeep();
    LOG("Calibration done. Saving...");

    SimpleSdCard::saveCalibrationValuesFile(*scaleValues, outValuesPath);
    LOG("Calibration saved.");
    delete scalePoints;
    delete scaleValues;
}

void LoadCells::calibrateFoodScale()
{
    LoadCells::calibrateAScale(CALIBRATION_FOOD_POINTS_FILE_NAME, CALIBRATION_FOOD_VALUES_FILE_NAME, LoadCells::readFoodScale);
}

void LoadCells::calibrateCatScale()
{
    LoadCells::calibrateAScale(CALIBRATION_CAT_POINTS_FILE_NAME, CALIBRATION_CAT_VALUES_FILE_NAME, LoadCells::readCatScaleSum);
}

void LoadCells::initFromSDCard()
{
    LOG("Initializing from SD card...");
    foodScaleSpline = new tk::spline;
    catScaleSpline = new tk::spline;


    std::vector<float> *foodScalePoints = nullptr;
    SimpleSdCard::readCalibrationPoints(foodScalePoints, CALIBRATION_FOOD_POINTS_FILE_NAME);

    std::vector<float> *catScalePoints = nullptr;
    SimpleSdCard::readCalibrationPoints(catScalePoints, CALIBRATION_CAT_POINTS_FILE_NAME);


    std::vector<float> *foodScaleValues = nullptr;
    SimpleSdCard::readCalibrationValuesFile(foodScaleValues, CALIBRATION_FOOD_VALUES_FILE_NAME);

    std::vector<float> *catScaleValues = nullptr;
    SimpleSdCard::readCalibrationValuesFile(catScaleValues, CALIBRATION_CAT_VALUES_FILE_NAME);


    std::vector<double> catScalePointsDouble(catScalePoints->begin(), catScalePoints->end());
    std::vector<double> catScaleValuesDouble(catScaleValues->begin(), catScaleValues->end());
    LOG("Cat scale calibration retrieved of size %d:", catScaleValuesDouble.size());
    for (int i = 0; i < catScalePointsDouble.size(); ++i)
    {
        LOG("%f\t%f", catScalePointsDouble.at(i), catScaleValuesDouble.at(i));
    }

    std::vector<double> foodScalePointsDouble(foodScalePoints->begin(), foodScalePoints->end());
    std::vector<double> foodScaleValuesDouble(foodScaleValues->begin(), foodScaleValues->end());
    LOG("Food scale calibration retrieved of size %d:", foodScaleValuesDouble.size());
    for (int i = 0; i < foodScalePointsDouble.size(); ++i)
    {
        LOG("%f\t%f", foodScalePointsDouble.at(i), foodScaleValuesDouble.at(i));
    }

    minCatScaleReading = (long)round(*std::min_element(catScaleValues->begin(), catScaleValues->end()));
    maxCatScaleReading = (long)round(*std::max_element(catScaleValues->begin(), catScaleValues->end()));
    minFoodScaleReading = (long)round(*std::min_element(foodScaleValues->begin(), foodScaleValues->end()));
    maxFoodScaleReading = (long)round(*std::max_element(foodScaleValues->begin(), foodScaleValues->end()));

    LOG("Cat scale min: %ld max: %ld", minCatScaleReading, maxCatScaleReading);
    LOG("Food scale min: %ld max: %ld", minFoodScaleReading, maxFoodScaleReading);

    foodScaleSpline->set_points(foodScaleValuesDouble, foodScalePointsDouble);
    catScaleSpline->set_points(catScaleValuesDouble, catScalePointsDouble);

    delete catScalePoints;
    delete foodScalePoints;
    delete catScaleValues;
    delete foodScaleValues;
}

long LoadCells::readCatScaleSum()
{
    return catScale1->read() + catScale2->read();
}

long LoadCells::readFoodScale()
{
    return foodScale->read();
}

long LoadCells::trimFoodScaleReadingToCalibrationLimit(long reading)
{
    //TODO: emit metric how much the value is trimed
    return std::min(std::max(reading, minFoodScaleReading), maxFoodScaleReading);
}

long LoadCells::trimCatScaleReadingToCalibrationLimit(long reading)
{
    //TODO: emit metric how much the value is trimed
    return std::min(std::max(reading, minCatScaleReading), maxCatScaleReading);
}

void LoadCells::test_getFoodAmount(int supersamplingCount, int trimCount)
{
    static std::vector<long> measures;
    static int measureNumber;
    measures.clear();
    for (measureNumber = 0; measureNumber < supersamplingCount; measureNumber++)
    {
        measures.push_back(readFoodScale());
        delay(MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
    }
    float avg = trimAndCalculateAverage(measures, trimCount);
    double result = (*foodScaleSpline)((double)avg);
    LOG("Avg food reading: %f (scale units) --spline--> %f g", avg, result);
}


int LoadCells::protothreadGetFoodAmount(struct pt *pt, double *result, int supersamplingCount, int trimCount, bool limitReadingToCalibrationRange)
{
    static std::vector<long> measures;
    static int measureNumber;
    static DeadSimpleTimer::deadTimer maximumMeasureFrequencySafetyTimer;

    PT_BEGIN(pt);

    LOG("Food load cell - measuring food amount. Supersampling: %d, trimCount: %d", supersamplingCount, trimCount);
    measures.clear();
    for (measureNumber = 0; measureNumber < supersamplingCount; measureNumber++)
    {
        PT_WAIT_UNTIL(pt, foodScale->is_ready());
        if (limitReadingToCalibrationRange) {
            measures.push_back(trimFoodScaleReadingToCalibrationLimit(readFoodScale()));
        } else {
            measures.push_back(readFoodScale());
        }
        DeadSimpleTimer::setMs(&maximumMeasureFrequencySafetyTimer, MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&maximumMeasureFrequencySafetyTimer));
    }
    LOG("Food load cell - measuring food amount done.");

    float avg = trimAndCalculateAverage(measures, trimCount);
    *result = (*foodScaleSpline)((double)avg);
    if (*result < 0 && limitReadingToCalibrationRange) {
        *result = 0.0; //triming to 0 is safe, as the trim amount should be monitored in trimFoodScaleReadingToCalibrationLimit.
    }
    LOG("Avg food reading: %f (scale units) --spline--> %f g", avg, *result);

    PT_END(pt);
}

int LoadCells::protothreadGetCatWeight(struct pt *pt, double *result)
{
    static std::vector<long> measures;
    static long catScaleMeasure1;
    static long catScaleMeasure2;
    static int measureNumber;
    static DeadSimpleTimer::deadTimer maximumMeasureFrequencySafetyTimer;

    PT_BEGIN(pt);

    LOG("Cat load cells - measuring 15 times...");
    measures.clear();
    for (measureNumber = 0; measureNumber < 15; measureNumber++)
    {
        PT_WAIT_UNTIL(pt, catScale1->is_ready());
        DeadSimpleTimer::setMs(&maximumMeasureFrequencySafetyTimer, MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&maximumMeasureFrequencySafetyTimer));
        catScaleMeasure1 = catScale1->read();
        
        PT_WAIT_UNTIL(pt, catScale2->is_ready());
        DeadSimpleTimer::setMs(&maximumMeasureFrequencySafetyTimer, MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
        PT_WAIT_UNTIL(pt, DeadSimpleTimer::expired(&maximumMeasureFrequencySafetyTimer));
        catScaleMeasure2 = catScale2->read();

        measures.push_back(trimCatScaleReadingToCalibrationLimit(catScaleMeasure1 + catScaleMeasure2));
    }
    LOG("Cat load cells - weighing cat done.");

    float avg = trimAndCalculateAverage(measures, 5);
    *result = (*catScaleSpline)((double)avg);
    if (*result < 0) {
        *result = 0.0; //triming to 0 is safe, as the trim amount should be monitored in trimCatScaleReadingToCalibrationLimit.
    }
    LOG("Avg cat reading: %f (scale units) --spline--> %f g", avg, *result);

    PT_END(pt);
}

void LoadCells::test_getCatWeight()
{
    static std::vector<long> measures;
    static long catScaleMeasure1;
    static long catScaleMeasure2;
    static int measureNumber;

    measures.clear();
    for (measureNumber = 0; measureNumber < 15; measureNumber++)
    {
        while(!catScale1->is_ready()) {
            usleep(100);
        }
        delay(MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
        catScaleMeasure1 = catScale1->read();

        while(!catScale2->is_ready()) {
            usleep(100);
        }
        delay(MINIMUM_GAPS_BETWEEN_READINGS_IN_MS);
        catScaleMeasure2 = catScale2->read();

        measures.push_back(catScaleMeasure1 + catScaleMeasure2);
    }

    float avg = trimAndCalculateAverage(measures, 5);
    double result = (*catScaleSpline)((double)avg);
    LOG("Avg cat reading: %f (scale units) --spline--> %f g", avg, result);
}