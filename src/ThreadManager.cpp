#include <dirent.h>
#include <string>

#include "ThreadManager.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/
Error_t ThreadManager::getInstance (ThreadManager **ppThreadManager)
{
    static ThreadManager threadManagerInstance = ThreadManager ();
    static bool initialized = false;

    *ppThreadManager = &threadManagerInstance;

    if (initialized == false)
    {
        Error_t ret = threadManagerInstance.init ();
        if (ret != E_SUCCESS)
        {
            return E_FAILED_TO_INIT_KERNEL_ENV;
        }
    }

    return E_SUCCESS;
}

/**************************** PRIVATE FUNCTIONS *******************************/
Error_t ThreadManager::init () {
    // Hardcode the PID's of the ktimersoftd threads since getting the PID's
    // dynamically is tricky. 
    const static uint8_t KTIMERSOFTD_0_PID = 4;
    const static uint8_t KTIMERSOFTD_1_PID = 20;
    const static char *PROC_NAME_1 = "ktimersoftd/0";
    const static char *PROC_NAME_2 = "ktimersoftd/1";

    // Verify that the PID's map to the correct processes and priority is 1
    // ... not sure how to do othis, process setpriority seems like doesn't
    // work for RT processes? what if use pthread funcs? Appears these PIDs
    // just have 1 thread, so pid should be equal
    // LEFT OFF: open /proc/<pid>/comm, contents should equal string above
    // verify pri is min SCHED_FIFO, scheduling is SCHED_FIFO, then set pri to
    // some greater value but < hw timer
    //FILE *ktimersoftd0File = open ("/proc/" + )


    return E_SUCCESS;
}

ThreadManager::ThreadManager () {}
ThreadManager::ThreadManager (ThreadManager const &) {}
ThreadManager& ThreadManager::operator=(ThreadManager const &) {
    return *this;
};