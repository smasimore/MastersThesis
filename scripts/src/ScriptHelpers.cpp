#include <memory>
#include <stdint.h>
#include <time.h>

#include "ScriptHelpers.hpp"
#include "TimeNs.hpp"

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
    // Initialize the TimeNs module if this is the first call.
    static TimeNs* pTimeNs(nullptr);
    if (pTimeNs == nullptr)
    {
        Error_t err = TimeNs::getInstance (pTimeNs);
        if (err != E_SUCCESS)
        {
            ERROR ("Error: ScriptHelpers::timeS failed to create timekeeper");
        }
    }

    // Get timestamp from TimeNs.
    TimeNs::TimeNs_t time;
    Error_t err = pTimeNs->getTimeSinceInit (time);
    if (err != E_SUCCESS)
    {
        ERROR ("Error: ScriptHelpers::timeS failed to generate timestamp");
    }

    // Compute elapsed seconds and return.
    static const uint64_t NS_IN_S = 1e9;
    return time * 1.0 / NS_IN_S;
}
