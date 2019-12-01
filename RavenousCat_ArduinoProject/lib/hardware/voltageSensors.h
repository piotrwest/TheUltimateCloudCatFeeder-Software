#pragma once

class VoltageSensors
{

public:
    static void setup();
	static int protothread();

private:
	static struct pt pt;
};