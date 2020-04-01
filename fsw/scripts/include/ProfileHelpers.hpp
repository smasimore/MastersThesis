/**
 * Helper functions used for profiling.
 */

# ifndef PROFILE_HELPERS_HPP
# define PROFILE_HELPERS_HPP

#include <string>
#include <vector>
#include <map>

#include "Time.hpp"
#include "ThreadManager.hpp"

namespace ProfileHelpers
{

    /**
     * Struct to hold process stats.
     */
    typedef struct ProcessStats
    {
        uint32_t pid;
        std::string name;
        int32_t priority;
        int32_t cpuLastRanOn;
        int32_t numVoluntarySwitches;
    } ProcessStats_t;

    /**
     * Set current thread to have minimum FSW thread priority and only use core 0.
     */
    void setThreadPriAndAffinity ();

    /**
     * Get current time in ns.
     */
    Time::TimeNs_t getTimeNs ();

    /**
     * Returns baseline time elapsed. This is overhead of calling clock_gettime.
     */
    uint64_t measureBaseline ();

    /**
     * Print /proc/<pid>/status file for process. This is used to debug spikes in
     * elapsed time.
     */
    void printProcessStats ();

    /**
     * Calculate and print avg, min, and max.
     */
    void printVectorStats (std::vector<uint64_t>& results, std::string header);

    /**
     * Get stats for all processes with PID <= 2000.
     *
     * @ret  Map from PID to process' stats.
     */
    std::map<uint32_t, ProcessStats_t> getProcessStats ();

    /**
     * Print processes that ran during an event.
	 *
	 * @param  kPre      PID to ProcessStats_t map collected before event.
	 * @param  kPost     PID to ProcessStats_t map collected after event.
	 * @param  kCpupSet  Set of relevant CPU's.
     */
    void printActiveProcesses (
                    std::map<uint32_t, ProfileHelpers::ProcessStats_t> kPre,
                    std::map<uint32_t, ProfileHelpers::ProcessStats_t> kPost,
                    ThreadManager::Affinity_t kCpuSet);

}

# endif
