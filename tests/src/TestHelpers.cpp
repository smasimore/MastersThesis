#include <time.h>

#include "TestHelpers.hpp"

void TestHelpers::sleepMs (uint32_t ms)
{
    static const uint32_t NS_IN_MS = 1000000;
    timespec timeToSleep;
    timeToSleep.tv_sec = 0;
    timeToSleep.tv_nsec = ms * NS_IN_MS;
    nanosleep (&timeToSleep, nullptr);
}
