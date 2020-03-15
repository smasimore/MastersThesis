#include <memory>
#include <stdint.h>
#include <time.h>

#include "ScriptHelpers.hpp"
#include "Time.hpp"

void ScriptHelpers::sleepMs (uint32_t kMs)
{
    static const uint32_t MS_IN_S = 1000;
    static const uint32_t NS_IN_MS = 1000000;
    timespec timeToSleep;
    timeToSleep.tv_sec = kMs / MS_IN_S;
    timeToSleep.tv_nsec = (kMs % MS_IN_S) * NS_IN_MS;
    nanosleep (&timeToSleep, nullptr);
}

double ScriptHelpers::timeS ()
{
    // Initialize the Time module if this is the first call.
    static Time* pTime(nullptr);
    if (pTime == nullptr)
    {
        Error_t err = Time::getInstance (pTime);
        if (err != E_SUCCESS)
        {
            ERROR ("Error: ScriptHelpers::timeS failed to create timekeeper");
        }
    }

    // Get timestamp from Time.
    Time::TimeNs_t timeNs;
    Error_t err = pTime->getTimeNs (timeNs);
    if (err != E_SUCCESS)
    {
        ERROR ("Error: ScriptHelpers::timeS failed to generate timestamp");
    }

    // Compute elapsed seconds and return.
    return timeNs * 1.0 / Time::NS_IN_SECOND;
}
