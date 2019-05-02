#include <iostream>

#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"
 
TEST_GROUP (ThreadManager)
{
};

TEST (ThreadManager, verifyProcessSchedOther)
{
    // Test using process watchdog/0. On mainline Linux this is PID 13 and on
    // RT Linux this is PID 14. Handle both of these cases so that test does not
    // have different result depending on whether or not it is run on RT Linux. 
    const static uint8_t SYSTEMD_PID = 13;
    const static std::string SYSTEMD_NAME = "watchdog/0";
    const static uint8_t EXPECTED_DEFAULT_PRIORITY = 99;
    const static uint8_t EXPECTED_DEFAULT_SCHED_POLICY = SCHED_FIFO;

    bool verified = false;
    Error_t ret = ThreadManager::verifyProcess (SYSTEMD_PID, "not_my_name", 
                                                EXPECTED_DEFAULT_PRIORITY,
                                                EXPECTED_DEFAULT_SCHED_POLICY,
                                                verified);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (false, verified);

}

/* Test ThreadManager singleton. */
//TEST (ThreadManager, ConstructTwo)
//{
//    // Get first instance.
//    ThreadManager *pThreadManagerOne = nullptr;
//    Error_t ret = ThreadManager::getInstance (&pThreadManagerOne);
//    CHECK_EQUAL (E_SUCCESS, ret);
//    CHECK_TRUE (pThreadManagerOne != nullptr);
//    CHECK_TRUE (typeid (*pThreadManagerOne) == typeid (ThreadManager));
//
//    // Get second instance.
//    ThreadManager *pThreadManagerTwo = nullptr;
//    ret = ThreadManager::getInstance (&pThreadManagerTwo);
//    CHECK_TRUE (ret == E_SUCCESS);
//    CHECK_TRUE (pThreadManagerTwo != nullptr);
//    CHECK_TRUE (typeid (*pThreadManagerTwo) == typeid (ThreadManager));
//
//    // Verify they point to the same ThreadManager.
//    CHECK_TRUE (pThreadManagerOne == pThreadManagerTwo);
//}
//
///* Test creating threads with invalid params. */
//TEST (ThreadManager, CreateThreadInvalid)
//{
//
//}
