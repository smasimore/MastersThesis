/* All #include statements should come before the CppUTest include */
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <iostream>
#include <limits>

#include "Errors.hpp"
#include "Log.hpp"
#include "ThreadManager.hpp"

#include "TestHelpers.hpp"

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
 *
 * @param  kRawArgs   Args.
 *
 * @ret    E_SUCCESS  Successfully executed thread.
 */
static void* funcLog (void* kRawArgs)
{
    // Log event to test log.
    struct ThreadFuncArgs *pArgs = (struct ThreadFuncArgs *) kRawArgs;
    uint8_t threadId = pArgs->threadId;
    pArgs->testLog->logEvent (Log::LogEvent_t::THREAD_START, threadId);

    return (void *) E_SUCCESS;
}

/**
 * Thread that logs its thread ID to the test log and then returns.
 *
 * @param  _kRawArgs  Unused.
 *
 * @ret    E_SUCCESS  Successfully executed thread.
 */
static void* funcNoArgs (void* _kRawArgs)
{
    return (void *) E_SUCCESS;
}

/**
 * Global flag to stop spinning thread.
 */
static bool gStopSpin = false;

/**
 * Thread that logs once and then spins until global flag is true.
 *
 * @param  kRawArgs   Args.
 */
static void* funcSpin (void* kRawArgs)
{
    // Log event to test log.
    struct ThreadFuncArgs *pArgs = (struct ThreadFuncArgs *) kRawArgs;
    uint8_t threadId = pArgs->threadId;
    CHECK_SUCCESS (pArgs->testLog->logEvent (Log::LogEvent_t::THREAD_START, 
                                             threadId));

    while (1) 
    {
        // Spin.
        if (gStopSpin == true)
        {
            break;
        }
    }

    // Thread should never get here.
    return (void *) E_SUCCESS;
}

/**
 * Thread that sleeps for 20ms to miss a 10ms deadline.
 *
 * @param  _kRawArgs  Unused.
 *
 * @ret    E_SUCCESS  Successfully executed thread.
 */
static void* funcMiss10MsDeadline (void* _kRawArgs)
{
    TestHelpers::sleepMs (20);
    return (void *) E_SUCCESS;
}

/**
 * Thread that returns error.
 *
 * @param  _kRawArgs          Unused.
 *
 * @ret    E_INVALID_POINTER  Intentional error.
 */
static void* funcError (void* _kRawArgs)
{
    return (void *) E_INVALID_POINTER;
}

/**
 * Handle a missed scheduler deadline. 
 *
 * @param  kError     Error to handle.
 *
 * @ret    E_SUCCESS     
 */
static Error_t periodicErrorHandler (Error_t kError)
{
    // Verify error expected.
    if (kError != E_MISSED_SCHEDULER_DEADLINE &&
        kError != E_INVALID_POINTER)
    {
        FAIL ("Unexpected periodic thread error");
    }

    return kError;
}

/********************************* TESTS **************************************/

TEST_GROUP (ThreadManagerInit)
{
    // Reset priorities of software irq threads.
    void teardown ()
    {
        // Priority of threads on boot.
        const uint8_t KSOFTIRQD_PRIORITY = 8;
        const uint8_t KTIMERSOFTD_PRIORITY = 1;

        CHECK_SUCCESS (ThreadManager::setKernelProcessPriority (
                                                 ThreadManager::KSOFTIRQD_0_PID,
                                                 KSOFTIRQD_PRIORITY));
        CHECK_SUCCESS (ThreadManager::setKernelProcessPriority (
                                             ThreadManager::KSOFTIRQD_1_PID,
                                             KSOFTIRQD_PRIORITY));
        CHECK_SUCCESS (ThreadManager::setKernelProcessPriority (
                                           ThreadManager::KTIMERSOFTD_0_PID,
                                           KTIMERSOFTD_PRIORITY));
        CHECK_SUCCESS (ThreadManager::setKernelProcessPriority (
                                           ThreadManager::KTIMERSOFTD_1_PID,
                                           KTIMERSOFTD_PRIORITY));
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
    CHECK_SUCCESS (ThreadManager::verifyProcess (SYSTEMD_PID, "not_my_name", 
                                                 verified));
    CHECK_EQUAL (false, verified);

    // Test correct name.
    CHECK_SUCCESS (ThreadManager::verifyProcess (SYSTEMD_PID, SYSTEMD_NAME, 
                                                 verified));
    CHECK_EQUAL (true, verified);
}

/* Test setProcessPriority function. */
TEST (ThreadManagerInit, SetProcessPriority)
{
    static const uint8_t DEFAULT_PRIORITY = 1;

    // Set priority and verify. Sleep for 1ms to allow priority change to 
    // propagate. 
    CHECK_SUCCESS (ThreadManager::setKernelProcessPriority (
                                    ThreadManager::KSOFTIRQD_0_PID,
                                    ThreadManager::SW_IRQ_PRIORITY));
    TestHelpers::sleepMs (1);
    struct sched_param schedParam;
    sched_getparam (ThreadManager::KSOFTIRQD_0_PID, &schedParam);
    CHECK_EQUAL (ThreadManager::SW_IRQ_PRIORITY, 
                 schedParam.__sched_priority);
    
    // Set priority back to default and verify. Sleep for 1ms to allow
    // priority change to propagate. 
    CHECK_SUCCESS (ThreadManager::setKernelProcessPriority (
                                                 ThreadManager::KSOFTIRQD_0_PID, 
                                                 DEFAULT_PRIORITY));
    TestHelpers::sleepMs (1);
    sched_getparam (ThreadManager::KSOFTIRQD_0_PID, &schedParam);
    CHECK_EQUAL (DEFAULT_PRIORITY, schedParam.__sched_priority);
}

/* Test passing in an invalid priority to setProcessPriority. */
TEST (ThreadManagerInit, SetProcessPriorityInvalidPri)
{
    CHECK_ERROR (ThreadManager::setKernelProcessPriority (
                                    ThreadManager::KSOFTIRQD_0_PID,
                                    sched_get_priority_max (SCHED_FIFO) + 1),
                 E_INVALID_PRIORITY)
    CHECK_ERROR (ThreadManager::setKernelProcessPriority (
                                    ThreadManager::KSOFTIRQD_0_PID,
                                    sched_get_priority_min (SCHED_FIFO) - 1),
                 E_INVALID_PRIORITY);
}

/* Test ThreadManager singleton. This test will fail if not run on RT Linux. */
TEST (ThreadManagerInit, ConstructTwo)
{
    // Get first instance.
    ThreadManager *pThreadManagerOne = nullptr;
    CHECK_SUCCESS (ThreadManager::getInstance (pThreadManagerOne));
    CHECK_TRUE (pThreadManagerOne != nullptr);
    CHECK_TRUE (typeid (*pThreadManagerOne) == typeid (ThreadManager));

    // Get second instance.
    ThreadManager *pThreadManagerTwo = nullptr;
    CHECK_SUCCESS (ThreadManager::getInstance (pThreadManagerTwo));
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
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcLog;
    struct ThreadFuncArgs args = {&testLog, 1};
    CHECK_ERROR (pThreadManager->createThread (
                                        thread1, nullptr, &args, sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL),
                 E_INVALID_POINTER);

    // Invalid priority. 
    CHECK_ERROR (pThreadManager->createThread (
                                    thread1, threadFunc, &args, 
                                    sizeof (args),
                                    ThreadManager::MAX_NEW_THREAD_PRIORITY + 1, 
                                    ThreadManager::Affinity_t::ALL),
                 E_INVALID_PRIORITY);
    CHECK_ERROR (pThreadManager->createThread (
                                    thread1, threadFunc, &args, 
                                    sizeof (args),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY - 1, 
                                    ThreadManager::Affinity_t::ALL),
                 E_INVALID_PRIORITY);

    // Invalid affinity. 
    CHECK_ERROR (pThreadManager->createThread (
                                        thread1, threadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::LAST),
                 E_INVALID_AFFINITY);

    // Non-zero args length with nullptr args.
    CHECK_ERROR (pThreadManager->createThread (
                                        thread1, threadFunc, nullptr, 1,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL),
                 E_INVALID_ARGS_LENGTH);

    // Expect both logs to be empty.
    VERIFY_LOGS;
}

/* Test creating and running a thread with no arguments. */
TEST (ThreadManagerCreate, CreateThreadNoArgsAndWait)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    // Create thread.
    pthread_t thread;
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcNoArgs;
    CHECK_SUCCESS (pThreadManager->createThread (
                                        thread, threadFunc, nullptr, 0,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL));

    // Wait for thread.
    Error_t threadReturn;
    CHECK_SUCCESS (pThreadManager->waitForThread (thread, threadReturn));

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
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcLog;
    ThreadManager::Affinity_t affinity = ThreadManager::Affinity_t::ALL;
    CHECK_SUCCESS (pThreadManager->createThread (
                                        thread1, threadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        affinity));

    // Wait for thread.
    Error_t threadReturn;
    CHECK_SUCCESS (pThreadManager->waitForThread (thread1, threadReturn));
    CHECK_EQUAL (E_SUCCESS, threadReturn);

    // Log that this thread returned from wait.
    testLog.logEvent (Log::LogEvent_t::THREAD_WAITED, 0);

    // Set expected log.
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1);
    expectedLog.logEvent (Log::LogEvent_t::THREAD_WAITED, 0);

    VERIFY_LOGS;
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

    ThreadManager::ThreadFunc_t threadFuncLog = 
        (ThreadManager::ThreadFunc_t) funcLog;

    // Create the three threads. Order doesn't matter since they all have CPU
    // affinity of 0 (same as cpputest thread), and cpputest thread has the
    // highest priority.
    CHECK_SUCCESS (pThreadManager->createThread (
                                        lowPriThread3, threadFuncLog, 
                                        &argsThread3, sizeof (argsThread3),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0));
    CHECK_SUCCESS (pThreadManager->createThread (
                                    medPriThread2, threadFuncLog, 
                                    &argsThread2, sizeof (argsThread2),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY + 1,
                                    ThreadManager::Affinity_t::CORE_0));
    CHECK_SUCCESS (pThreadManager->createThread (
                                        highPriThread1, threadFuncLog, 
                                        &argsThread1, sizeof (argsThread1),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::CORE_0));

    // Since newly created threads have lower priority than the cpputest thread,
    // neither should have run at this point. Verify the test log is empty.
    VERIFY_LOGS;

    // Wait for lowest pri thread.
    Error_t lowPriThreadReturn;
    CHECK_SUCCESS (pThreadManager->waitForThread (lowPriThread3, 
                                                  lowPriThreadReturn));
    CHECK_EQUAL (E_SUCCESS, lowPriThreadReturn);

    // Build expected log. 
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1);
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 2);
    expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 3);
    VERIFY_LOGS;

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
    ThreadManager::ThreadFunc_t threadFuncSpin = 
        (ThreadManager::ThreadFunc_t) funcSpin;
    CHECK_SUCCESS (pThreadManager->createThread (
                                        highPriThread1, threadFuncSpin, 
                                        &argsThread1, sizeof (argsThread1),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::CORE_0));

    // Create low pri thread that logs once then returns.
    ThreadManager::ThreadFunc_t threadFuncLog = 
        (ThreadManager::ThreadFunc_t) funcLog;
    CHECK_SUCCESS (pThreadManager->createThread (
                                        lowPriThread2, threadFuncLog, 
                                        &argsThread2, sizeof (argsThread2),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0));

    // Since newly created threads have lower priority than the cpputest thread,
    // neither should have run at this point. Verify the test log is empty.
    VERIFY_LOGS;

    // Block for 100ms to allow high pri thread to run.
    TestHelpers::sleepMs (100);

    // At this point only the high priority thread should have run.
    CHECK_SUCCESS (expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1));
    VERIFY_LOGS;

    // Stop the high pri thread and wait for the low pri thread to finish.
    gStopSpin = true;
    CHECK_SUCCESS (pThreadManager->waitForThread (lowPriThread2, ret));

    // Now low pri thread should have run as well.
    CHECK_SUCCESS (expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 2));
    VERIFY_LOGS;

    // Clean up threads.
    CHECK_SUCCESS (pThreadManager->waitForThread (highPriThread1, ret)); 
}
 
TEST_GROUP (ThreadManagerCreatePeriodic)
{
};

/* Test creating a periodic thread with invalid params. */
TEST (ThreadManagerCreatePeriodic, CreateThreadInvalidParams)
{
    const uint32_t THREAD_PERIOD_MS = 10;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Error handler.
    ThreadManager::ErrorHandler_t fErrorHandler = 
        (ThreadManager::ErrorHandler_t) periodicErrorHandler;

    // Invalid function.
    pthread_t thread1;
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcLog;
    struct ThreadFuncArgs args = {&testLog, 1};
    CHECK_ERROR (pThreadManager->createPeriodicThread (
                                        thread1, nullptr, &args, sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS, fErrorHandler),
                 E_INVALID_POINTER);

    // Invalid priority. 
    CHECK_ERROR (pThreadManager->createPeriodicThread (
                                        thread1, threadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY 
                                            + 1, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS, fErrorHandler),
                 E_INVALID_PRIORITY);
    CHECK_ERROR (pThreadManager->createPeriodicThread (
                                    thread1, threadFunc, &args, sizeof (args),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY - 1, 
                                    ThreadManager::Affinity_t::ALL,
                                    THREAD_PERIOD_MS, fErrorHandler),
                 E_INVALID_PRIORITY);

    // Invalid affinity. 
    CHECK_ERROR (pThreadManager->createPeriodicThread (
                                        thread1, threadFunc, &args, 
                                        sizeof (args),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::LAST,
                                        THREAD_PERIOD_MS, fErrorHandler),
                 E_INVALID_AFFINITY);

    // Non-zero args length with nullptr args.
    CHECK_ERROR (pThreadManager->createPeriodicThread (
                                        thread1, threadFunc, nullptr, 1, 
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS, fErrorHandler),
                 E_INVALID_ARGS_LENGTH);

    // Invalid error handler.
    CHECK_ERROR (pThreadManager->createPeriodicThread (
                                        thread1, threadFunc, &args, 
                                        sizeof (args), 
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS, nullptr),
                 E_INVALID_POINTER);

    // Expect both logs to be empty.
    VERIFY_LOGS;
}

/* Test creating and running a periodic thread with no arguments. */
TEST (ThreadManagerCreate, CreatePeriodicThreadNoArgs)
{
    const uint32_t THREAD_PERIOD_MS = 10;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Error handler.
    ThreadManager::ErrorHandler_t fErrorHandler = 
        (ThreadManager::ErrorHandler_t) periodicErrorHandler;

    // Create thread.
    pthread_t thread;
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcNoArgs;
    CHECK_SUCCESS (pThreadManager->createPeriodicThread (
                                        thread, threadFunc, nullptr, 0,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                                        ThreadManager::Affinity_t::ALL,
                                        THREAD_PERIOD_MS, fErrorHandler));

    // Clean up thread.
    pthread_cancel (thread);
    CHECK_SUCCESS (pThreadManager->waitForThread (thread, ret));

    // Expect the thread to return -1. Since ret is a uint32, this is the same 
    // as max uint32.
    CHECK_EQUAL (std::numeric_limits<uint32_t>::max (), ret);
}

/* Test creating and running a periodic thread with arguments. */
TEST (ThreadManagerCreatePeriodic, CreatePeriodicArgsThread)
{
    const uint32_t THREAD_PERIOD_MS = 10;
    const uint32_t TIME_TO_SLEEP_MS = 100;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Error handler.
    ThreadManager::ErrorHandler_t fErrorHandler = 
        (ThreadManager::ErrorHandler_t) periodicErrorHandler;

    pthread_t highPriPeriodicThread;

    struct ThreadFuncArgs argsThread = {&testLog, 1};

    // Create high pri, looping thread.
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcLog;
    CHECK_SUCCESS (pThreadManager->createPeriodicThread (
                                        highPriPeriodicThread, 
                                        threadFunc, &argsThread,
                                        sizeof (argsThread),
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0,
                                        THREAD_PERIOD_MS, fErrorHandler));

    // Block for 100ms to allow high pri thread to run 10 times.
    TestHelpers::sleepMs (TIME_TO_SLEEP_MS);

    // Build expected log.
    for (uint32_t i = 0; i < TIME_TO_SLEEP_MS; i += THREAD_PERIOD_MS)
    {
        CHECK_SUCCESS (expectedLog.logEvent (Log::LogEvent_t::THREAD_START, 1));
    }

    // Clean up thread. Do this before verifying in case the test fails.
    // If it fails, this test does not continue and the thread remains active
    // during subsequent tests. Return value is -1 because the thread was 
    // cancelled.
    pthread_cancel (highPriPeriodicThread);
    CHECK_SUCCESS (pThreadManager->waitForThread (highPriPeriodicThread, ret));

    // Expect the thread to return -1. Since ret is a uint32, this is the same 
    // as max uint32.
    CHECK_EQUAL (std::numeric_limits<uint32_t>::max (), ret);

    // Verify.
    VERIFY_LOGS;
}

/* Test creating and running a periodic thread that misses its deadline. */
TEST (ThreadManagerCreatePeriodic, CreatePeriodicDeadlineMiss)
{
    const uint32_t THREAD_PERIOD_MS = 10;
    const uint32_t TIME_TO_SLEEP_MS = 20;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Error handler.
    ThreadManager::ErrorHandler_t fErrorHandler = 
        (ThreadManager::ErrorHandler_t) periodicErrorHandler;

    // Create thread.
    pthread_t thread;
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcMiss10MsDeadline;
    CHECK_SUCCESS (pThreadManager->createPeriodicThread (
                                        thread, 
                                        threadFunc, nullptr, 0,
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0,
                                        THREAD_PERIOD_MS, fErrorHandler));

    // Block for 20ms to allow thread to run.
    TestHelpers::sleepMs (TIME_TO_SLEEP_MS);

    // Clean up thread.
    CHECK_SUCCESS (pThreadManager->waitForThread (thread, ret));

    // Expect deadline miss.
    CHECK_EQUAL (E_MISSED_SCHEDULER_DEADLINE, ret);
}

/* Test creating and running a periodic thread that returns an error. */
TEST (ThreadManagerCreatePeriodic, CreatePeriodicError)
{
    const uint32_t THREAD_PERIOD_MS = 10;
    const uint32_t TIME_TO_SLEEP_MS = 20;

    INIT_THREAD_MANAGER_AND_LOGS;

    // Error handler.
    ThreadManager::ErrorHandler_t fErrorHandler = 
        (ThreadManager::ErrorHandler_t) periodicErrorHandler;

    // Create thread.
    pthread_t thread;
    ThreadManager::ThreadFunc_t threadFunc = 
        (ThreadManager::ThreadFunc_t) funcError;
    CHECK_SUCCESS (pThreadManager->createPeriodicThread (
                                        thread, 
                                        threadFunc, nullptr, 0,
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0,
                                        THREAD_PERIOD_MS, fErrorHandler));

    // Block for 20ms to allow thread to run.
    TestHelpers::sleepMs (TIME_TO_SLEEP_MS);

    // Clean up thread.
    CHECK_SUCCESS (pThreadManager->waitForThread (thread, ret));

    // Expect loop error.
    CHECK_EQUAL (E_INVALID_POINTER, ret);
}

