#include <time.h>

#include "TestHelpers.hpp"

void TestHelpers::sleepMs (uint32_t ms)
{
    static const uint32_t MS_IN_S = 1000;
    static const uint32_t NS_IN_MS = 1000000;
    timespec timeToSleep;
    timeToSleep.tv_sec = ms / MS_IN_S;
    timeToSleep.tv_nsec = (ms % MS_IN_S) * NS_IN_MS;
    nanosleep (&timeToSleep, nullptr);
}
