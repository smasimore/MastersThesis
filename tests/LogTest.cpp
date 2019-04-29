#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Log.hpp"
 
TEST_GROUP (Logger)
{
};
 
TEST (Logger, LogEvent)
{
    Log log = Log ();
    Error_t ret = log.logEvent (Log::LogEvent_t::THREAD_START, 1);

    CHECK_TRUE (ret == E_SUCCESS);
}
