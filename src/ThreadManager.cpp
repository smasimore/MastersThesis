#include <dirent.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

#include "ThreadManager.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/
Error_t ThreadManager::getInstance (ThreadManager **ppThreadManager)
{
    // 1) Initialize the kernel environment (e.g. set priorities for the timer's
    //    soft interrupt threads.
    static bool initialized = false;
    if (initialized == false)
    {
        Error_t ret = ThreadManager::init ();
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

/**************************** PRIVATE FUNCTIONS *******************************/
ThreadManager::ThreadManager () {}

ThreadManager::ThreadManager (ThreadManager const &) {}

ThreadManager& ThreadManager::operator=(ThreadManager const &) {
    return *this;
};

Error_t ThreadManager::init () {
    // 1) Hardcode the PID's of the ktimersoftd threads since these do not 
    //    appear to change per OS configuration and getting the PID's 
    //    dynamically is tricky. 
    const static uint8_t KTIMERSOFTD_0_PID = 4;
    const static uint8_t KTIMERSOFTD_1_PID = 20;

    // 2) Verify that the PID's map to the correct processes and default 
    //    priority is what we expect.
    const static std::string EXPECTED_KTIMERSOFTD_0_NAME = "ktimersoftd/0";
    const static std::string EXPECTED_KTIMERSOFTD_1_NAME = "ktimersoftd/1";
    const static uint8_t EXPECTED_DEFAULT_PRIORITY = 1;
    const static uint8_t EXPECTED_DEFAULT_SCHED_POLICY = SCHED_FIFO;

    bool verified = false;
    Error_t ret = ThreadManager::verifyProcess (KTIMERSOFTD_0_PID, 
                                                EXPECTED_KTIMERSOFTD_0_NAME, 
                                                EXPECTED_DEFAULT_PRIORITY, 
                                                EXPECTED_DEFAULT_SCHED_POLICY, 
                                                verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    verified = false;
    ret = ThreadManager::verifyProcess (KTIMERSOFTD_1_PID, 
                                        EXPECTED_KTIMERSOFTD_1_NAME, 
                                        EXPECTED_DEFAULT_PRIORITY, 
                                        EXPECTED_DEFAULT_SCHED_POLICY, 
                                        verified);
    if (ret != E_SUCCESS || verified == false)
    {
        return E_FAILED_TO_VERIFY_PROCESS;
    }

    // ... not sure how to do othis, process setpriority seems like doesn't
    // work for RT processes? what if use pthread funcs? Appears these PIDs
    // just have 1 thread, so pid should be equal
    // LEFT OFF: open /proc/<pid>/comm, contents should equal string above
    // verify pri is min SCHED_FIFO, scheduling is SCHED_FIFO, then set pri to
    // some greater value but < hw timer
    //FILE *ktimersoftd0File = open ("/proc/" + )


    return E_SUCCESS;
Error on stream (such as when this function catches an exception thrown by an internal operation).
When set, the integrity of the stream may have been affected.
Error_t ThreadManager::verifyProcess (const uint8_t pid, 
                                      const std::string expectedName, 
                                      const uint8_t expectedPri, 
                                      const uint8_t expectedSchedPolicy, 
                                      bool &verified)
{
    // todo: check for errors on return

    // 1) Get name of process by reading the /proc/<pid>/comm file.
    const static std::string PROC_DIR = "/proc/";
    const static std::string PROC_COMM_DIR = "/comm";
    std::string procNameFilePath = PROC_DIR + std::to_string (pid) + PROC_COMM_DIR;
    std::fstream procNameFile;

    // Open file.
    procNameFile.open (procNameFilePath, std::ios::in);
    if ((procNameFile.rdstate () & std::ifstream::failbit) != 0)
    {
        return E_FAILED_TO_OPEN_FILE;
    }

    // Read file.
    const static uint8_t MAX_NAME_LEN = 32;
    char actualName[MAX_NAME_LEN];
    procNameFile.read (actualName, MAX_NAME_LEN);
    if ((procNameFile.rdstate () & 
        (std::ifstream::failbit | std::ifstream::badbit) != 0)
    {
        return E_FAILED_TO_READ_FILE;
    }

    // LEFT OFF HERE

    // Close file.
    procNameFile.close();
    if ((procNameFile.rdstate () & std::ifstream::failbit) != 0)
    {
        return E_FAILED_TO_CLOSE_FILE;
    }

    //  todo: compare

    // TODO
    verified = true;

    return E_SUCCESS;
}
