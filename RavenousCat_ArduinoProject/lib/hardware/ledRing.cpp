
#include "pt.h"
#include "log4arduino.h"
#include "ledRing.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"

pt LedRing::pt;
CRGB LedRing::leds[LED_STRIP_LED_COUNT];
CurrentAnimation LedRing::currentAnimation = CurrentAnimation::Clear;
int LedRing::currentAnimationSpeed = 20;

void LedRing::setup()
{
    LOG("Entering LedRing setup");
    FastLED.addLeds<WS2811, UC_PIN_LED_STRIP, GRB>(leds, LED_STRIP_LED_COUNT);
    PT_INIT(&pt);
    LOG("Finished LedRing setup");
}

void LedRing::showFatalError(Error error)
{
    while (true)
    {
        for (int i = 0; i < error; i++)
        {
            clear();
            delay(1000);
            fill_solid(leds, LED_STRIP_LED_COUNT, CRGB::Red);
            FastLED.show();
            delay(1000);
        }
        delay(5000);
    }
}

void LedRing::showWaitingForNetworkTime(int progress)
{
    int scaledProgress = ((double)progress / (double)100) * 126;
    fill_solid(leds, LED_STRIP_LED_COUNT, CRGB(scaledProgress, scaledProgress, 0));
    FastLED.show();
}

void LedRing::clear()
{
    fill_solid(leds, LED_STRIP_LED_COUNT, CRGB::Black);
    FastLED.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
CRGB LedRing::wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void LedRing::setAnimation(CurrentAnimation animation, int speed)
{
    currentAnimation = animation;
    currentAnimationSpeed = speed;
}

/**
 * Fast implementation of: (int) std::ceil( (double)numerator / denominator );
 * Source: https://ideone.com/3OrviU
 **/
int idiv_ceil(int numerator, int denominator)
{
    return numerator / denominator + (((numerator < 0) ^ (denominator > 0)) && (numerator % denominator));
}

int LedRing::protothread()
{
    static uint16_t i;
    static uint16_t j;
    static uint16_t q;

    static DeadSimpleTimer::deadTimer animationTimer;
    static const int CLEAR_STEP_MS = 1000;
    static CRGB whiteColor = CRGB::White;

    PT_BEGIN(&pt);

    while (1)
    {
        if (currentAnimation == CurrentAnimation::Clear)
        {
            clear();
            DeadSimpleTimer::setMs(&animationTimer, CLEAR_STEP_MS);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
        }
        else if (currentAnimation == CurrentAnimation::PuttingFoodIntoBowl)
        {
            //Theatre-style crawling lights (theaterChase) from Adafruit_NeoPixel example
            for (j = 0; j < 10; j++)
            { //do 10 cycles of chasing
                for (q = 0; q < 5; q++)
                {
                    for (i = 0; i < LED_STRIP_LED_COUNT; i = i + 5)
                    {
                        leds[(i + q) % LED_STRIP_LED_COUNT] = whiteColor; //turn every 5th pixel on
                    }
                    FastLED.show();

                    DeadSimpleTimer::setMs(&animationTimer, currentAnimationSpeed * 2);
                    PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
                    if (currentAnimation != CurrentAnimation::PuttingFoodIntoBowl)
                    {
                        break;
                    }

                    for (int i = 0; i < LED_STRIP_LED_COUNT; i = i + 5)
                    {
                        leds[(i + q) % LED_STRIP_LED_COUNT] = CRGB::Black;
                    }
                }
                if (currentAnimation != CurrentAnimation::PuttingFoodIntoBowl)
                {
                    break;
                }
            }
        }
        else if (currentAnimation == CurrentAnimation::JustAboutToOpenLid)
        {
            static const CRGB headColor = CRGB::IndianRed;
            static const CRGB mainColor = CRGB::Green;
            static const CRGB fadeColor = blend(CRGB::Green, CRGB::Black, 100);  //tail
            static const CRGB fadeColor2 = blend(CRGB::Green, CRGB::Black, 200); //next to tail
            static const byte EyeSize = 3;
            static const byte FadeColorCount = 2;
            for (i = 0; i < LED_STRIP_LED_COUNT; i++)
            {
                fill_solid(leds, LED_STRIP_LED_COUNT, CRGB::Black);
                leds[(i + EyeSize + FadeColorCount) % LED_STRIP_LED_COUNT] = headColor;
                leds[(i) % LED_STRIP_LED_COUNT] = fadeColor2;
                leds[(i + 1) % LED_STRIP_LED_COUNT] = fadeColor;
                for (j = FadeColorCount; j < (EyeSize + FadeColorCount); j++)
                {
                    leds[(i + j) % LED_STRIP_LED_COUNT] = mainColor;
                }

                FastLED.show();
                DeadSimpleTimer::setMs(&animationTimer, currentAnimationSpeed);
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
                if (currentAnimation != CurrentAnimation::JustAboutToOpenLid)
                {
                    break;
                }
            }
        }
        else if (currentAnimation == CurrentAnimation::GivingFood)
        {
            //rainbowCycle from the Adafruit_NeoPixel example
            for (j = 0; j < 256 * 5; j++)
            { // 5 cycles of all colors on wheel
                for (i = 0; i < LED_STRIP_LED_COUNT; i++)
                {
                    leds[i] = wheel(((i * 256 / LED_STRIP_LED_COUNT) + j) & 255);
                }
                FastLED.show();
                DeadSimpleTimer::setMs(&animationTimer, currentAnimationSpeed / 2);
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
                if (currentAnimation != CurrentAnimation::GivingFood)
                {
                    break;
                }
            }
        }
        else if (currentAnimation == FoodDispensedWaitingAndChillingOut)
        {
            static const CRGB headColor = CRGB::IndianRed;
            static const CRGB mainColor = CRGB::Green;
            static const CRGB fadeColor = blend(CRGB::Green, CRGB::Black, 100);  //tail
            static const CRGB fadeColor2 = blend(CRGB::Green, CRGB::Black, 200); //next to tail
            static const byte EyeSize = 3;
            static const byte FadeColorCount = 2;
            for (i = 0; i < LED_STRIP_LED_COUNT; i++)
            {
                fill_solid(leds, LED_STRIP_LED_COUNT, CRGB::Black);
                leds[(i + EyeSize + FadeColorCount) % LED_STRIP_LED_COUNT] = headColor;
                if (idiv_ceil(currentAnimationSpeed, 3600) > 1) //if more than 2h left
                {
                    leds[(i + EyeSize + FadeColorCount) % LED_STRIP_LED_COUNT] = mainColor;
                }
                leds[(i) % LED_STRIP_LED_COUNT] = fadeColor2;
                leds[(i + 1) % LED_STRIP_LED_COUNT] = fadeColor;
                for (j = FadeColorCount; j < (EyeSize + FadeColorCount); j++)
                {
                    leds[(i + j) % LED_STRIP_LED_COUNT] = mainColor;
                }

                FastLED.show();
                if (idiv_ceil(currentAnimationSpeed, 3600) <= 1) //if 0..2h left
                {
                    DeadSimpleTimer::setMs(&animationTimer, currentAnimationSpeed);
                }
                else
                {
                    DeadSimpleTimer::setMs(&animationTimer, idiv_ceil(currentAnimationSpeed, 3600) * 1000);
                }
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
                if (currentAnimation != CurrentAnimation::FoodDispensedWaitingAndChillingOut)
                {
                    break;
                }
            }
        }
        else if (currentAnimation == WaitForCatArrival)
        {
            clear();
            for (i = 0; i < 7; i++)
            {
                leds[random(LED_STRIP_LED_COUNT)] = CRGB(random(255), random(255), random(255));
                FastLED.show();
                DeadSimpleTimer::setMs(&animationTimer, currentAnimationSpeed);
                PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
                if (currentAnimation != CurrentAnimation::WaitForCatArrival)
                {
                    break;
                }
            }
            DeadSimpleTimer::setMs(&animationTimer, currentAnimationSpeed);
            PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&animationTimer));
        }
    }
    PT_END(&pt);
}
