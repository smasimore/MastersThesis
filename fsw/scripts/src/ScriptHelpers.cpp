#include <memory>
#include <stdint.h>
#include <time.h>

#include "ScriptHelpers.hpp"
#include "Time.hpp"

void ScriptHelpers::sleepNs (uint64_t kNs)
{
    timespec timeToSleep;
    timeToSleep.tv_sec = kNs / Time::NS_IN_S;
    timeToSleep.tv_nsec = kNs % Time::NS_IN_S;
    nanosleep (&timeToSleep, nullptr);
}

void ScriptHelpers::sleepMs (uint32_t kMs)
{
    ScriptHelpers::sleepNs (kMs * Time::NS_IN_MS);
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
    return timeNs * 1.0 / Time::NS_IN_S;
}
