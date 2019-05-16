/**
 * Singleton class to manage threads. To use this class call 
 * ThreadManager::getInstance to get the singleton. The first time this is 
 * called, ThreadManager::initKernelSchedulingEnvironment will be called to 
 * initialize the kernel scheduling environment.
 *    
 * NOTE: This object is intended to be called from only one thread and is not
 *       threadsafe.
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
 * There are 4 categories of threads we need to manage the priorities of:
 * 
 * 1. Hardware IRQ Threads: These kernel threads service the top half of hw 
 *    interrupts such as a key stroke or timer expiring. In RT Linux their
 *    default priority is 50, so all fsw threads must have a priority lower than
 *    this to make sure there is minimal latency in servicing these interrupts.
 *    Some of these hw IRQ threads do not do the full computation required to 
 *    service the interrupt, but instead schedule a software IRQ thread to 
 *    finish the processing (the bottom half of the interrupt servicing).
 * 
 * 2. Software IRQ Threads: These kernel threads are sometimes scheduled by the 
 *    hw IRQ threads to finish servicing a hardware interrupt. The sw IRQ 
 *    threads we care about are the ktimersoftd/N threads, where N is the core
 *    ID the thread runs on. These timer threads are critical for the periodic
 *    thread implementation in the ThreadManager and must not be starved. Their
 *    default scheduling policy is SCHED_FIFO (great), but their default 
 *    priorities are only 1. This means any fsw thread running at 1 or higher
 *    can starve these threads. To make sure we do not starve these threads, 
 *    the ThreadManager sets their priority to be 49 (below the hw IRQ 
 *    priority and giving plenty of room below to set various fsw thread 
 *    priorities).
 * 
 * 3. FSW Init Thread: This is the thread per node that initializes the other
 *    FSW threads (e.g. initializes a RIO's main loop thread & comms thread).
 *    This thread has a priority below the software IRQ threads and above the
 *    FSW App threads so that all of the app threads can be initialized without
 *    risk of the init thread being blocked.
 * 
 * 4. FSW App Threads: These threads are our real-time application threads used 
 *    to read sensors, process data, communicate over the network, and set 
 *    actuators. Their priorities are to be >= 1 and <= 47 so that the hw and sw
 *    timer IRQ threads have no risk of starvation.
 * 
 * After the ThreadManager is initialized, the following thread priorities are
 * set:
 * 
 *      Hardware IRQ Threads         = HW_IRQ_PRIORITY           = 50 (default)
 *      Software Timer IRQ Threads   = KTIMERSOFTD_PRIORITY      = 49
 *      FSW Init thread              = FSW_INIT_THREAD_PRIORITY  = 48
 *      Max new thread priority      = MAX_NEW_THREAD_PRIORITY   = 47
 *      Min new thread priority      = MIN_NEW_THREAD_PRIORITY   = 1 
 */

# ifndef THREAD_MANAGER_HPP
# define THREAD_MANAGER_HPP

#include <stdint.h>
#include <pthread.h>
#include <vector>

#include "Errors.h"

class ThreadManager final 
{
public:

    /**
     * Type of function expected by the thread creation methods.
     */
    typedef void ThreadFunc_t (void *);

    /**
     * Priority type used by thread creation methods. 
     */
    typedef uint8_t Priority_t;

    /**
     * CPU affinity possibilities for new threads.
     */
    enum class Affinity_t : uint8_t
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
     * in the pThreadManager param. Initializes kernel scheduling environment
     * the first time it is called.
     * 
     * @param   pThreadManager              Pointer to pointer to ThreadManager 
     *                                      object.     
     * 
     * @ret     E_SUCCESS                   Successfully pointed pThreadManager 
     *                                      to ThreadManager object.
     *          E_FAILED_TO_INIT_KERNEL_ENV Failed to initialize kernel 
     *                                      environment.
     */
    static Error_t getInstance (ThreadManager **ppThreadManager);

    /**
     * Create a thread with SCHED_FIFO scheduling policy. 
     * 
     * Note: Affinity is set after thread is created due to pthread API 
     *       limitations.
     * 
     * @param   thread                      pthread_t for new thread.
     * @param   pFunc                       Pointer to ThreadFunc_t function new 
     *                                      thread will execute.
     * @param   pArgs                       Pointer to arguments passed to new 
     *                                      thread's function.
     * @param   priority                    Priority to set new thread. Must be
     *                                      >= MIN_NEW_THREAD_PRIORITY and <= 
     *                                      MAX_NEW_THREAD_PRIORITY.
     * @param   cpuAffinity                 CPU affinity for new thread.
     * 
     * @ret     E_SUCCESS                   Successfully created new thread.
     *          E_INVALID_POINTER           Invalid pFunc param.
     *          E_INVALID_PRIORITY          Priority invalid.
     *          E_INVALID_AFFINITY          CPU affinity invalid.
     *          E_FAILED_TO_INIT_THREAD_ATR Failed to init thread attribute.
     *          E_FAILED_TO_SET_SCHED_POL   Failed to set scheduling policy.
     *          E_FAILED_TO_SET_PRIORITY    Failed to set thread priority.
     *          E_FAILED_TO_SET_SCHED_INH   Failed to set thread attribute
     *                                      inheritance.
     *          E_FAILED_TO_CREATE_THREAD   Failed to create thread.
     *          E_FAILED_TO_SET_AFFINITY    Failed to set thread affinity.
     */
    Error_t createThread (pthread_t &thread, ThreadFunc_t *pFunc, void *pArgs, 
                          Priority_t priority, Affinity_t cpuAffinity);

    /**
     * Block until specified thread returns.
     * 
     * @param   thread                      Thread to wait on.
     * @param   threadReturn                Reference to Error_t to fill with
     *                                      thread's return value.
     * 
     * @ret     E_SUCCESS                   Successfully waited on thread.
     *          E_FAILED_TO_WAIT_ON_THREAD  Failed to wait on thread.
     */
    Error_t waitForThread (pthread_t &thread, Error_t &threadReturn); 

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * PID's of the timer soft IRQ kernel threads. There is one per core, and
     * the sbRIO-96<2|3>7 has 2 cores. Hardcode the PID's since these do not
     * appear to change per system boot and getting the PID's dynamically is
     * tricky. These are verified on initialization using verifyProcess. 
     */
    static const uint8_t KTIMERSOFTD_0_PID;
    static const uint8_t KTIMERSOFTD_1_PID;

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * Hardcoded thread priorities.
     */
    static const uint8_t HW_IRQ_PRIORITY;
    static const uint8_t KTIMERSOFTD_PRIORITY;
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
     * @param   pid                     PID of the process to verify.
     * @param   expectedName            Expected name of process.
     * @param   verified                Will be set to true if process is 
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
    static Error_t verifyProcess (const uint8_t pid, 
                                  const std::string expectedName, 
                                  bool &verified);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF THREADMANAGER
     * 
     * Set the priority of a SCHED_FIFO process given its PID. This function is
     * used during ThreadManager initialization to update the priorities of
     * time-critical kernel threads.It is static so that it can be done before 
     * the ThreadManager object is constructed.
     * 
     * @param   pid                         PID of the process to verify.
     * @param   priority                    SCHED_FIFO priority to set process 
     *                                      to. Must be between 1-49 to not risk 
     *                                      starving the hw IRQ thread with 
     *                                      priority 50.
     * 
     * @ret     E_SUCCESS                   Priority successfully set.  
     *          E_INVALID_PRIORITY          Priority out of bounds.
     *          E_FAILED_TO_SET_PRIORITY    Failed to set priority.
     */
    static Error_t setProcessPriority (const uint8_t pid, 
                                       const uint8_t priority);

private:

    /** 
     * Whether or not the init function has been called. 
     */
    static bool initialized;

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
     * 2. Sets the ktimersoftd threads (one per core) to have a priority of 49. 
     *    This is below the priority of the hw IRQ threads (50) and gives plenty 
     *    of room below this to set priorities for fsw threads. 
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
};

#endif