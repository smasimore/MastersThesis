#include <dirent.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sched.h>

#include "ThreadManager.hpp"

const uint8_t ThreadManager::KTIMERSOFTD_0_PID = 4;
const uint8_t ThreadManager::KTIMERSOFTD_1_PID = 20;

const uint8_t ThreadManager::HW_IRQ_PRIORITY = 50;
const uint8_t ThreadManager::KTIMERSOFTD_PRIORITY = 
    ThreadManager::HW_IRQ_PRIORITY - 1;
const uint8_t ThreadManager::FSW_INIT_THREAD_PRIORITY = 
    ThreadManager::KTIMERSOFTD_PRIORITY - 1;
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

Error_t ThreadManager::verifyProcess (const uint8_t pid, 
                                      const std::string expectedName, 
                                      bool &verified)
{
    verified = true;

    // 1) Build path to file 
    const static std::string PROC_DIR = "/proc/";
    const static std::string PROC_COMM_DIR = "/comm";
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

Error_t ThreadManager::createThread (pthread_t &thread, 
                                     ThreadManager::ThreadFunc_t * pFunc, 
                                     void *pArgs, 
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

    // 2) Initialize the thread attribute struct.
    pthread_attr_t attr;
    if (pthread_attr_init (&attr) != 0)
    {
        return E_FAILED_TO_INIT_THREAD_ATR;
    }

    // 3) Set thread scheduling policy to SCHED_FIFO. This means the thread will 
    //    only stop executing if a thread with higher priority is ready. Threads 
    //    with the same priority will not be scheduled until this thread is 
    //    blocked or has finished.
    if (pthread_attr_setschedpolicy (&attr, SCHED_FIFO) != 0)
    {
        return E_FAILED_TO_SET_SCHED_POL;
    }

    // 4) Set the thread priority.
    struct sched_param param;
    param.__sched_priority = priority;
    if (pthread_attr_setschedparam (&attr, &param) != 0)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }

    // 5) Set pthread to inherit sched params from attr instead of parent 
    //     thread.
    if (pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED) != 0)
    {
        return E_FAILED_TO_SET_SCHED_INH;
    }

    // 6) Create the thread.
    if (pthread_create (&thread, &attr, (void *(*)(void*)) pFunc, pArgs) != 0)
    {
        return E_FAILED_TO_CREATE_THREAD;
    }

    // 7) Set the CPU affinity of the new thread.
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

    // 8) Destroy thread attr struct. POSIX requires this function always 
    //    succeed, so don't check return. Additionally, if this does fail, the 
    //    consequence is a small amount of leaked memory, so if for some reason 
    //    it does fail, tolerate this error by ignoring it.
    pthread_attr_destroy (&attr);

    return E_SUCCESS;
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

ThreadManager::ThreadManager () {}

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

    // 3) Verify that the hardcoded ktimersoftd/n PID's map to the correct 
    //     processes.
    const static std::string EXPECTED_KTIMERSOFTD_0_NAME = "ktimersoftd/0";
    const static std::string EXPECTED_KTIMERSOFTD_1_NAME = "ktimersoftd/1";
    bool verified = false;
    Error_t ret = ThreadManager::verifyProcess (KTIMERSOFTD_0_PID, 
                                                EXPECTED_KTIMERSOFTD_0_NAME, 
                                                verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    verified = false;
    ret = ThreadManager::verifyProcess (KTIMERSOFTD_1_PID, 
                                        EXPECTED_KTIMERSOFTD_1_NAME, 
                                        verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    // 4) Set the priorities of the ktimersoftd/n threads.
    ret = ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_0_PID, 
                                        ThreadManager::KTIMERSOFTD_PRIORITY);
    if (ret != E_SUCCESS)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }
    ret = ThreadManager::setProcessPriority (ThreadManager::KTIMERSOFTD_1_PID, 
                                        ThreadManager::KTIMERSOFTD_PRIORITY);
    if (ret != E_SUCCESS)
    {
        return E_FAILED_TO_SET_PRIORITY;
    }

    return E_SUCCESS;
}