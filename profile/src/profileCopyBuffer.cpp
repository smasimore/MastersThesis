/**
 * Measure time it takes to copy a vector<uint8_t> with 2000 elements. This
 * simulates copying the Data Vector's buffer for 500 elements of 4 bytes
 * each using 3 different copy methods.
 *
 * The purpose of this profiling script is to better understand cost of copying
 * regions of (or the entire) Data Vector for the purposes of tx'ing/rx'ing
 * across the flight network. The alternative to passing around copies of
 * regions/Data Vector is to pass the Network Manager a pointer to the region/
 * Data Vector.
 */

#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "ProfileHelpers.hpp"

/**
 * Set to true to print additional debug information.
 */
static bool gDebugPrint = false;

/**
 * # of times to run.
 */
static const uint32_t NUM_TIMES_TO_RUN = 10000;

/**
 * Value to place in each byte of buffer.
 */
static const uint8_t  BUF_FILL = 0xff;

/**
 * Size of buffer.
 */
static const uint32_t BUF_SIZE = 2000;

/**
 * Measure time to copy buf to a vector initialized with buf size.
 */
uint64_t measureCopyTime_InitVecSize (std::vector<uint8_t>& buf, 
                                      uint16_t runIdx)
{
    // Start time.
    uint64_t startNs = ProfileHelpers::getTimeNs ();    
    
    // Create another vector with size initialized and copy the buffer to it. 
    std::vector<uint8_t> bufCopy (BUF_SIZE);
    bufCopy = buf;

    // End time.
    uint64_t endNs = ProfileHelpers::getTimeNs ();

    // Calculate elapsed.
    uint64_t elapsedNs = abs (endNs - startNs);

    // Print results.
    if (gDebugPrint == true)
    {
        std::cout << "RUN " << runIdx << " INIT_VEC_SIZE NS: " << elapsedNs 
            << std::endl;
        ProfileHelpers::printProcessStats ();
    }

    return elapsedNs;
}

/**
 * Measure time to copy buf to a vector without an initial size.
 */
uint64_t measureCopyTime_NoInitVecSize (std::vector<uint8_t>& buf, 
                                        uint16_t runIdx)
{
    // Start time.
    uint64_t startNs = ProfileHelpers::getTimeNs ();    
    
    // Create another vector without initializing it size, and copy the buffer 
    // to it. 
    std::vector<uint8_t> bufCopy;
    bufCopy = buf;

    // End time.
    uint64_t endNs = ProfileHelpers::getTimeNs ();

    // Calculate elapsed.
    uint64_t elapsedNs = abs (endNs - startNs);

    // Print results.
    if (gDebugPrint == true)
    {
        std::cout << "RUN " << runIdx << " NO_INIT_VEC_SIZE NS: " << elapsedNs 
            << std::endl;
        ProfileHelpers::printProcessStats ();
    }

    return elapsedNs;
}

/**
 * Measure time to copy buf to a vector initialized with buf size.
 */
uint64_t measureCopyTime_StaticVec (std::vector<uint8_t>& buf, uint16_t runIdx)
{
    // Start time.
    uint64_t startNs = ProfileHelpers::getTimeNs ();    
    
    // Create another vector with size initialized and copy the buffer to it. 
    static std::vector<uint8_t> bufCopy (BUF_SIZE);
    bufCopy = buf;

    // End time.
    uint64_t endNs = ProfileHelpers::getTimeNs ();

    // Calculate elapsed.
    uint64_t elapsedNs = abs (endNs - startNs);

    // Print results.
    if (gDebugPrint == true)
    {
        std::cout << "RUN " << runIdx << " STATIC_VEC NS: " << elapsedNs 
            << std::endl;
        ProfileHelpers::printProcessStats ();
    }

    return elapsedNs;
}

int main ()
{
    // Set thread to be SCHED_FIFO.
    ProfileHelpers::setThreadPriAndAffinity ();

    std::vector<uint8_t> buf (BUF_SIZE, BUF_FILL);

    std::vector<uint64_t> results_Baseline      (NUM_TIMES_TO_RUN);
    std::vector<uint64_t> results_InitVecSize   (NUM_TIMES_TO_RUN);
    std::vector<uint64_t> results_NoInitVecSize (NUM_TIMES_TO_RUN);
    std::vector<uint64_t> results_StaticVec     (NUM_TIMES_TO_RUN);

    // Do in 2 separate for loops so that individual run prints are grouped.
    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_Baseline[i] = ProfileHelpers::measureBaseline ();
    }

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_InitVecSize[i] = measureCopyTime_InitVecSize (buf, i);
    }

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_NoInitVecSize[i] = measureCopyTime_NoInitVecSize (buf, i);
    }

    for (uint16_t i = 0; i < NUM_TIMES_TO_RUN; i++)
    {
        results_StaticVec[i] = measureCopyTime_StaticVec (buf, i);
    }

    std::cout << "------ Results ------" << std::endl;
    std::cout << "# of runs: " << NUM_TIMES_TO_RUN << std::endl;

    ProfileHelpers::printVectorStats (results_Baseline,      
                                      "\nBASELINE");
    ProfileHelpers::printVectorStats (results_InitVecSize,   
                                      "\nINIT_VEC_SIZE");
    ProfileHelpers::printVectorStats (results_NoInitVecSize, 
                                      "\nNO_INIT_VEC_SIZE");
    ProfileHelpers::printVectorStats (results_StaticVec,     
                                      "\nSTATIC_VEC");
}
