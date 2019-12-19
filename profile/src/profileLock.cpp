/**
 * Measure time it takes to lock and unlock the State Vector.
 *
 * The purpose of this profiling script is to better understand cost of locking
 * and unlocking the State Vector. If cheap, the SV implementation would be 
 * simplifying by always locking/unlocking instead of branching depending on
 * the context.
 */

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "StateVector.hpp"
#include "ProfileHelpers.hpp"

/**
 * # of times to run.
 */
static const uint32_t NUM_TIMES_TO_RUN = 10000;

/**
 * Measure time to lock and unlock the State Vector.
 */
uint64_t measureLockTime (uint16_t runIdx, std::shared_ptr<StateVector>& pSv)
{
    // Start time.
    uint64_t startNs = ProfileHelpers::getTimeNs ();    

    // Acquire lock.
    pSv->acquireLock ();
    pSv->releaseLock ();

    // End time.
    uint64_t endNs = ProfileHelpers::getTimeNs ();

    // Return elapsed.
    return abs (endNs - startNs);
}

int main ()
{
    ProfileHelpers::setThreadPriAndAffinity ();

    // Initialize State Vector.
    Error_t ret = E_SUCCESS;
    std::shared_ptr<StateVector> pSv;
    StateVector::StateVectorConfig_t config = {
        // Regions
        {    
            //////////////////////////////////////////////////////////////////////////////////
        
            // Region
            {SV_REG_TEST0,
        
            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                   SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            ),
            }},
            //////////////////////////////////////////////////////////////////////////////////
        }    
    };
    StateVector::createNew (config, pSv);
    if (ret != E_SUCCESS)
    {
        throw "Failed to initialize State Vector.";
    }

    std::vector<uint64_t> results_Baseline (NUM_TIMES_TO_RUN);
    std::vector<uint64_t> results_Lock (NUM_TIMES_TO_RUN);

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_Baseline[i] = ProfileHelpers::measureBaseline ();
    }

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_Lock[i] = measureLockTime (i, pSv);
    }

    std::cout << "------ Results ------" << std::endl;
    std::cout << "# of runs: " << NUM_TIMES_TO_RUN << std::endl;

    ProfileHelpers::printVectorStats (results_Baseline,      
                                      "\nBASELINE");
    ProfileHelpers::printVectorStats (results_Lock,   
                                      "\nLOCK");
}
