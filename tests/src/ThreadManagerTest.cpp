#include <iostream>
#include <sched.h>

#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"
 
TEST_GROUP (ThreadManager)
{
};

/* Test the verifyProcess function. This test will fail if not run on 
   RT Linux. */
TEST (ThreadManager, verifyProcess)
{
    // Test using process watchdog/0. On RT Linux this is PID 14. 
    static const uint8_t SYSTEMD_PID = 14;
    static const std::string SYSTEMD_NAME = "watchdog/0";

    // Test incorrect name.
    bool verified = false;
    Error_t ret = ThreadManager::verifyProcess (SYSTEMD_PID, "not_my_name", 
                                                verified);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (false, verified);

    // Test correct name.
    ret = ThreadManager::verifyProcess (SYSTEMD_PID, SYSTEMD_NAME, 
                                        verified);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (true, verified);
}

TEST (ThreadManager, setProccessPriority)
{
    static const uint8_t PID = 4;
    static const uint8_t DEFAULT_PRIORITY = 1;
    static const uint8_t TARGET_PRIORITY = 49;

    // Set priority and verify.
    ThreadManager::setProcessPriority (PID, TARGET_PRIORITY); 
    struct sched_param schedParam;
    sched_getparam (PID, &schedParam);
    CHECK_EQUAL (schedParam.__sched_priority, TARGET_PRIORITY);
    
    // Set priority back to default and verify.
    ThreadManager::setProcessPriority (PID, DEFAULT_PRIORITY);
    sched_getparam (PID, &schedParam);
    CHECK_EQUAL (schedParam.__sched_priority, DEFAULT_PRIORITY);
}

/* Test ThreadManager singleton. This test will fail if not run on RT Linux. */
TEST (ThreadManager, ConstructTwo)
{
    // Get first instance.
    ThreadManager *pThreadManagerOne = nullptr;
    Error_t ret = ThreadManager::getInstance (&pThreadManagerOne);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_TRUE (pThreadManagerOne != nullptr);
    CHECK_TRUE (typeid (*pThreadManagerOne) == typeid (ThreadManager));

    // Get second instance.
    ThreadManager *pThreadManagerTwo = nullptr;
    ret = ThreadManager::getInstance (&pThreadManagerTwo);
    CHECK_TRUE (ret == E_SUCCESS);
    CHECK_TRUE (pThreadManagerTwo != nullptr);
    CHECK_TRUE (typeid (*pThreadManagerTwo) == typeid (ThreadManager));

    // Verify they point to the same ThreadManager.
    CHECK_TRUE (pThreadManagerOne == pThreadManagerTwo);
}

/* Test creating threads with invalid params. */
TEST (ThreadManager, CreateThreadInvalid)
{

}
