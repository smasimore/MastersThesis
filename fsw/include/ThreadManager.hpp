/**
 * Singleton class to manage threads on a real-time target. The purpose of the
 * Thread Manager is provide tight control over the scheduling environment for 
 * binaries running on the flight computers (sbRIO's) running NI's Linux 
 * Real-Time. The scheduling on these targets needs to be extremely controlled 
 * and reliable to meet our 10ms loop deadlines. For other binaries, e.g. 
 * programs running on a ground computer, this level of control is likely 
 * unnecessary and should use the pthreads API directly. 
 *
 * To use this class call ThreadManager::getInstance to get the singleton. The 
 * first time this is called, ThreadManager::initKernelSchedulingEnvironment 
 * will be called to initialize the kernel scheduling environment. On thread 
 * creation, the arguments to the thread function are copied to the heap to 
 * protect against the caller's passed in arguments unintentionally going out of 
 * scope.
 *    
 * WARNINGS 
 *
 *     #1 This object is intended to be called from only one thread and is not
 *        threadsafe.
 *
 *     #2 The Thread Manager is strictly intended to be used on real-time 
 *        targets running NILRT (i.e. the sbRIO's). For creating threads on 
 *        other targets (e.g. the ground computer), use the pthreads API 
 *        directly. 
 * 
 * 
 *                    ---------- SCHEDULING ------------
 * SCHEDULING POLICY: 
 * 
 * For all fsw and time-critical kernel threads the SCHED_FIFO scheduling policy 
 * will be used. This policy schedules the highest priority thread until it has 
 * blocked or exited. 
 * 
 * 
 * SCHEDULING PRIORITIES:
 * 
 * For SCHED_FIFO threads, 99 is the highest priority and 1 is the lowest.
 * There are 5 categories of threads we need to manage the priorities of:
 * 
 * 1. Hardware IRQ Threads: These kernel threads service the top half of hw 
 *    interrupts such as a key stroke or timer expiring. In NILRT Linux their
 *    default priority is 15, so all fsw threads must have a priority lower than
 *    this to make sure there is minimal latency in servicing these interrupts.
 *    Some of these hw IRQ threads do not do the full computation required to 
 *    service the interrupt, but instead schedule a software IRQ thread to 
 *    finish the processing (the bottom half of the interrupt servicing). The
 *    hardware irq thread names all start with "irq".
 * 
 * 2. Software IRQ Threads: These kernel threads are sometimes scheduled by the 
 *    hw IRQ threads to finish servicing a hardware interrupt. The sw IRQ 
 *    threads we care about are the ksoftirqd/N and ktimersoftirqd/N threads, 
 *    where N is the core ID the thread runs on. These threads are critical 
 *    for the periodic thread implementation in the ThreadManager, which relies 
 *    on the hardware timer, and must not be starved. Additionally, these 
 *    threads are used to service network rx and tx. Their default scheduling 
 *    policy is SCHED_FIFO with a priority of 1 (timer) and 8 (soft). The 
 *    ThreadManager increases the priorities to 14 so that we have a larger 
 *    real-time priority range to give to FSW threads (which should run at 
 *    priorities lower than these threads).
 *
 *    Note: The hardware serviced by the ksoftird/n threads is listed in 
 *          kernel/softirq.c in the kernel source.
 * 
 * 3. FSW Init Thread: This is the thread per node that initializes the other
 *    FSW threads (e.g. initializes a RIO's main loop thread & comms thread).
 *    This thread has a priority below the software IRQ threads and above the
 *    FSW App threads so that all of the app threads can be initialized without
 *    risk of the init thread being blocked.
 * 
 * 4. FSW App Threads: These threads are our real-time application threads used 
 *    to read sensors, process data, communicate over the network, and set 
 *    actuators. Their priorities are to be >= 1 and <= 12 so that the hw and sw
 *    timer IRQ threads have no risk of starvation.
 *
 * 5. RCU Threads: Implements a synchronization method called read-copy update 
 *    to improve concurrency in the Linux kernel. Moving to a lower priority 
 *    than fsw threads had no measureable repurcussions and improves determinism 
 *    by ensuring the fsw threads can regain control of the CPU from these 
 *    threads.
 *                 
 * 
 * After the ThreadManager is initialized, the following thread priorities are
 * set:
 * 
 *      Hardware IRQ Threads         = HW_IRQ_PRIORITY           = 15 (default)
 *      Software IRQ Threads         = KSOFTIRQD_PRIORITY        = 14
 *      FSW Init thread              = FSW_INIT_THREAD_PRIORITY  = 13
 *      Max new thread priority      = MAX_NEW_THREAD_PRIORITY   = 12
 *      Min new thread priority      = MIN_NEW_THREAD_PRIORITY   = 2 
 *      RCU Threads                  = RCU_PRIORITY              = 1 (default) 
 */

#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <stdint.h>
#include <pthread.h>
#include <vector>
#include <string>

#include "Errors.hpp"

class ThreadManager final 
{
public:

    /**
     * Type of function expected by the thread creation methods.
     *
     * @param  void*  Void pointer to arguments struct.
     *
     * @ret    E_SUCCESS  Function executed successfully.
     *         [other]    Function error.
     */
    typedef void* (*ThreadFunc_t) (void*);

    /**
     * Type of function provided by user and called by periodic thread if there
     * is a missed scheduler deadline or error returned by user's periodic 
     * function. If this function returns anything but E_SUCCESS, the periodic
     * thread will exit.
     *
     * ------------------------- Errors to Handle ------------------------------
     *
     *      E_MISSED_SCHEDULER_DEADLINE     Thread completed after period
     *                                      elapsed.
     *      [other]                         Error returned from caller's 
     *                                      periodic function.
     *
     * -------------------------------------------------------------------------
     *
     * @param  Error_t    Error surfaced in periodic thread and to be handled by
     *                    provided error handler.
     *
     * @ret    E_SUCCESS  Error handled successfully. Indicates to periodic 
     *                    thread that it can proceed safely.
     *         [other]    Critical error. Periodic thread will exit.
     */
    typedef Error_t (*ErrorHandler_t) (Error_t);

    /**
     * Priority type used by thread creation methods. 
     */
    typedef uint8_t Priority_t;

    /**
     * CPU affinity possibilities for new threads.
     */
    enum Affinity_t : uint8_t
    {
        CORE_0,
        CORE_1,
        ALL,
        LAST
    };

    /**
     * Max priority for new threads.
     */
    static const uint8_t MAX_NEW_THREAD_PRIORITY;

    /**
     * Min priority for new threads.
     */
    static const uint8_t MIN_NEW_THREAD_PRIORITY;

    /**
     * Construct the ThreadManager if it does not already exist and return it
     * in the kPThreadManager param. Initializes kernel scheduling environment
     * the first time it is called.
     * 
     * @param   kPThreadManagerRet          Pointer to ThreadManager object.
     * 
     * @ret     E_SUCCESS                   Successfully pointed pThreadManager 
     *                                      to ThreadManager object.
     *          E_FAILED_TO_INIT_KERNEL_ENV Failed to initialize kernel 
     *                                      environment.
     */
    static Error_t getInstance (ThreadManager*& kPThreadManagerRet);

    /**
     * Create a thread with SCHED_FIFO scheduling policy. All created threads
     * must be waited on using waitForThread for proper cleanup.
     * 
     * Note: Affinity is set after thread is created due to pthread API 
     *       limitations.
     * 
     * @param   kThread                     pthread_t for new thread.
     * @param   kFunc                       Function new thread will execute.
     * @param   kPArgs                      Pointer to arguments passed to new 
     *                                      thread's function. These arguments
     *                                      will be copied to the heap, so the
     *                                      caller does not need to keep the 
     *                                      memory pArgs points to after this
     *                                      function returns successfully.
     * @param   kNumArgBytes                Number of bytes pArgs points to.
     * @param   kPriority                   Priority to set new thread. Must be
     *                                      >= MIN_NEW_THREAD_PRIORITY and <= 
     *                                      MAX_NEW_THREAD_PRIORITY.
     * @param   kCpuAffinity                CPU affinity for new thread.
     * 
     * @ret     E_SUCCESS                   Successfully created new thread.
     *          E_INVALID_POINTER           Invalid pFunc param.
     *          E_INVALID_PRIORITY          Priority invalid.
     *          E_INVALID_ARGS_LENGTH       Args length invalid.
     *          E_INVALID_AFFINITY          CPU affinity invalid.
     *          E_FAILED_TO_ALLOCATE_ARGS   Malloc failed for args.
     *          E_FAILED_TO_ALLOCATE_THREAD Malloc failed for thread struct.
     *          E_FAILED_TO_INIT_THREAD_ATR Failed to init thread attribute.
     *          E_FAILED_TO_SET_SCHED_POL   Failed to set scheduling policy.
     *          E_FAILED_TO_SET_PRIORITY    Failed to set thread priority.
     *          E_FAILED_TO_SET_SCHED_INH   Failed to set thread attribute
     *                                      inheritance.
     *          E_FAILED_TO_CREATE_THREAD   Failed to create thread.
     *          E_FAILED_TO_SET_AFFINITY    Failed to set thread affinity.
     */
    Error_t createThread (pthread_t& kThread, ThreadFunc_t kFunc, 
                          void* kPArgs, uint32_t kNumArgBytes, 
                          Priority_t kPriority, Affinity_t kCpuAffinity);

    /**
     * Create a periodic thread with SCHED_FIFO scheduling policy. All created 
     * threads must be waited on using waitForThread for proper cleanup.
     * 
     * Note: Affinity is set after thread is created due to pthread API 
     *       limitations.
     *
     * Warning: If there is a timer error in the periodic implementation, the 
     *          thread will exit. This is explicitly not tolerated, as a timer
     *          failure is a critical system error.
     * 
     * @param   kThread                     pthread_t for new thread.
     * @param   kFunc                       Function new thread will execute
     *                                      periodically.
     * @param   kPArgs                      Pointer to arguments passed to new 
     *                                      thread's function. These arguments
     *                                      will be copied to the heap, so the
     *                                      caller does not need to keep the 
     *                                      memory pArgs points to after this
     *                                      function returns successfully.
     * @param   kNumArgBytes                Number of bytes pArgs points to.
     * @param   kPriority                   Priority to set new thread. Must be
     *                                      >= MIN_NEW_THREAD_PRIORITY and <= 
     *                                      MAX_NEW_THREAD_PRIORITY.
     * @param   kCpuAffinity                CPU affinity for new thread.
     * @param   kPeriodMs                   Period of thread in ms.
     * @param   kErrorHandlerFunc           Callback function in case of error
     *                                      during execution of periodic thread
     *                                      or deadline miss.
     * 
     * @ret     E_SUCCESS                   Successfully created new thread.
     *          E_INVALID_POINTER           Invalid function pointer.
     *          E_INVALID_ARGS_LENGTH       Args length invalid.
     *          E_INVALID_PRIORITY          Priority invalid.
     *          E_INVALID_AFFINITY          CPU affinity invalid.
     *          E_FAILED_TO_ALLOCATE_ARGS   Malloc failed for args.
     *          E_FAILED_TO_ALLOCATE_THREAD Malloc failed for thread struct.
     *          E_FAILED_TO_INIT_THREAD_ATR Failed to init thread attribute.
     *          E_FAILED_TO_SET_SCHED_POL   Failed to set scheduling policy.
     *          E_FAILED_TO_SET_PRIORITY    Failed to set thread priority.
     *          E_FAILED_TO_SET_SCHED_INH   Failed to set thread attribute
     *                                      inheritance.
     *          E_FAILED_TO_CREATE_THREAD   Failed to create thread.
     *          E_FAILED_TO_SET_AFFINITY    Failed to set thread affinity.
     */
    Error_t createPeriodicThread (pthread_t& kThread, ThreadFunc_t kFunc, 
                                  void* kPArgs, uint32_t kNumArgBytes, 
                                  Priority_t kPriority, Affinity_t kCpuAffinity, 
                                  uint32_t kPeriodMs, 
                                  ErrorHandler_t kErrorHandlerFunc);

    /**
     * Block until specified thread returns. Waiting on an invalid thread has
     * undefined behavior (most likely a seg fault).
     * 
     * @param   kThread                     Thread to wait on.
     * @param   kThreadRet                  Reference to Error_t to fill with
     *                                      thread's return value.
     * 
     * @ret     E_SUCCESS                   Successfully waited on thread.
     *          E_FAILED_TO_WAIT_ON_THREAD  Failed to wait on thread.
     *          E_THREAD_NOT_FOUND          Failed to find thread and free 
     *                                      memory.
     */
    Error_t waitForThread (pthread_t& kThread, Error_t& kThreadRet); 

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * PID's of the software IRQ kernel threads. There is one per core, and
     * the sbRIO-96<2|3>7 has 2 cores. Hardcode the PID's since these do not
     * appear to change per system boot and getting the PID's dynamically is
     * tricky. These are verified on initialization using verifyProcess. 
     */
    static const uint8_t KSOFTIRQD_0_PID;
    static const uint8_t KSOFTIRQD_1_PID;
    static const uint8_t KTIMERSOFTD_0_PID;
    static const uint8_t KTIMERSOFTD_1_PID;

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * Hardcoded thread priorities.
     */
    static const uint8_t RCU_PRIORITY;
    static const uint8_t HW_IRQ_PRIORITY;
    static const uint8_t SW_IRQ_PRIORITY;
    static const uint8_t FSW_INIT_THREAD_PRIORITY;

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * Verify that the process identified by the provided PID has the expected
     * name. This is done by reading the /proc/<pid>/comm file. This function is
     * used during ThreadManager initialization to verify that the correct 
     * system threads are being modified. It is static so that it can be done
     * before the ThreadManager object is constructed.
     * 
     * @param   kPid                    PID of the process to verify.
     * @param   kExpectedName           Expected name of process.
     * @param   kVerifiedRet            Will be set to true if process is 
     *                                  successfully verified and false 
     *                                  otherwise.
     * 
     * @ret     E_SUCCESS               Verification successfully completed. 
     *                                  Does not mean the process passed 
     *                                  verification.  
     *          E_FAILED_TO_OPEN_FILE   Unable to open /proc file.
     *          E_FAILED_TO_READ_FILE   Unable to read /proc file.  
     *          E_FAILED_TO_CLOSE_FILE  Unable to close /proc file.
     */
    static Error_t verifyProcess (const uint8_t kPid,
                                  const std::string kExpectedName, 
                                  bool& kVerifiedRet);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * Set the priority of a SCHED_FIFO kernel process given its PID. This 
     * function is used during ThreadManager initialization to update the 
     * priorities of time-critical kernel threads. It is static so that it can 
     * be done before the ThreadManager object is constructed.
     * 
     * @param   kPid                        PID of the process to verify.
     * @param   kPriority                   SCHED_FIFO priority to set process 
     *                                      to. Must be between min and max
     *                                      SCHED_FIFO priorities.
     * 
     * @ret     E_SUCCESS                   Priority successfully set.  
     *          E_INVALID_PRIORITY          Priority out of bounds.
     *          E_FAILED_TO_SET_PRIORITY    Failed to set priority.
     */
    static Error_t setKernelProcessPriority (const uint8_t kPid, 
                                             const uint8_t kPriority);

private:

    /**
     * Struct to contain relevant info per thread.
     */
    struct Thread {
        pthread_t thread;   
        void *pArgs;
        struct Thread *next;
    };

    /**
     * List of active threads.
     */
    struct Thread* mThreadList;

    /**
     * Constructor.
     */        
    ThreadManager ();

    /**
     * Private copy constructor to enforce singleton.
     */
    ThreadManager (ThreadManager const &);

    /**
     * Private assignment operator to enforce singleton.
     */
    ThreadManager& operator=(ThreadManager const &);

    /**
     * Initialize the kernel scheduling environment:
     * 
     * 1. Sets the current thread (the thread that will be initializing the FSW 
     *    app) to have a scheduling policy of SCHED_FIFO and priority 1 below 
     *    the software timer IRQ and 1 above the max allowed new thread 
     *    priority. Sets current thread to have affinity to CPU 0 to provide 
     *    determinism in the FSW app startup and for testing purposes.
     * 
     * 2. Sets the software irq threads to have a priority below the hardware
     *    irq threads and above the max allowable fsw thread priorities.
     * 
     * @ret     E_SUCCESS                   Successfully initialized kernel 
     *                                      scheduling env. 
     *          E_FAILED_TO_SET_PRIORITY    Could not update the priority of the
     *                                      current thread or the kernel 
     *                                      threads.
     *          E_FAILED_TO_SET_AFFINITY    Failed to set current thread's CPU
     *                                      affinity. 
     *          E_FAILED_TO_VERIFY_PROCESS  Could not verify PID(s) were 
     *                                      kernel threads intended to have 
     *                                      their priorities updated.
     */
    static Error_t initKernelSchedulingEnvironment ();

    /**
     * Wrapper function for periodic thread implementation. Manages timer and
     * calls thread function each period. Does not return except on error.
     *
     * @param   kRawArgs                        Buffer with first 4 bytes 
     *                                          representing the thread's 
     *                                          period in ms, and second
     *                                          4 bytes representing the 
     *                                          function to call every period.
     *
     * @ret     E_FAILED_TO_CREATE_TIMERFD      Failed to create timer.
     *          E_FAILED_TO_ARM_TIMERFD         Failed to set and arm timer.
     *          E_FAILED_TO_GET_TIMER_FLAGS     Failed to get timer flags.
     *          E_FAILED_TO_SET_TIMER_FLAGS     Failed to set timer flags.
     *          E_FAILED_TO_READ_TIMERFD        Failed to read timer.
     *          E_DATA_VECTOR_WRITE             Failed to log deadline miss.
     *          E_MISSED_SCHEDULER_DEADLINE     Thread completed after period
     *                                          elapsed.
     *          E_TIMER_EXPIRED_MORE_THAN_ONCE  While blocked waiting for timer,
     *                                          timer unexpectedly expired more 
     *                                          than once.
     *          [other]                         Error returned from caller's 
     *                                          error handler.
     */
    static void* periodicWrapperFunc (void* kRawArgs);
};

#endif
