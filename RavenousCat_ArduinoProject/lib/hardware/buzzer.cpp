
#include <Arduino.h>

#include "pt.h"
#include "pt-sem.h"

#include "deadSimpleTimer.h"
#include "buzzer.h"
#include "hardwareConsts.h"
#include "log4arduino.h"

//private static
pt_sem Buzzer::playFoodIntroSemaphore;
pt Buzzer::pt;

void Buzzer::setup()
{
    LOG("Entering buzzer setup");
    
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQUENCY, BUZZER_RESOLUTION_BITS);
    ledcAttachPin(UC_PIN_BUZZER, BUZZER_CHANNEL);

    PT_SEM_INIT(&playFoodIntroSemaphore, 0);
    PT_INIT(&pt);

    LOG("Finished buzzer setup");
}

void Buzzer::playFoodIntro()
{
    PT_SEM_SIGNAL(pt, &playFoodIntroSemaphore);
}

void Buzzer::longBeep()
{
    buzzStart(NOTE_E5, 1);
    delay(500);
    buzzStop();
    delay(100);
}

void Buzzer::shortBeep()
{
    buzzStart(NOTE_A7, 1);
    delay(50);
    buzzStop();
}

int Buzzer::protothread()
{
    //the variables used in protothreads have to be static
    static int thisNote;
    static DeadSimpleTimer::deadTimer buzzingTimer;
    static unsigned int noteDuration;
    static unsigned int pauseBetweenNotes;

    PT_BEGIN(&pt);

    while(1) {
        PT_SEM_WAIT(&pt, &playFoodIntroSemaphore);
        
        for (thisNote = 0; thisNote < themeSize; thisNote++)
        {
            // to calculate the note duration, take one second
            // divided by the note type.
            //e.g. quarter note = (1000*1000)/2 / 4, eighth note = 1 000 000 /8, etc.
            noteDuration = (1000*1000) / tempo[thisNote];

            buzzStart(melody[thisNote], 1);
            DeadSimpleTimer::setUs(&buzzingTimer, noteDuration);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&buzzingTimer));
            buzzStop();

            // to distinguish the notes, set a minimum time between them.
            // the note's duration + 30% seems to work well:
            pauseBetweenNotes = noteDuration * 1;
            DeadSimpleTimer::setUs(&buzzingTimer, pauseBetweenNotes);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&buzzingTimer));
        }
        thisNote = 0;
    }
    PT_END(&pt);
}

void Buzzer::buzzStart(long frequency, int duty)
{
    ledcWrite(BUZZER_CHANNEL, duty);
    ledcWriteTone(BUZZER_CHANNEL, frequency);
}

void Buzzer::buzzStop()
{
    ledcWrite(BUZZER_CHANNEL, 0);
}

const int Buzzer::melody[16] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7, 
    0, NOTE_C7, NOTE_E7, 0,
    NOTE_G7, 0, 0,  0,
    NOTE_G6, 0, 0, 0, 
/*
    NOTE_C7, 0, 0, NOTE_G6, 
    0, 0, NOTE_E6, 0, 
    0, NOTE_A6, 0, NOTE_B6, 
    0, NOTE_AS6, NOTE_A6, 0, 

    NOTE_G6, NOTE_E7, NOTE_G7, 
    NOTE_A7, 0, NOTE_F7, NOTE_G7, 
    0, NOTE_E7, 0,NOTE_C7, 
    NOTE_D7, NOTE_B6, 0, 0,

    NOTE_C7, 0, 0, NOTE_G6, 
    0, 0, NOTE_E6, 0, 
    0, NOTE_A6, 0, NOTE_B6, 
    0, NOTE_AS6, NOTE_A6, 0, 

    NOTE_G6, NOTE_E7, NOTE_G7, 
    NOTE_A7, 0, NOTE_F7, NOTE_G7, 
    0, NOTE_E7, 0,NOTE_C7, 
    NOTE_D7, NOTE_B6, 0, 0*/
};

const int Buzzer::tempo[16] = {
    12, 12, 12, 12, 
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12, 
/*
    12, 12, 12, 12,
    12, 12, 12, 12, 
    12, 12, 12, 12, 
    12, 12, 12, 12, 

    9, 9, 9,
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,

    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,

    9, 9, 9,
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,*/
};