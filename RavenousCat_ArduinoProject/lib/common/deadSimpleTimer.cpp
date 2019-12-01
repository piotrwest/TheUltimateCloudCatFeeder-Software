#include "deadSimpleTimer.h"

namespace DeadSimpleTimer
{

inline int64_t clock_time()
{
    return esp_timer_get_time();
}

bool expired(struct deadTimer *t)
{
    return (clock_time() - t->start) >= (t->intervalUs);
}

void setUs(struct deadTimer *t, unsigned long uS)
{
    t->intervalUs = uS;
    t->start = clock_time();
}

void setMs(struct deadTimer *t, unsigned long mS)
{
    t->intervalUs = mS * 1000L;
    t->start = clock_time();
}

}