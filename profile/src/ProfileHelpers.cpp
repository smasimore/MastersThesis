#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>

#include "ThreadManager.hpp"
#include "ProfileHelpers.hpp"

void ProfileHelpers::setThreadPriAndAffinity ()
{
    // Set priority to lowest FSW thread priority.
    pthread_t currentThread = pthread_self();
    struct sched_param schedParams;
    schedParams.sched_priority = ThreadManager::MIN_NEW_THREAD_PRIORITY;
    if (pthread_setschedparam (currentThread, SCHED_FIFO, &schedParams) != 0)
    {   
        throw "Failed to set priority.";
    }   

    // Use core 0 for determinism.
    cpu_set_t cpuset;
    CPU_ZERO (&cpuset);
    CPU_SET (0, &cpuset);
    if (pthread_setaffinity_np(currentThread, sizeof(cpu_set_t), &cpuset) != 0)
    {   
        throw "Failed to set affinity.";
    }   
}

uint64_t ProfileHelpers::getTimeNs ()
{
    static const uint32_t NS_IN_S = 1000000000;

    struct timespec ts;
    if (clock_gettime (CLOCK_MONOTONIC_RAW, &ts) != 0)
    {
        throw "Failed to get time.";
    }

    return (((uint64_t) ts.tv_sec) * NS_IN_S) + ts.tv_nsec;
}

void ProfileHelpers::printProcessStats ()
{
    static pid_t pid = getpid ();
    std::string filePath = "/proc/" + std::to_string (pid) + "/status";
    std::ifstream f (filePath);

    if (f.is_open())
    {
        std::cout << f.rdbuf();
    }
    else
    {
        throw "Failed to to open " + filePath;
    }
}

void ProfileHelpers::printVectorStats (std::vector<uint64_t>& results, 
                                       std::string header)
{
    uint64_t avg =  std::accumulate (results.begin (), results.end (), 0)
                        / results.size ();
    uint64_t min = *std::min_element (results.begin (), results.end ());
    uint64_t max = *std::max_element (results.begin (), results.end ());

    std::cout << header << std::endl;
    std::cout << "Average: " << avg << std::endl;
    std::cout << "Min:     " << min << std::endl;
    std::cout << "Max:     " << max << std::endl;
}

