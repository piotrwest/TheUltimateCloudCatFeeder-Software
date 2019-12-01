#pragma once

class WaitForCatArrival
{
public:
	static int protothread(struct pt *pt, bool *catArrived, double catFoodStartAmount);
	static const long catArrivalTimeoutInMs = 1L*30L*60L*1000L; //30 min
	static constexpr double catPresentMinimumReading = 1000;
private:
};