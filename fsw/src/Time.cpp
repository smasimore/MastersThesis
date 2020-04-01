#include <stdlib.h>
#include <time.h>
#include <limits>

#include "Time.hpp"

Error_t Time::getTimeNs (Time::TimeNs_t& kTimeNsRet)
{
    struct timespec realtime;
    // Using CLOCK_REALTIME so that NTP adjustments made during calls to 
    // ClockSync are reflected.
    uint32_t success = clock_gettime (CLOCK_REALTIME, &realtime);
    if (success != 0)
    {
        return E_FAILED_TO_GET_TIME;
    }

    // Store current time.
    kTimeNsRet = NS_IN_S * realtime.tv_sec + realtime.tv_nsec;

    return E_SUCCESS;
}

Error_t Time::getInstance (Time*& kPTimeNsRet)
{
    static Error_t initRet = E_SUCCESS;
    static Time instance = Time (initRet);

    if (initRet != E_SUCCESS)
    {
        return initRet;
    }

    kPTimeNsRet = &instance;
    return E_SUCCESS;
}

Time::Time (Error_t& kErrorRet)
{
    kErrorRet = E_SUCCESS;
    struct timespec realtime;
    if (clock_gettime (CLOCK_REALTIME, &realtime) != 0)
    {
        kErrorRet = E_FAILED_TO_GET_TIME;
        return;
    }
    
    // If less than one year from overflow, return error.
    int32_t maxAllowedSeconds = std::numeric_limits<int32_t>::max () - 
                                    SECONDS_AWAY_FROM_OVERFLOW_TO_INIT;
    if (realtime.tv_sec > maxAllowedSeconds)
    {
        kErrorRet = E_OVERFLOW_IMMINENT;
        return;
    }

    mTimeAtInit =  NS_IN_S * realtime.tv_sec + realtime.tv_nsec;
}

Time::Time (Time const &) {}

Time& Time::operator= (Time const &)
{
    return *this;
}
