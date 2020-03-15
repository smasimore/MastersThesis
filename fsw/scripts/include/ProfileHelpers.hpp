/**
 * Helper functions used for profiling.
 */

# ifndef PROFILE_HELPERS_HPP
# define PROFILE_HELPERS_HPP

#include <string>
#include <vector>

#include "Time.hpp"

namespace ProfileHelpers
{

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

}

# endif
