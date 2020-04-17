#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <string.h>

#include "Errors.hpp"
#include "ProfileHelpers.hpp"

void ProfileHelpers::setThreadPriAndAffinity ()
{
    // Initialize ThreadManager so that kernel environment is set.
    ThreadManager* pTm;
    if (ThreadManager::getInstance (pTm) != E_SUCCESS)
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

    // Use core 1 for determinism.
    cpu_set_t cpuset;
    CPU_ZERO (&cpuset);
    CPU_SET (1, &cpuset);
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

Time::TimeNs_t ProfileHelpers::measureBaseline ()
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
        throw std::runtime_error ("Failed to open " + filePath);
    }
}

void ProfileHelpers::printVectorStats (std::vector<Time::TimeNs_t>& kResults, 
                                       std::string kHeader)
{
    // Type last input to accumulate to be uint64_t to prevent overflow.
    uint64_t avg =  std::accumulate (kResults.begin (), kResults.end (), 
                                     (uint64_t) 0) / kResults.size ();
    uint64_t min = *std::min_element (kResults.begin (), kResults.end ());
    uint64_t max = *std::max_element (kResults.begin (), kResults.end ());

    std::cout << kHeader << std::endl;
    std::cout << "Average: " << avg << std::endl;
    std::cout << "Min:     " << min << std::endl;
    std::cout << "Max:     " << max << std::endl;
}

void ProfileHelpers::printVectorStats (std::vector<int64_t>& kResults, 
                                       std::string kHeader)
{
    // Type last input to accumulate to be int64_t to prevent overflow.
    int64_t avg =  std::accumulate (kResults.begin (), kResults.end (), 
                                    (int64_t) 0) / kResults.size ();
    int64_t min = *std::min_element (kResults.begin (), kResults.end ());
    int64_t max = *std::max_element (kResults.begin (), kResults.end ());

    std::vector<uint64_t> absResults (kResults.size ());
    for (uint32_t i = 0; i < kResults.size (); i++)
    {
        absResults[i] = std::abs (kResults[i]);
    }
    uint64_t absAvg =  std::accumulate (absResults.begin (), absResults.end (), 
                                    (uint64_t) 0) / absResults.size ();

    std::cout << kHeader << std::endl;
    std::cout << "Average:     " << avg << std::endl;
    std::cout << "Abs Average: " << absAvg << std::endl;
    std::cout << "Min:         " << min << std::endl;
    std::cout << "Max:         " << max << std::endl;
}

std::map<uint32_t, ProfileHelpers::ProcessStats_t> 
    ProfileHelpers::getProcessStats ()
{
    const uint32_t MAX_PID = 2000;

    // Loop over PID's 0 - 1999 and store stats.
    std::map<uint32_t, ProfileHelpers::ProcessStats_t> pidToStats;
    for (uint32_t pid = 0; pid < MAX_PID; pid++)
    {
        // 1) Open proc stat and status files.
        std::string statFilePath = "/proc/" + std::to_string (pid) + "/stat";
        std::string statusFilePath = "/proc/" + std::to_string (pid) + 
                                     "/status";
        std::ifstream statFile (statFilePath);
        std::ifstream statusFile (statusFilePath);

        // 2) If either file not open, pid doesn't exist.
        if (statFile.is_open() == false || statusFile.is_open () == false)
        {
            continue;
        }

        // 3) Read proc/<pid>/stat file (one line) into a vector, where each 
        //    word is an element.
        std::string statLine = "";
        std::getline (statFile, statLine);
        std::stringstream statLineStream (statLine);
        std::vector<std::string> statWords;
        std::string statWord;
        while (std::getline (statLineStream, statWord, ' '))
        {
            statWords.push_back (statWord);
        }

        // 4) Read proc/<pid>/status file into a vector, where each line is an 
        //    element.
        std::vector<std::string> statusLines;
        std::string statusLine;
        while (std::getline (statusFile, statusLine))
        {
            statusLines.push_back (statusLine);
        }

        // 5) Create info struct.
        ProfileHelpers::ProcessStats_t stats;
        stats.pid = pid;
        stats.name = statWords[1];
        stats.priority = std::stoi (statWords[17]);
        stats.cpuLastRanOn = std::stoi (statWords[38]);

        // 6) Get voluntary context switches. Not all /status files have same # 
        //    of lines, but switches are always last two.
        std::string vCtxSwLine = statusLines[statusLines.size () - 2];
        stats.numVoluntarySwitches = std::stoi (vCtxSwLine.substr (24));

        // 7) Add pid and stats to map.
        pidToStats[pid] = stats;
    }

    return pidToStats;
}

void ProfileHelpers::printActiveProcesses (
                    std::map<uint32_t, ProfileHelpers::ProcessStats_t> kPre,
                    std::map<uint32_t, ProfileHelpers::ProcessStats_t> kPost,
                    ThreadManager::Affinity_t kCpuSet)
{
    for (std::pair<uint32_t, ProfileHelpers::ProcessStats_t> elem : kPre)
    {
        ProfileHelpers::ProcessStats_t pre = elem.second;
        ProfileHelpers::ProcessStats_t post = kPost[pre.pid];

        // Compare pre and post voluntary context switches. If number of 
        // switches didn't go up, skip.
        int32_t numSwitches = post.numVoluntarySwitches - 
                                  pre.numVoluntarySwitches;
        if (numSwitches == 0)
        {
            continue;
        }

        // Check if process ran on a cpu we care about. If not, skip.
        if (kCpuSet == ThreadManager::Affinity_t::CORE_0 && 
            post.cpuLastRanOn != 0)
        {
            continue;
        }
        else if (kCpuSet == ThreadManager::Affinity_t::CORE_1 && 
                 post.cpuLastRanOn != 1)
        {
            continue;
        }

        // Print process' stats.
        std::cout << "PID: " << post.pid << " NAME: " << post.name
            << " PRIORITY: " << post.priority << " CPU: " << post.cpuLastRanOn
            << " NUM VOL SWITCHES: " << numSwitches << std::endl;
    }
}

