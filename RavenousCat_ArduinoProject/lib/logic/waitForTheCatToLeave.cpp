
#include "pt.h"

#include "waitForTheCatToLeave.h"
#include "log4arduino.h"
#include "loadCells.h"
#include "waitForCatArrival.h"

int WaitForTheCatToLeave::protothread(struct pt *pt)
{
    static struct pt PT_GetCatWeight;
    static double *catWeight = new double;

    PT_BEGIN(pt);
    LOG("Waiting for cat to leave...");
    PT_SPAWN(pt, &PT_GetCatWeight, LoadCells::protothreadGetCatWeight(&PT_GetCatWeight, catWeight));
    while (*catWeight > WaitForCatArrival::catPresentMinimumReading) 
    {
        PT_SPAWN(pt, &PT_GetCatWeight, LoadCells::protothreadGetCatWeight(&PT_GetCatWeight, catWeight));
    }
    LOG("Cat left.");
    PT_END(pt);
}