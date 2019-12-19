#include <iostream>
#include <unistd.h>
#include <math.h>
#include <memory>
#include "Errors.h"
#include "TimeNs.hpp"

#include "TestHelpers.hpp"

/********************************** MACROS ************************************/

/**
 * Verifies getTimeSinceInit
 *
 * @param kSecToSleep             Seconds to sleep
 * @param kNsToSleep              Nanoseconds to sleep
 */
#define VERIFY_GET_TIME_ELAPSED(kSecToSleep, kNsToSleep)                       \
{                                                                              \
    testTime.tv_sec = kSecToSleep;                                             \
    testTime.tv_nsec = kNsToSleep;                                             \
    CHECK_SUCCESS (test->getTimeSinceInit (elapsedOne));                       \
    nanosleep (&testTime, 0);                                                  \
    CHECK_SUCCESS (test->getTimeSinceInit (elapsedTwo));                       \
    timeDif = elapsedTwo - elapsedOne;                                         \
    approxDif = kSecToSleep * test->NS_IN_SECOND + kNsToSleep;                 \
    CHECK_IN_BOUND (timeDif, approxDif, NS_IN_MS);                             \
}

/*********************************** TESTS *************************************/

TEST_GROUP (TimeNs_Instances)
{

};

/**
 * Verify Singleton
 *
 * Checks to make sure only one instance of TimeNs is created
 */
TEST (TimeNs_Instances, TimeNs)
{
    // Get first instance.
    TimeNs* pTimeNs = nullptr;
    Error_t ret = TimeNs::getInstance (pTimeNs);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_TRUE (pTimeNs != nullptr);
    CHECK_TRUE (typeid (*pTimeNs) == typeid (TimeNs));

    // Get second instance.
    TimeNs* pTimeNsTwo = nullptr;
    Error_t retTwo = TimeNs::getInstance (pTimeNsTwo);
    CHECK_EQUAL (E_SUCCESS, retTwo);
    CHECK_TRUE (pTimeNsTwo != nullptr);
    CHECK_TRUE (typeid (*pTimeNsTwo) == typeid (TimeNs));

    // Check if equal
    CHECK_TRUE (pTimeNs == pTimeNsTwo);
};

TEST_GROUP (TimeNs_Elapsed)
{

};

/**
 * Verify getTimeSinceInit
 *
 * Checks difference between elapsed time and expected elapsed time
 */
TEST (TimeNs_Elapsed, getTimeSinceInit)
{
    const uint32_t NS_IN_MS = 1000000;
    uint64_t elapsedOne;
    uint64_t elapsedTwo;
    uint64_t timeDif;
    uint64_t approxDif;
    TimeNs* test;
    TimeNs::getInstance (test);
    struct timespec testTime;

    VERIFY_GET_TIME_ELAPSED (10, 0); // 10 sec

    VERIFY_GET_TIME_ELAPSED (3, 0);  // 3 sec

    VERIFY_GET_TIME_ELAPSED (0, 5000000); // 5 ms

    VERIFY_GET_TIME_ELAPSED (8, 0); // 8 sec

    VERIFY_GET_TIME_ELAPSED (0, 4000000); // 4 ms

    VERIFY_GET_TIME_ELAPSED (0, 4000000); // 4 ms

    VERIFY_GET_TIME_ELAPSED (0, 4000000); // 4 ms

    VERIFY_GET_TIME_ELAPSED (30, 0); // 30 sec

    VERIFY_GET_TIME_ELAPSED (3, 0); // 3 sec

    VERIFY_GET_TIME_ELAPSED (2, 0); // 2 sec

    VERIFY_GET_TIME_ELAPSED (2, 0); // 2 sec

    VERIFY_GET_TIME_ELAPSED (10, 0); // 10 sec
};
