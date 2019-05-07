#include <dirent.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sched.h>

#include "ThreadManager.hpp"

const uint8_t ThreadManager::KTIMERSOFTD_0_PID = 4;
const uint8_t ThreadManager::KTIMERSOFTD_1_PID = 20;
const uint8_t ThreadManager::KTIMERSOFTD_PRIORITY = 49;
const uint8_t ThreadManager::HW_IRQ_PRIORITY = 50;

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

Error_t ThreadManager::setProcessPriority (const uint8_t pid, 
                                           const uint8_t priority)
{
    // Only allow priorities below hw IRQ thread priority.
    if (priority >= ThreadManager::HW_IRQ_PRIORITY)
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

ThreadManager& ThreadManager::operator=(ThreadManager const &) {
    return *this;
};

Error_t ThreadManager::initKernelSchedulingEnvironment () {
    // 1) Verify that the PID's map to the correct processes.
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

    // 2) Set the priority of the processes to be 49 (1 below the hw IRQ 
    //    priorities of 50).
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
