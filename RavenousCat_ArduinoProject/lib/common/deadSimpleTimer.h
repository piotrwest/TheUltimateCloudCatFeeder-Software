#pragma once

#include <Arduino.h>

namespace DeadSimpleTimer
{

struct deadTimer
{
    int64_t start, intervalUs;
};

bool expired(struct deadTimer *t);
void setUs(struct deadTimer *t, unsigned long uS);
void setMs(struct deadTimer *t, unsigned long mS);

}