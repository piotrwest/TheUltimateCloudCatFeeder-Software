#pragma once

#include "buzzer_pitches.h"

class Buzzer
{

public:
    static void setup();
	static void playFoodIntro();
	static void shortBeep();
	static void longBeep();
	static int protothread();

private:
	Buzzer(); //pure static madness

	static struct pt_sem playFoodIntroSemaphore;
	static struct pt pt;
	
	static void buzzStart(long frequency, int duty);
	static void buzzStop();

	//Mario main theme melody
	static const int melody[16];
	//Mario main theme tempo
    static const int tempo[16];
	static const int themeSize = sizeof(melody) / sizeof(int);
};