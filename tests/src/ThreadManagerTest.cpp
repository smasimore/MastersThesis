#include <sched.h>
#include <signal.h>
#include <time.h>
#include <iostream>


#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"

/******************************** MACROS **************************************/

#define INIT_THREAD_MANAGER_AND_LOGS                                           \
    ThreadManager *pThreadManager = nullptr;                                   \
    ThreadManager::getInstance (&pThreadManager);                              \
    Error_t ret = E_SUCCESS;                                                   \
    Log expectedLog = Log (ret);                                               \
    Log testLog = Log (ret);                                                   \

#define VERIFY_LOGS(logsEqual)                                                 \
{                                                                              \
    logsEqual = false;                                                         \
    ret = Log::verify (expectedLog, testLog, logsEqual);                       \
    CHECK_EQUAL (E_SUCCESS, ret);                                              \
    CHECK_TRUE (logsEqual);                                                    \
}

/**************************** HELPER FUNCTIONS ********************************/
void sleepMs (uint32_t Ms)
{
    const uint32_t NS_IN_MS = 1000000;
    timespec timeToSleep;
    timeToSleep.tv_sec = 0;
    timeToSleep.tv_nsec = Ms * NS_IN_MS;
    nanosleep (&timeToSleep, nullptr);
}

/**************************** THREAD FUNCTIONS ********************************/

/**
 * Params to pass log and thread ID to thread functions.
 */
struct ThreadFuncArgs 
{
    Log *testLog;
    uint8_t threadId;
};

/**
 * Thread that logs its thread ID to the test log and then returns.
 */
void *threadFuncLog (void *rawArgs)
{
    // Log event to test log.
    struct ThreadFuncArgs *pArgs = (struct ThreadFuncArgs *) rawArgs;
    uint8_t threadId = pArgs->threadId;
    pArgs->testLog->logEvent (Log::LogEvent_t::THREAD_START, threadId);

    return (void *) E_SUCCESS;
}

/**
 * Thread that logs once and then spins.
 */
void *threadFuncSpin (void *rawArgs)
{
    // Set cancellation type to be asynchronous so cpputest thread can cancle
    // thread.
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

    // Log event to test log.
    struct ThreadFuncArgs *pArgs = (struct ThreadFuncArgs *) rawArgs;
    uint8_t threadId = pArgs->threadId;
    pArgs->testLog->logEvent (Log::LogEvent_t::THREAD_START, threadId);

    while (1) 
    {
        // Spin.
    }

    // Thread should never get here.
    return (void *) E_SUCCESS;
}

/********************************* TESTS **************************************/

TEST_GROUP (ThreadManagerInit)
{
    // Reset priorities of ktimersoftd threads.
    void teardown()
    {
        ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID,
                                    ThreadManager::KTIMERSOFTD_PRIORITY);
        ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_1_PID,
                                    ThreadManager::KTIMERSOFTD_PRIORITY);
    }
};

/* Test the verifyProcess function. This test will fail if not run on 
   RT Linux. */
TEST (ThreadManagerInit, VerifyProcess)
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
TEST (ThreadManagerInit, SetProcessPriority)
{
    static const uint8_t DEFAULT_PRIORITY = 1;

    // Set priority and verify.
    Error_t ret = ThreadManager::setProcessPriority (
                                    ThreadManager::KTIMERSOFTD_0_PID,
                                    ThreadManager::KTIMERSOFTD_PRIORITY); 
    CHECK_EQUAL (E_SUCCESS, ret);
    struct sched_param schedParam;
    sched_getparam (ThreadManager::KTIMERSOFTD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::KTIMERSOFTD_PRIORITY, 
                 schedParam.__sched_priority);
    
    // Set priority back to default and verify.
    ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID, 
                                       DEFAULT_PRIORITY);
    sched_getparam (ThreadManager::KTIMERSOFTD_0_PID, &schedParam);
    CHECK_EQUAL (DEFAULT_PRIORITY, schedParam.__sched_priority);
}

/* Test passing in an invalid priority to setProcessPriority. */
TEST (ThreadManagerInit, SetProcessPriorityInvalidPri)
{
    Error_t ret = ThreadManager::setProcessPriority (
                                    ThreadManager::KTIMERSOFTD_0_PID,
                                    ThreadManager::HW_IRQ_PRIORITY); 
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);
    ret = ThreadManager::setProcessPriority (
                                    ThreadManager::KTIMERSOFTD_0_PID,
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY - 1); 
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);
}

/* Test ThreadManager singleton. This test will fail if not run on RT Linux. */
TEST (ThreadManagerInit, ConstructTwo)
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
    CHECK_EQUAL (ThreadManager::KTIMERSOFTD_PRIORITY, 
                 schedParam.__sched_priority);
    sched_getparam (ThreadManager::KTIMERSOFTD_1_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::KTIMERSOFTD_PRIORITY, 
                 schedParam.__sched_priority);

    // Verify that the current thread sched policy and priority was set.
    int policy;
    pthread_getschedparam (pthread_self (), &policy, &schedParam);
    CHECK_EQUAL (SCHED_FIFO, policy);
    CHECK_EQUAL (ThreadManager::FSW_INIT_THREAD_PRIORITY, 
                 schedParam.__sched_priority);
}
 
TEST_GROUP (ThreadManagerCreate)
{
};

/* Test creating a thread with invalid params. */
TEST (ThreadManagerCreate, CreateThreadInvalidParams)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    // Invalid function. 
    pthread_t thread1;
    ThreadManager::ThreadFunc_t *pThreadFunc = (ThreadManager::ThreadFunc_t *) 
                                                    &threadFuncLog;
    struct ThreadFuncArgs args = {&testLog, 1};
    ret = pThreadManager->createThread (thread1, nullptr, &args, sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_INVALID_POINTER, ret);

    // Invalid priority. 
    ret = pThreadManager->createThread (thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY 
                                            + 1, 
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);
    ret = pThreadManager->createThread (thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY 
                                            - 1, 
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);

    // Invalid affinity. 
    ret = pThreadManager->createThread (thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::LAST);
    CHECK_EQUAL (E_INVALID_AFFINITY, ret);

    // Expect both logs to be empty.
    bool logsEqual;
    VERIFY_LOGS (logsEqual);
}

/* Test creating and running a single thread and then waiting for thread to 
   complete. */
TEST (ThreadManagerCreate, CreateThreadAndWait)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    // Create thread.
    pthread_t thread1;
    struct ThreadFuncArgs args = {&testLog, 1};
    ThreadManager::ThreadFunc_t *pThreadFunc = (ThreadManager::ThreadFunc_t *) 
                                                    &threadFuncLog;
    ThreadManager::Affinity_t affinity = ThreadManager::Affinity_t::ALL;
    ret = pThreadManager->createThread (thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        affinity);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Wait for thread.
    Error_t threadReturn;
    ret = pThreadManager->waitForThread (thread1, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (E_SUCCESS, threadReturn);

    // Log that this thread returned from wait.
    testLog.logEvent (Log::LogEvent_t::THREAD_WAITED, 0); 

    // Set expected log.
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1);
    expectedLog.logEvent (Log::LogEvent_t::THREAD_WAITED, 0);

    bool logsEqual;
    VERIFY_LOGS (logsEqual);
}

/* Test setting thread priorities by creating 3 threads with different 
   priorities. */ 
TEST (ThreadManagerCreate, Priorities)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    pthread_t highPriThread1;
    pthread_t medPriThread2;
    pthread_t lowPriThread3;

    struct ThreadFuncArgs argsThread1 = {&testLog, 1};
    struct ThreadFuncArgs argsThread2 = {&testLog, 2};
    struct ThreadFuncArgs argsThread3 = {&testLog, 3};

    ThreadManager::ThreadFunc_t *pThreadFuncLog = 
        (ThreadManager::ThreadFunc_t *) &threadFuncLog;

    // Create the three threads. Order doesn't matter since they all have CPU
    // affinity of 0 (same as cpputest thread), and cpputest thread has the
    // highest priority.
    ret = pThreadManager->createThread (lowPriThread3, pThreadFuncLog, 
                                        &argsThread3, sizeof (argsThread3),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pThreadManager->createThread (medPriThread2, pThreadFuncLog, 
                                        &argsThread2, sizeof (argsThread2),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY
                                            + 1,
                                        ThreadManager::Affinity_t::CORE_0);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pThreadManager->createThread (highPriThread1, pThreadFuncLog, 
                                        &argsThread1, sizeof (argsThread1),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::CORE_0);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Since newly created threads have lower priority than the cpputest thread,
    // neither should have run at this point. Verify the test log is empty.
    bool logsEqual;
    VERIFY_LOGS (logsEqual);

    // Wait for lowest pri thread.
    Error_t lowPriThreadReturn;
    ret = pThreadManager->waitForThread (lowPriThread3, lowPriThreadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (E_SUCCESS, lowPriThreadReturn);

    // Build expected log. 
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1);
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 2);
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 3);
    VERIFY_LOGS (logsEqual);

    // Clean up threads.
    pThreadManager->waitForThread (highPriThread1, ret); 
    pThreadManager->waitForThread (medPriThread2, ret); 
    pThreadManager->waitForThread (lowPriThread3, ret); 
}

/* Test affinity by creating a spinning thread with a high priority on CPU 0. 
   Then create a second thread with lower priority also on CPU 0. Neither thread 
   should be able to run until the cpputest thread blocks, since the cpputest
   thread has affinity set to CPU 0 as well. Thie second thread should not run 
   until the first thread has been cancelled. */
TEST (ThreadManagerCreate, AffinityCore0)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    pthread_t highPriThread1;
    pthread_t lowPriThread2;

    struct ThreadFuncArgs argsThread1 = {&testLog, 1};
    struct ThreadFuncArgs argsThread2 = {&testLog, 2};

    // Create high pri, looping thread.
    ThreadManager::ThreadFunc_t *pThreadFuncSpin = 
        (ThreadManager::ThreadFunc_t *) &threadFuncSpin;
    ret = pThreadManager->createThread (highPriThread1, pThreadFuncSpin, 
                                        &argsThread1, sizeof (argsThread1),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::CORE_0);

    // Create low pri thread that logs once then returns.
    ThreadManager::ThreadFunc_t *pThreadFuncLog = 
        (ThreadManager::ThreadFunc_t *) &threadFuncLog;
    ret = pThreadManager->createThread (lowPriThread2, pThreadFuncLog, 
                                        &argsThread2, sizeof (argsThread2),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Since newly created threads have lower priority than the cpputest thread,
    // neither should have run at this point. Verify the test log is empty.
    bool logsEqual;
    VERIFY_LOGS (logsEqual);

    // Block for 100ms to allow high pri thread to run.
    sleepMs (100);

    // At this point only the high priority thread should have run.
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1);
    VERIFY_LOGS (logsEqual);

    // Cancel the high pri thread and wait for the low pri thread to finish.
    pthread_cancel (highPriThread1);
    pThreadManager->waitForThread (lowPriThread2, ret);

    // Now low pri thread should have run as well.
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 2);
    VERIFY_LOGS (logsEqual);

    // Clean up threads.
    pThreadManager->waitForThread (highPriThread1, ret); 
    pThreadManager->waitForThread (lowPriThread2, ret); 
}
