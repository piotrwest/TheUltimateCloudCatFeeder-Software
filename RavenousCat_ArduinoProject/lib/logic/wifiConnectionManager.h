#pragma once

#include "inMemoryStore.h"

class WifiConnectionManager
{

public:
	static void setup();
	static int protothread();

private:
	static struct pt pt;
};