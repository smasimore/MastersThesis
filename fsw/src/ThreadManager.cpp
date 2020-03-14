#include <dirent.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sched.h>
#include <cstring>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <unistd.h>

#include "ThreadManager.hpp"

/******************************* CONSTANTS ************************************/

const uint8_t ThreadManager::KSOFTIRQD_0_PID = 7;
const uint8_t ThreadManager::KSOFTIRQD_1_PID = 22;
const uint8_t ThreadManager::KTIMERSOFTD_0_PID = 8;
const uint8_t ThreadManager::KTIMERSOFTD_1_PID = 21;
const uint8_t ThreadManager::HW_IRQ_PRIORITY = 15;
const uint8_t ThreadManager::SW_IRQ_PRIORITY =
    ThreadManager::HW_IRQ_PRIORITY - 1;
const uint8_t ThreadManager::FSW_INIT_THREAD_PRIORITY = 
    ThreadManager::SW_IRQ_PRIORITY - 1;
const uint8_t ThreadManager::MAX_NEW_THREAD_PRIORITY = 
    ThreadManager::FSW_INIT_THREAD_PRIORITY - 1;
const uint8_t ThreadManager::MIN_NEW_THREAD_PRIORITY = 1;

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t ThreadManager::getInstance (ThreadManager **ppThreadManager)
{
    // 1) Initialize the kernel scheduling environment.
    static bool initialized = false;
    if (initialized == false)
    {
        Error_t ret = ThreadManager::initKernelSchedulingEnvironment ();
        if (ret != E_SUCCESS)
        {
            return E_FAILED_TO_INIT_KERNEL_ENV;
        }
        initialized = true;
    }

    // 2) Construct the ThreadManager instance and store it's pointer in the
    //    return param.
    static ThreadManager threadManagerInstance = ThreadManager ();
    *ppThreadManager = &threadManagerInstance;

    return E_SUCCESS;
}

Error_t ThreadManager::createThread (pthread_t &thread, 
                                     ThreadManager::ThreadFunc_t * pFunc, 
                                     void *pArgs, uint32_t numArgBytes,
                                     ThreadManager::Priority_t priority, 
                                     ThreadManager::Affinity_t cpuAffinity)
{
    // 1) Validate params. 
    if (pFunc == nullptr)
    {
        return E_INVALID_POINTER;
    } 
    else if (priority < ThreadManager::MIN_NEW_THREAD_PRIORITY || 
             priority > ThreadManager::MAX_NEW_THREAD_PRIORITY)
    {
        return E_INVALID_PRIORITY;
    }
    else if (cpuAffinity >= ThreadManager::Affinity_t::LAST)
    {
        return E_INVALID_AFFINITY;
    }
    else if (pArgs == nullptr && numArgBytes != 0)
    {
        return E_INVALID_ARGS_LENGTH;
    }

    // 2) Copy the passed in args to the heap.
    void *pArgsCopy = nullptr;
    if (numArgBytes > 0)
    {
        pArgsCopy = malloc (numArgBytes);
        if (pArgsCopy == nullptr)
        {
            return E_FAILED_TO_ALLOCATE_ARGS;
        }
        std::memcpy (pArgsCopy, pArgs, numArgBytes);
    }

    // 3) Initialize the thread attribute struct.
    pthread_attr_t attr;
    if (pthread_attr_init (&attr) != 0)
    {
        return E_FAILED_TO_INIT_THREAD_ATR;
    }

    // 4) Set thread scheduling policy to SCHED_FIFO. This means the thread will 
    //    only stop executing if a thread with higher priority is ready. Threads 
    //    with the same priority will not be scheduled until this thread is 
    //    blocked or has finished.
    if (pthread_attr_setschedpolicy (&attr, SCHED_FIFO) != 0)
    {
        return E_FAILED_TO_SET_SCHED_POL;
    }

    // 5) Set the thread priority.
    struct sched_param param;
    param.__sched_priority = priority;
    if (pthread_attr_setschedparam (&attr, &param) != 0)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }

    // 6) Set pthread to inherit sched params from attr instead of parent 
    //     thread.
    if (pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED) != 0)
    {
        return E_FAILED_TO_SET_SCHED_INH;
    }

    // 7) Create the thread.
    if (pthread_create (&thread, &attr, (void *(*)(void*)) pFunc, pArgsCopy) 
            != 0)
    {
        return E_FAILED_TO_CREATE_THREAD;
    }

    // 8) Set the CPU affinity of the new thread.
    cpu_set_t cpuset;
    CPU_ZERO (&cpuset);
    if (cpuAffinity == ThreadManager::Affinity_t::CORE_0)
    {
        CPU_SET (0, &cpuset);
    }
    else if (cpuAffinity == ThreadManager::Affinity_t::CORE_1)
    {
        CPU_SET (1, &cpuset);
    }
    else 
    {
        CPU_SET (0, &cpuset);
        CPU_SET (1, &cpuset);
    }

    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0)
    {
        return E_FAILED_TO_SET_AFFINITY;
    }

    // 9) Destroy thread attr struct. POSIX requires this function always 
    //    succeed, so don't check return. Additionally, if this does fail, the 
    //    consequence is a small amount of leaked memory, so if for some reason 
    //    it does fail, tolerate this error by ignoring it.
    pthread_attr_destroy (&attr);

    // 10) Allocate a thread struct to keep track of thread and insert it at the
    //     front of ThreadList.
    struct Thread *pNewThread = (struct Thread *) 
                                malloc (sizeof (struct Thread));
    if (pNewThread == nullptr)
    {
        return E_FAILED_TO_ALLOCATE_THREAD;
    }
    pNewThread->thread = thread;
    pNewThread->pArgs = pArgsCopy;
    pNewThread->next = this->ThreadList;
    this->ThreadList = pNewThread;

    return E_SUCCESS;
}

Error_t ThreadManager::createPeriodicThread (pthread_t &thread, 
                                             ThreadFunc_t *pFunc,
                                             void *pArgs,  uint32_t numArgBytes, 
                                             Priority_t priority, 
                                             Affinity_t cpuAffinity, 
                                             uint32_t periodMs)
{
    // 1) Validate pFunc and pArgs. The rest of the params will be validated 
    //    in the createThread call.
    if (pFunc == nullptr)
    {
        return E_INVALID_POINTER;
    } 
    else if (pArgs == nullptr && numArgBytes != 0)
    {
        return E_INVALID_ARGS_LENGTH;
    }

    // 2) Copy args to a buffer with an extra 8 bytes at the beginning to store
    //    periodMs and pFunc. These last 2 elements will be used by the periodic
    //    wrapper thread, while the original arguments will be passed down the
    //    pFunc.
    uint32_t argsBufferSize = numArgBytes + sizeof(periodMs) + sizeof(pFunc);
    uint8_t argsBuffer[argsBufferSize];

    // 3) Store periodMs in the buffer.
    argsBuffer[3] = (periodMs >> 24) & 0xFF;
    argsBuffer[2] = (periodMs >> 16) & 0xFF;
    argsBuffer[1] = (periodMs >> 8) & 0xFF;
    argsBuffer[0] = periodMs & 0xFF;

    // 4) Store pFunc in the buffer.
    argsBuffer[7] = ((uint32_t) pFunc >> 24) & 0xFF;
    argsBuffer[6] = ((uint32_t) pFunc >> 16) & 0xFF;
    argsBuffer[5] = ((uint32_t) pFunc >> 8) & 0xFF;
    argsBuffer[4] = (uint32_t) pFunc & 0xFF;

    // 5) Copy the pFunc args to the rest of the buffer.
    if (numArgBytes > 0)
    {
        std::memcpy (argsBuffer + 8, pArgs, numArgBytes);
    }
   
    // 6) Create wrapper thread, which will call pFunc.
    ThreadManager::ThreadFunc_t *pPeriodicWrapperFunc = 
        (ThreadManager::ThreadFunc_t *) &ThreadManager::periodicWrapperFunc;
    Error_t ret = this->createThread (thread, pPeriodicWrapperFunc, argsBuffer,
                                      argsBufferSize, priority, cpuAffinity);

    return ret;
}


Error_t ThreadManager::waitForThread (pthread_t &thread, Error_t &threadReturn)
{
    void * threadReturnVoid = nullptr;
    if (pthread_join (thread, &threadReturnVoid) != 0)
    {
        return E_FAILED_TO_WAIT_ON_THREAD;
    }

    threadReturn = static_cast<Error_t> (
                    reinterpret_cast<uint64_t> (threadReturnVoid));

    // Find thread struct and free allocated memory.
    struct ThreadManager::Thread *curr = this->ThreadList;
    struct ThreadManager::Thread *prev = nullptr;
    bool foundInList = false;
    while (curr != nullptr)
    {
        foundInList = true;

        // If thread found, remove from list and free memory.
        if (pthread_equal (curr->thread, thread) != 0)
        {
            // Handle if curr is first thread in the list.
            if (prev == nullptr)
            {
                this->ThreadList = curr->next;
            }

            // Handle if curr is not first thread in the list.
            else
            {
                prev->next = curr->next;
            }

            // Free args and thread struct. 
            free (curr->pArgs);
            free (curr);

            // Thread found and freed, break out of loop.
            break;
        }

        // Otherwise, go to next thread.
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }

    if (foundInList == false)
    {
        return E_THREAD_NOT_FOUND;
    }

    return E_SUCCESS;
} 

Error_t ThreadManager::verifyProcess (const uint8_t pid, 
                                      const std::string expectedName, 
                                      bool &verified)
{
    verified = true;

    // 1) Build path to file 
    const std::string PROC_DIR = "/proc/";
    const std::string PROC_COMM_DIR = "/comm";
    std::string procNameFilePath = PROC_DIR + std::to_string (pid) + 
                                   PROC_COMM_DIR;
    std::fstream procNameFile;

    // 2) Open file.
    procNameFile.open (procNameFilePath, std::ios::in);
    if ((procNameFile.rdstate () & std::ifstream::failbit) != 0)
    {
        return E_FAILED_TO_OPEN_FILE;
    }

    // 3) Read the first line of the file.
    const static uint8_t MAX_NAME_LEN = 32;
    char actualName[MAX_NAME_LEN];
    procNameFile >> actualName;
    if ((procNameFile.rdstate () & (std::ifstream::eofbit | 
         std::ifstream::badbit | std::ifstream::failbit)) != 0)
    {
        return E_FAILED_TO_READ_FILE;
    }

    // 4) Close file.
    procNameFile.close();
    if ((procNameFile.rdstate () & std::ifstream::failbit) != 0)
    {
        return E_FAILED_TO_CLOSE_FILE;
    }

    // 5) Compare actual to expected.
    if (strcmp (actualName, expectedName.c_str ()) != 0)
    {
        verified = false;
    }

    return E_SUCCESS;
}

Error_t ThreadManager::setProcessPriority (const uint8_t pid, 
                                           const uint8_t priority)
{

    // Only allow priorities below hw IRQ thread priority and above min
    // SCHED_FIFO priority.
    if (priority < ThreadManager::MIN_NEW_THREAD_PRIORITY || 
        priority >= ThreadManager::HW_IRQ_PRIORITY)
    {
        return E_INVALID_PRIORITY;
    }
    struct sched_param schedParam;
    schedParam.__sched_priority = priority;
    if (sched_setparam (pid, &schedParam) != 0)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }

    return E_SUCCESS;
}

/**************************** PRIVATE FUNCTIONS *******************************/

ThreadManager::ThreadManager () 
{
    this->ThreadList = nullptr;
}

ThreadManager::ThreadManager (ThreadManager const &) {}

ThreadManager& ThreadManager::operator= (ThreadManager const &) 
{
    return *this;
};

Error_t ThreadManager::initKernelSchedulingEnvironment () 
{
    // 1) Set the current thread to have FSW_INIT_THREAD_PRIORITY. This is 1 
    //    above the maximum priority allowed for new threads. This is done so 
    //    that the initial FSW thread can create all threads before being 
    //    descheduled.
    pthread_t currentThread = pthread_self();
    struct sched_param schedParams;
    schedParams.sched_priority = ThreadManager::FSW_INIT_THREAD_PRIORITY;
    if (pthread_setschedparam (currentThread, SCHED_FIFO, &schedParams) != 0)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }
    
    // 2) Set the current thread to have CPU affinity to CPU 0. This makes 
    //    the startup of the FSW application more deterministic by limiting the
    //    init thread to a specific CPU. This is also important in maintaining
    //    reproducibility/determinism of the ThreadManager tests.
    cpu_set_t cpuset;
    CPU_ZERO (&cpuset);
    CPU_SET (0, &cpuset);
    if (pthread_setaffinity_np(currentThread, sizeof(cpu_set_t), &cpuset) != 0)
    {
        return E_FAILED_TO_SET_AFFINITY;
    }

    // 3) Verify that the hardcoded sw irq PID's map to the correct processes.
    const std::string EXPECTED_KSOFTIRQD_0_NAME = "ksoftirqd/0";
    const std::string EXPECTED_KSOFTIRQD_1_NAME = "ksoftirqd/1";
    const std::string EXPECTED_KTIMERSOFTD_0_NAME = "ktimersoftd/0";
    const std::string EXPECTED_KTIMERSOFTD_1_NAME = "ktimersoftd/1";

    bool verified = false;
    Error_t ret = ThreadManager::verifyProcess (KSOFTIRQD_0_PID, 
                                                EXPECTED_KSOFTIRQD_0_NAME, 
                                                verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    ret = ThreadManager::verifyProcess (KSOFTIRQD_1_PID, 
                                        EXPECTED_KSOFTIRQD_1_NAME, 
                                        verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    ret = ThreadManager::verifyProcess (KTIMERSOFTD_0_PID, 
                                        EXPECTED_KTIMERSOFTD_0_NAME, 
                                        verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    ret = ThreadManager::verifyProcess (KTIMERSOFTD_1_PID, 
                                        EXPECTED_KTIMERSOFTD_1_NAME, 
                                        verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    // 4) Set the priorities of the sw irq threads.
    ret = ThreadManager::setProcessPriority (ThreadManager::KSOFTIRQD_0_PID, 
                                             ThreadManager::SW_IRQ_PRIORITY);
    if (ret != E_SUCCESS)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }
    ret = ThreadManager::setProcessPriority (ThreadManager::KSOFTIRQD_1_PID, 
                                             ThreadManager::SW_IRQ_PRIORITY);
    if (ret != E_SUCCESS)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }
    ret = ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID, 
                                             ThreadManager::SW_IRQ_PRIORITY);
    if (ret != E_SUCCESS)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }
    ret = ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_1_PID, 
                                             ThreadManager::SW_IRQ_PRIORITY);
    if (ret != E_SUCCESS)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }

    return E_SUCCESS;
}

void *ThreadManager::periodicWrapperFunc (void *rawArgs)
{
    // 1) Get period and function to call every period from first 8 bytes in 
    //    argsBuffer.
    uint32_t *argsBuffer = (uint32_t *) rawArgs;
    uint32_t periodMs = argsBuffer[0];
    ThreadManager::ThreadFunc_t *pFunc = (ThreadManager::ThreadFunc_t *) argsBuffer[1];

    // 2) Set up periodic timer.
    static const uint32_t NUM_MS_IN_S = 1000;
    static const uint32_t NUM_NS_IN_MS = 1000000;

    // 2a) Create the timer. Make nonblocking so can check if missed deadline.
    int32_t timerFd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd < 0)
    {
        return (void *) E_FAILED_TO_CREATE_TIMERFD;
    }

    // 2b) Make the timer periodic.
    struct itimerspec itval;
    uint32_t sec = periodMs / NUM_MS_IN_S;
    uint32_t ns = (periodMs - (sec * NUM_MS_IN_S)) * NUM_NS_IN_MS;
    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;
    if (timerfd_settime (timerFd, 0, &itval, NULL) < 0)
    {
        return (void *) E_FAILED_TO_ARM_TIMERFD;
    }

    // 3) Enter periodic loop.
    while (1)
    {
        // 3a) Call pFunc with the args buffer starting after the period and
        //     pFunc elements.
        Error_t ret = static_cast<Error_t> (reinterpret_cast<uint64_t> (
                                                        pFunc(&argsBuffer[2])));
        if (ret != E_SUCCESS)
        {
            return (void *) ret;
        }

        // 3b) Set fd as non-blocking to be able to check for deadline miss.
        int32_t flags = fcntl (timerFd, F_GETFL, 0);
        if (flags == -1)
        {
            return (void *) E_FAILED_TO_GET_TIMER_FLAGS;
        }

        flags |= O_NONBLOCK;
        if (fcntl (timerFd, F_SETFL, flags) == -1)
        {
            return (void *) E_FAILED_TO_SET_TIMER_FLAGS;
        }

        // 3c) Check for deadline miss.
        int32_t readRet = 0;
        uint64_t ticksSinceLastRead = 0;
        readRet = read (timerFd, &ticksSinceLastRead, sizeof (ticksSinceLastRead));
        if (readRet < 0 && errno != EAGAIN)
        {
            return (void *) E_FAILED_TO_READ_TIMERFD;
        }
        else if (ticksSinceLastRead > 0)
        {
            return (void *) E_MISSED_SCHEDULER_DEADLINE;
        }

        // 3d) Set fd as blocking so can block until timer elapses.
        flags &= ~O_NONBLOCK;
        if (fcntl (timerFd, F_SETFL, flags) == -1)
        {
            return (void *) E_FAILED_TO_SET_TIMER_FLAGS;
        }

        // 3e) Block until timer elapses and make sure timer only expired once.
        readRet = read (timerFd, &ticksSinceLastRead, sizeof (ticksSinceLastRead));
        if (ticksSinceLastRead > 1)
        {
            return (void *) E_MISSED_SCHEDULER_DEADLINE;
        }
    }
}

