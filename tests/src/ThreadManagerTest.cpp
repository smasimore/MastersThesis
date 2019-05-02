#include <sched.h>

#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"
 
TEST_GROUP (ThreadManager)
{
    // Reset priorities of ktimersoftd threads.
    void teardown()
    {
        ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID,
                                    ThreadManager::KTIMERSOFTD_TARGET_PRIORITY);
        ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_1_PID,
                                    ThreadManager::KTIMERSOFTD_TARGET_PRIORITY);
    }
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

/* Test setProcessPriority function. */
TEST (ThreadManager, setProccessPriority)
{
    static const uint8_t DEFAULT_PRIORITY = 1;

    // Set priority and verify.
    Error_t ret = ThreadManager::setProcessPriority (
                                    ThreadManager::KTIMERSOFTD_0_PID,
                                    ThreadManager::KTIMERSOFTD_TARGET_PRIORITY); 
    CHECK_EQUAL (E_SUCCESS, ret);
    struct sched_param schedParam;
    sched_getparam (ThreadManager::KTIMERSOFTD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::KTIMERSOFTD_TARGET_PRIORITY, 
                 schedParam.__sched_priority);
    
    // Set priority back to default and verify.
    ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID, 
                                       DEFAULT_PRIORITY);
    sched_getparam (ThreadManager::KTIMERSOFTD_0_PID, &schedParam);
    CHECK_EQUAL (DEFAULT_PRIORITY, schedParam.__sched_priority);
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

    // Verify that ktimersoftd thread priorities set.
    struct sched_param schedParam;
    sched_getparam (ThreadManager::KTIMERSOFTD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::KTIMERSOFTD_TARGET_PRIORITY, 
                 schedParam.__sched_priority);
    sched_getparam (ThreadManager::KTIMERSOFTD_1_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::KTIMERSOFTD_TARGET_PRIORITY, 
                 schedParam.__sched_priority);
}
