/** All #include statements should come before the CppUTest include */
#include "Errors.h"
#include "Log.hpp"

#include "CppUTest/TestHarness.h"
 
TEST_GROUP (Logger)
{
};

/* Test constructing the log and logging 2 events. */
TEST (Logger, LogEvent)
{
    Error_t ret = E_SUCCESS;
    Log log = Log (ret);
    CHECK_TRUE (ret == E_SUCCESS);

    // Test invalid event.
    ret = log.logEvent ((Log::LogEvent_t) 123, 1);
    CHECK_TRUE (ret == E_INVALID_ENUM);

    // Test successful log.
    ret = log.logEvent (Log::LogEvent_t::THREAD_START, 1);
    CHECK_TRUE (ret == E_SUCCESS);

    // Test a second successful log.
    ret = log.logEvent (Log::LogEvent_t::THREAD_START, 2);
    CHECK_TRUE (ret == E_SUCCESS);
}

/* Test constructing two logs and verifying they are different and then the 
   same. */
TEST (Logger, VerifyLog)
{
    Error_t ret = E_SUCCESS;
    Log logOne = Log (ret);
    Log logTwo = Log (ret);

    // Log only to one log and verify.
    bool areEqual = false;
    logOne.logEvent (Log::LogEvent_t::THREAD_START, 1);
    logOne.logEvent (Log::LogEvent_t::THREAD_START, 2);
    ret = Log::verify (logOne, logTwo, areEqual);
    CHECK_TRUE (ret == E_SUCCESS);
    CHECK_TRUE (areEqual == false);

    // Log to other and verify again.
    logTwo.logEvent (Log::LogEvent_t::THREAD_START, 1);
    logTwo.logEvent (Log::LogEvent_t::THREAD_START, 2);
    ret = Log::verify (logOne, logTwo, areEqual);
    CHECK_TRUE (ret == E_SUCCESS);
    CHECK_TRUE (areEqual == true);
}
