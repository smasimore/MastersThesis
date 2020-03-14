#include <stdlib.h>
#include <time.h>
#include <limits>

#include "TimeNs.hpp"

Error_t TimeNs::getTimeSinceInit (TimeNs::TimeNs_t& kElapsedTimeRet)
{
    struct timespec monotonic;
     // Using CLOCK_MONOTONIC_RAW for time without NTP adjustments.
     // CLOCK_MONOTONIC (with NTP adjustments)
     // gives results with less standard deviation, but greater error overall
    uint32_t success = clock_gettime (CLOCK_REALTIME, &monotonic);
    if (success != 0)
    {
        return E_FAILED_TO_GET_TIME;
    }
    TimeNs::TimeNs_t currentTime = NS_IN_SECOND * monotonic.tv_sec
                                   + monotonic.tv_nsec;
    if (currentTime <= mTimeAtInit)
    {
        return E_OVERFLOW;
    }
    kElapsedTimeRet = currentTime - mTimeAtInit;
    return E_SUCCESS;
}

Error_t TimeNs::getInstance (TimeNs*& kPTimeNsRet)
{
    static Error_t checkInit = E_SUCCESS;
    static TimeNs instance = TimeNs (checkInit);
    kPTimeNsRet = &instance;
    return checkInit;
}

TimeNs::TimeNs (Error_t& kErrorRet)
{
    kErrorRet = E_SUCCESS;
    struct timespec monotonic;
    uint32_t success = clock_gettime (CLOCK_REALTIME, &monotonic);
    kErrorRet = success != 0 ? E_FAILED_TO_GET_TIME : E_SUCCESS;
    
    // If less than one year from overflow, return error
    int32_t maxAllowedSeconds = std::numeric_limits<int32_t>::max () - 
                                    SECONDS_AWAY_FROM_OVERFLOW_TO_INIT;
    if (monotonic.tv_sec > maxAllowedSeconds)
    {
        kErrorRet = E_OVERFLOW_IMMINENT;
        return;
    }

    mTimeAtInit =  NS_IN_SECOND * monotonic.tv_sec + monotonic.tv_nsec;
}

TimeNs::TimeNs (TimeNs const &) {}

TimeNs& TimeNs::operator= (TimeNs const &)
{
    return *this;
}

