/**
 * Measure time it takes to lock and unlock the Data Vector.
 *
 * The purpose of this profiling script is to better understand cost of locking
 * and unlocking the Data Vector. If cheap, the DV implementation would be 
 * simplifying by always locking/unlocking instead of branching depending on
 * the context.
 */

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "DataVector.hpp"
#include "ProfileHelpers.hpp"
#include "ProfileLock.hpp"

/**
 * # of times to run.
 */
static const uint32_t NUM_TIMES_TO_RUN = 10000;

/**
 * Measure time to lock and unlock the Data Vector.
 */
uint64_t measureLockTime (uint16_t runIdx, std::shared_ptr<DataVector>& pDv)
{
    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Acquire lock.
    pDv->acquireLock ();
    pDv->releaseLock ();

    // End time.
    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    // Return elapsed.
    return abs (endNs - startNs);
}

void ProfileLock::main (int ac, char** av)
{
    ProfileHelpers::setThreadPriAndAffinity ();

    // Initialize Data Vector.
    Error_t ret = E_SUCCESS;
    std::shared_ptr<DataVector> pDv;
    DataVector::Config_t config = {
        // Regions
        {
            ////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,
        
            // Elements
            // TYPE                 ELEM           INITIAL_VALUE
            {
               DV_ADD_UINT8  (  DV_ELEM_TEST0,           0            ),
            }},
            ////////////////////////////////////////////////////////////////////
        }
    };
    DataVector::createNew (config, pDv);
    if (ret != E_SUCCESS)
    {
        throw "Failed to initialize Data Vector.";
    }

    std::vector<uint64_t> results_Baseline (NUM_TIMES_TO_RUN);
    std::vector<uint64_t> results_Lock (NUM_TIMES_TO_RUN);

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_Baseline[i] = ProfileHelpers::measureBaseline ();
    }

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_Lock[i] = measureLockTime (i, pDv);
    }

    std::cout << "------ Results ------" << std::endl;
    std::cout << "# of runs: " << NUM_TIMES_TO_RUN << std::endl;

    ProfileHelpers::printVectorStats (results_Baseline,
                                      "\nBASELINE");
    ProfileHelpers::printVectorStats (results_Lock,
                                      "\nLOCK");
}
