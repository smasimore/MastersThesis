#include <iostream>
#include <unistd.h>
#include <math.h>
#include <memory>
#include "Errors.hpp"
#include "Time.hpp"

#include "TestHelpers.hpp"

/********************************** MACROS ************************************/

/**
 * Allowed +/- nanoseconds between measured time elapsed and time slept.
 */
#define ELAPSED_NS_ERROR_BOUND 200000

/**
 * Verifies getTimeNs
 *
 * @param kSecToSleep             Seconds to sleep
 * @param kNsToSleep              Nanoseconds to sleep
 */
#define VERIFY_GET_TIME_NS(kSecToSleep, kNsToSleep)                            \
{                                                                              \
    Time* pTime;                                                               \
    CHECK_SUCCESS (Time::getInstance (pTime));                                 \
    struct timespec pTimeSpec;                                                 \
    pTimeSpec.tv_sec = kSecToSleep;                                            \
    pTimeSpec.tv_nsec = kNsToSleep;                                            \
    Time::TimeNs_t timeNsOne;                                                  \
    Time::TimeNs_t timeNsTwo;                                                  \
    CHECK_SUCCESS (pTime->getTimeNs (timeNsOne));                              \
    nanosleep (&pTimeSpec, 0);                                                 \
    CHECK_SUCCESS (pTime->getTimeNs (timeNsTwo));                              \
    Time::TimeNs_t timeDif = timeNsTwo - timeNsOne;                            \
    Time::TimeNs_t approxDif = kSecToSleep * Time::NS_IN_S + kNsToSleep;       \
    CHECK_IN_BOUND (timeDif, approxDif, ELAPSED_NS_ERROR_BOUND);               \
}

/*********************************** TESTS *************************************/

TEST_GROUP (Time)
{

};

/**
 * Verify Singleton
 *
 * Checks to make sure only one instance of Time is created
 */
TEST (Time, Singleton)
{
    // Get first instance.
    Time* pTime = nullptr;
    CHECK_SUCCESS (Time::getInstance (pTime));
    CHECK_TRUE (pTime != nullptr);
    CHECK_TRUE (typeid (*pTime) == typeid (Time));

    // Get second instance.
    Time* pTimeTwo = nullptr;
    CHECK_SUCCESS (Time::getInstance (pTimeTwo));
    CHECK_TRUE (pTimeTwo != nullptr);
    CHECK_TRUE (typeid (*pTimeTwo) == typeid (Time));

    // Check if equal
    CHECK_TRUE (pTime == pTimeTwo);
};

/**
 * Verify getTimeNs
 *
 * Checks difference between elapsed time and expected elapsed time
 */
TEST (Time, GetTimeNs)
{
    VERIFY_GET_TIME_NS (10, 0); // 10 sec
    VERIFY_GET_TIME_NS (3, 0);  // 3 sec
    VERIFY_GET_TIME_NS (0, 5000000); // 5 ms
    VERIFY_GET_TIME_NS (8, 0); // 8 sec
    VERIFY_GET_TIME_NS (0, 4000000); // 4 ms
    VERIFY_GET_TIME_NS (0, 4000000); // 4 ms
    VERIFY_GET_TIME_NS (0, 4000000); // 4 ms
    VERIFY_GET_TIME_NS (30, 0); // 30 sec
    VERIFY_GET_TIME_NS (3, 0); // 3 sec
    VERIFY_GET_TIME_NS (2, 0); // 2 sec
    VERIFY_GET_TIME_NS (2, 0); // 2 sec
    VERIFY_GET_TIME_NS (10, 0); // 10 sec
};

/**
 * Verify underlying clock (CLOCK_REALTIME) is monotonic in its behavior.
 *
 * Checks that for 10k samples, time is always increasing.
 */
TEST (Time, Monotonic)
{
    const uint16_t NUM_SAMPLES = 10000;
    Time* pTime = nullptr;
    CHECK_SUCCESS (Time::getInstance (pTime));
    Time::TimeNs_t lastTimeNs = 0;
    Time::TimeNs_t currTimeNs = 0;

    for (uint16_t i = 0; i < NUM_SAMPLES; i++)
    {
        CHECK_SUCCESS (pTime->getTimeNs (currTimeNs));
        CHECK_TRUE (currTimeNs > lastTimeNs);
        lastTimeNs = currTimeNs;
    }
};
