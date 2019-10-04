/* All #include statements should come before the CppUTest include */
#include <sched.h>
#include <signal.h>
#include <time.h>
#include <iostream>

#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"

#include "CppUTest/TestHarness.h"

/******************************** MACROS **************************************/

#define INIT_THREAD_MANAGER_AND_LOGS                                           \
    Error_t ret = E_SUCCESS;                                                   \
    ThreadManager *pThreadManager = nullptr;                                   \
    ret = ThreadManager::getInstance (&pThreadManager);                        \
    CHECK_EQUAL(E_SUCCESS, ret);                                               \
    Log expectedLog = Log (ret);                                               \
    Log testLog = Log (ret);


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
    static const uint32_t NS_IN_MS = 1000000;
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
 * Thread that logs its thread ID to the test log and then returns.
 */
void *threadFuncNoArgs (void *rawArgs)
{
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
    // Reset priorities of software irq threads.
    void teardown()
    {
		// Priority of threads on boot.
        const uint8_t KSOFTIRQD_PRIORITY = 8;
        const uint8_t KTIMERSOFTD_PRIORITY = 1;

        ThreadManager::setProcessPriority (ThreadManager::KSOFTIRQD_0_PID,
                                           KSOFTIRQD_PRIORITY);
        ThreadManager::setProcessPriority (ThreadManager::KSOFTIRQD_1_PID,
                                           KSOFTIRQD_PRIORITY);
        ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID,
                                           KTIMERSOFTD_PRIORITY);
        ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_1_PID,
                                           KTIMERSOFTD_PRIORITY);
    }
};

/* Test the verifyProcess function. */
TEST (ThreadManagerInit, VerifyProcess)
{
    // Test using process rcu_preempt. On RT Linux this is PID 7. 
    const uint8_t SYSTEMD_PID = 9;
    const std::string SYSTEMD_NAME = "rcu_preempt";

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
                                    ThreadManager::KSOFTIRQD_0_PID,
                                    ThreadManager::SW_IRQ_PRIORITY); 
    CHECK_EQUAL (E_SUCCESS, ret);
    struct sched_param schedParam;
    sched_getparam (ThreadManager::KSOFTIRQD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::SW_IRQ_PRIORITY, 
                 schedParam.__sched_priority);
    
    // Set priority back to default and verify.
    ThreadManager::setProcessPriority (ThreadManager::KSOFTIRQD_0_PID, 
                                       DEFAULT_PRIORITY);
    sched_getparam (ThreadManager::KSOFTIRQD_0_PID, &schedParam);
    CHECK_EQUAL (DEFAULT_PRIORITY, schedParam.__sched_priority);
}

/* Test passing in an invalid priority to setProcessPriority. */
TEST (ThreadManagerInit, SetProcessPriorityInvalidPri)
{
    Error_t ret = ThreadManager::setProcessPriority (
                                    ThreadManager::KSOFTIRQD_0_PID,
                                    ThreadManager::HW_IRQ_PRIORITY); 
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);
    ret = ThreadManager::setProcessPriority (
                                    ThreadManager::KSOFTIRQD_0_PID,
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

    // Verify software irq thread priorities set.
    struct sched_param schedParam;

    sched_getparam (ThreadManager::KSOFTIRQD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::SW_IRQ_PRIORITY, 
                 schedParam.__sched_priority);
    sched_getparam (ThreadManager::KSOFTIRQD_1_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::SW_IRQ_PRIORITY, 
                 schedParam.__sched_priority);
    sched_getparam (ThreadManager::KTIMERSOFTD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::SW_IRQ_PRIORITY, 
                 schedParam.__sched_priority);
    sched_getparam (ThreadManager::KTIMERSOFTD_1_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::SW_IRQ_PRIORITY, 
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

    // Non-zero args length with nullptr args.
    ret = pThreadManager->createThread (thread1, pThreadFunc, nullptr, 1,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_INVALID_ARGS_LENGTH, ret);

    // Expect both logs to be empty.
    bool logsEqual;
    VERIFY_LOGS (logsEqual);
}

/* Test creating and running a thread with no arguments. */
TEST (ThreadManagerCreate, CreateThreadNoArgsAndWait)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    // Create thread.
    pthread_t thread;
    ThreadManager::ThreadFunc_t *pThreadFunc = (ThreadManager::ThreadFunc_t *)
                                                    &threadFuncNoArgs;
    ret = pThreadManager->createThread (thread, pThreadFunc, nullptr, 0,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Wait for thread.
    Error_t threadReturn;
    ret = pThreadManager->waitForThread (thread, threadReturn);

    // Verify return value.
    CHECK_EQUAL (E_SUCCESS, threadReturn);
}

/* Test creating and running a thread with arguments. */
TEST (ThreadManagerCreate, CreateThreadArgsAndWait)
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
    CHECK_EQUAL (E_SUCCESS, ret);

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
 
TEST_GROUP (ThreadManagerCreatePeriodic)
{
};

/* Test creating a periodic thread with invalid params. */
TEST (ThreadManagerCreatePeriodic, CreateThreadInvalidParams)
{
    const uint32_t THREAD_PERIOD_MS = 10;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Invalid function.
    pthread_t thread1;
    ThreadManager::ThreadFunc_t *pThreadFunc = (ThreadManager::ThreadFunc_t *)
                                                    &threadFuncLog;
    struct ThreadFuncArgs args = {&testLog, 1};
    ret = pThreadManager->createPeriodicThread (thread1, nullptr, &args, sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS);
    CHECK_EQUAL (E_INVALID_POINTER, ret);

    // Invalid priority. 
    ret = pThreadManager->createPeriodicThread (
                                        thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY 
                                            + 1, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS);
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);
    ret = pThreadManager->createPeriodicThread (
                                        thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY 
                                            - 1, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS);
    CHECK_EQUAL (E_INVALID_PRIORITY, ret);

    // Invalid affinity. 
    ret = pThreadManager->createPeriodicThread (
                                        thread1, pThreadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::LAST,
                                        THREAD_PERIOD_MS);
    CHECK_EQUAL (E_INVALID_AFFINITY, ret);

    // Non-zero args length with nullptr args.
    ret = pThreadManager->createPeriodicThread (
                                        thread1, pThreadFunc, nullptr, 
                                        1, 
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS);
    CHECK_EQUAL (E_INVALID_ARGS_LENGTH, ret);

    // Expect both logs to be empty.
    bool logsEqual;
    VERIFY_LOGS (logsEqual);
}

/* Test creating and running a periodic thread with no arguments. */
TEST (ThreadManagerCreate, CreatePeriodicThreadNoArgs)
{
    const uint32_t THREAD_PERIOD_MS = 10;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Create thread.
    pthread_t thread;
    ThreadManager::ThreadFunc_t *pThreadFunc = (ThreadManager::ThreadFunc_t *)
                                                    &threadFuncNoArgs;
    ret = pThreadManager->createPeriodicThread (
                                        thread, pThreadFunc, nullptr, 0,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS);
   CHECK_EQUAL (E_SUCCESS, ret);

    // Clean up thread. 
    pthread_cancel (thread);
    pThreadManager->waitForThread (thread, ret);
    CHECK_EQUAL (-1, ret);
}

/* Test creating and running a periodic thread with arguments. */
TEST (ThreadManagerCreatePeriodic, CreatePeriodicArgsThread)
{
    const uint32_t THREAD_PERIOD_MS = 10;
    const uint32_t TIME_TO_SLEEP_MS = 100;

    INIT_THREAD_MANAGER_AND_LOGS;

    pthread_t highPriPeriodicThread;

    struct ThreadFuncArgs argsThread = {&testLog, 1};

    // Create high pri, looping thread.
    ThreadManager::ThreadFunc_t *pThreadFunc = (ThreadManager::ThreadFunc_t *)
                                                    &threadFuncLog;
    ret = pThreadManager->createPeriodicThread (
                                        highPriPeriodicThread, 
                                        pThreadFunc, &argsThread,
                                        sizeof (argsThread),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0,
                                        THREAD_PERIOD_MS);

    // Block for 100ms to allow high pri thread to run 10 times.
    sleepMs (TIME_TO_SLEEP_MS);

    // Build expected log.
    for (uint32_t i = 0; i < TIME_TO_SLEEP_MS; i += THREAD_PERIOD_MS)
    {
        expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1);
    }

    // Clean up thread. Do this before verifying in case the test fails.
    // If it fails, this test does not continue and the thread remains active
    // during subsequent tests. Return value is -1 because the thread was 
    // cancelled.
    pthread_cancel (highPriPeriodicThread);
    pThreadManager->waitForThread (highPriPeriodicThread, ret);
    CHECK_EQUAL (-1, ret);

    // Verify.
    bool logsEqual;
    VERIFY_LOGS (logsEqual);
}

