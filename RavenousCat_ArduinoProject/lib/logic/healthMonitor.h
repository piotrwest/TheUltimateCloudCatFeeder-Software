#pragma once

class HealthMonitor
{
public:
    static void setup();
	static int protothread();
private:
	static struct pt pt;
};