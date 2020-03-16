#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include "ThreadManager.hpp"
#include "Errors.hpp"
#include "ProfileHelpers.hpp"

void ProfileHelpers::setThreadPriAndAffinity ()
{
    // Initialize ThreadManager so that kernel environment is set.
    ThreadManager* pTm;
    if (ThreadManager::getInstance (&pTm) != E_SUCCESS)
    {
        throw std::runtime_error ("Failed to initialize Thread Manager");
    }

    // Set priority to lowest FSW thread priority.
    pthread_t currentThread = pthread_self();
    struct sched_param schedParams;
    schedParams.sched_priority = ThreadManager::MIN_NEW_THREAD_PRIORITY;
    if (pthread_setschedparam (currentThread, SCHED_FIFO, &schedParams) != 0)
    {   
        throw std::runtime_error ("Failed to set priority.");
    }   

    // Use core 0 for determinism.
    cpu_set_t cpuset;
    CPU_ZERO (&cpuset);
    CPU_SET (0, &cpuset);
    if (pthread_setaffinity_np(currentThread, sizeof(cpu_set_t), &cpuset) != 0)
    {   
        throw std::runtime_error ("Failed to set affinity.");
    }   
}

Time::TimeNs_t ProfileHelpers::getTimeNs ()
{
    static Time* pTime = nullptr;
    if (pTime == nullptr)
    {
        Error_t ret = Time::getInstance (pTime);
        if (ret != E_SUCCESS)
        {
            throw std::runtime_error ("Failed to init Time");
        }
    }

    Time::TimeNs_t timeNs;
    if (pTime->getTimeNs (timeNs) != E_SUCCESS)
    {
        throw std::runtime_error ("Failed to get time");
    }

    return timeNs;
}

uint64_t ProfileHelpers::measureBaseline ()
{
    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();    

    // End time.
    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    // Calculate elapsed.
    return endNs - startNs;
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
        throw std::runtime_error ("Failed to to open " + filePath);
    }
}

void ProfileHelpers::printVectorStats (std::vector<uint64_t>& results, 
                                       std::string header)
{
    // Type last input to accumulate to be uint64_t to prevent overflow.
    uint64_t avg =  std::accumulate (results.begin (), results.end (), 
                                     (uint64_t) 0) / results.size ();
    uint64_t min = *std::min_element (results.begin (), results.end ());
    uint64_t max = *std::max_element (results.begin (), results.end ());

    std::cout << header << std::endl;
    std::cout << "Average: " << avg << std::endl;
    std::cout << "Min:     " << min << std::endl;
    std::cout << "Max:     " << max << std::endl;
}

