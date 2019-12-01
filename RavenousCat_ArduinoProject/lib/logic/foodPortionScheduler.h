#pragma once

#include "inMemoryStore.h"

class FoodPortionScheduler
{

public:
	static void setup();
	static int protothread();

private:
	static struct pt pt;
};