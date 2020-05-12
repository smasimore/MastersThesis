#include <iostream>
#include <vector>
#include <cmath>

#include "FPGASession.hpp"
#include "ProfileFpgaApi.hpp"
#include "ScriptHelpers.hpp"
#include "ProfileHelpers.hpp"
#include "FPGAConstants.hpp"

/**
 * Measures time to read analog input.
 *
 * @param   kSession     FPGA session.
 * @param   kMode        Diff or RSE. See constants defined below.
 * @param   kRangeV      +/- 10V, 5V, 2V, or 1V. See constants defined below.
 * @param   kResultsVec  Vector to store time results in.
 */
#define MEASURE_AIN_READ(kSession, kMode, kRangeV, kResultsVec)                \
{                                                                              \
    initAnalogIn (kSession, kMode, kRangeV);                                   \
    for (uint16_t i = 0; i < NUM_RUNS; i++)                                    \
    {                                                                          \
        kResultsVec[i] = measureAnalogInRead (session);                        \
    }                                                                          \
}

/**
 * # of times to run.
 */
static const uint32_t NUM_RUNS = 10000;

/**
 * Initialize FPGA digital pins as input or output.
 *
 * @param   kSession   FPGA session.
 * @param   kOutput    True if pin should be set as output.
 */
static void initDigital (NiFpga_Session& kSession, bool kOutput)
{
    NiFpga_Status status = NiFpga_Status_Success;
    for (int32_t outputEnable : DOUT_ENABLE_VEC)
    {
        // Set pin as input or output.
        NiFpga_MergeStatus (&status, NiFpga_WriteBool (kSession, outputEnable,
                                                       (NiFpga_Bool) kOutput));
        if (status != NiFpga_Status_Success)
        {
            Errors::exitOnError (E_FPGA_WRITE, "Failed to init digital pin");
        }
    }
}

/**
 * Initialize FPGA analog in pin modes and ranges.
 *
 * @param   kSession   FPGA session.
 * @param   kMode      Diff or RSE. See constants defined above.
 * @param   kRangeV    +/- 10V, 5V, 2V, or 1V. See constants defined above.
 */
static void initAnalogIn (NiFpga_Session& kSession, uint8_t kMode, 
                          uint8_t kRangeV)
{
    NiFpga_Status status = NiFpga_Status_Success;
    for (uint8_t i = 0; i < AIN_MODE_VEC.size (); i++)
    {
        // Set mode.
        NiFpga_MergeStatus (&status, NiFpga_WriteU8 (kSession, AIN_MODE_VEC[i],
                                                     kMode));
        if (status != NiFpga_Status_Success)
        {
            Errors::exitOnError (E_FPGA_WRITE, "Failed to set analog in mode");
        }

        // Set range.
        NiFpga_MergeStatus (&status, NiFpga_WriteU8 (kSession, AIN_RANGE_VEC[i], 
                                                     kRangeV));
        if (status != NiFpga_Status_Success)
        {
            Errors::exitOnError (E_FPGA_WRITE, "Failed to set analog in range");
        }
    }
}

/**
 * Initialize the first four FPGA analog in pin modes and ranges. These pins 
 * will be used to read the analog out values to verify an output value has
 * successfully be set and reflected in the hardware.
 *
 * @param   kSession   FPGA session.
 */
static void initAnalogOut (NiFpga_Session& kSession)
{
    NiFpga_Status status = NiFpga_Status_Success;
    for (uint8_t i = 0; i < NUM_AOUT_PINS; i++)
    {
        // Set mode.
        NiFpga_MergeStatus (&status, NiFpga_WriteU8 (kSession, AIN_MODE_VEC[i],
                                                     AI_MODE_RSE));
        if (status != NiFpga_Status_Success)
        {
            Errors::exitOnError (E_FPGA_WRITE, "Failed to set analog in mode");
        }

        // Set range.
        NiFpga_MergeStatus (&status, NiFpga_WriteU8 (kSession, AIN_RANGE_VEC[i], 
                                                     AI_RANGE_10V));
        if (status != NiFpga_Status_Success)
        {
            Errors::exitOnError (E_FPGA_WRITE, "Failed to set analog in range");
        }
    }
}

/**
 * Measure time to read a digital in pin. Loops over set of digital in pins to 
 * simulate sampling pattern we expect on a Device Node.
 *
 * @param  kSession  FPGA session.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureDigitalInRead (NiFpga_Session& kSession)
{
    // Keep track of idx between function calls to loop over pins.
    static uint8_t idx = 0;
    NiFpga_Status status = NiFpga_Status_Success;
    bool pinValue = false;

    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Read pin.
    NiFpga_MergeStatus (&status, NiFpga_ReadBool (kSession, 
                                                  DIN_INDICATOR_VEC[idx],
                                                  (NiFpga_Bool*) &pinValue));
    if (status != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_READ, "Failed to read digital in");
    }

    // End time.
    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    // Increment idx.
    idx = (idx == DIN_INDICATOR_VEC.size () - 1) ? 0 : idx + 1;

    // Return elapsed.
    return endNs - startNs;
}

/**
 * Measure time to write to a digital out pin. If kWait is set to true, includes
 * time it takes for the written value to show up in a read. Loops over set of 
 * digital out pins to simulate write pattern we expect on a Device Node. After 
 * setting each pin, switches write value.
 *
 * @param  kSession  FPGA session.
 * @param  kWait     Wait for written value to be reflected by a read.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureDigitalOutWrite (NiFpga_Session& kSession,
                                              bool kWait)
{
    // Keep track of which pin being written to and value to write so that we
    // can loop over pins and alternate value being written between function
    // calls.
    static uint8_t idx = 0;
    static bool pinWrite = true;
    NiFpga_Status status = NiFpga_Status_Success;
    bool pinRead = false;

    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Write pin.
    NiFpga_MergeStatus (&status, NiFpga_WriteBool (kSession, 
                                                   DOUT_CONTROL_VEC[idx],
                                                   pinWrite));
    if (status != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_WRITE, "Failed to write digital out");
    }

    // If kWait set to true, wait until read reflects write value.
    if (kWait == true)
    {
        while (1)
        {
            // Read pin value.
            NiFpga_MergeStatus (&status, 
                                NiFpga_ReadBool (kSession, 
                                                 DIN_INDICATOR_VEC[idx], 
                                                 (NiFpga_Bool*) &pinRead));   
            if (status != NiFpga_Status_Success)
            {
                Errors::exitOnError (E_FPGA_READ, "Failed to read digital in");
            }

            // Break out if value read is same as value written.
            if (pinRead == pinWrite)
            {
                break;
            }
        }
    }

    // End time.
    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    // Increment idx and switch write value at end of loop.
    if (idx == DOUT_CONTROL_VEC.size () - 1)
    {
        idx = 0;
        pinWrite = pinWrite == true ? false : true;
    }
    else
    {
        idx++;
    }

    // Return elapsed.
    return endNs - startNs;
}

/**
 * Measure time to read an analog in pin. Loops over set of analog in pins to 
 * simulate sampling pattern we expect on a Device Node.
 *
 * @param  kSession  FPGA session.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureAnalogInRead (NiFpga_Session& kSession)
{
    // Keep track of idx between function calls to loop over pins.
    static uint8_t idx = 0;
    NiFpga_Status status = NiFpga_Status_Success;
    uint32_t pinValueFxp = 0;

    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Read fxp value.
    NiFpga_MergeStatus (&status, NiFpga_ReadU32 (
                                             kSession, 
                                             AIN_FXP_RESOURCE_VEC[idx],
                                             &pinValueFxp));
    if (status != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_READ, "Failed to read analog in");
    }

    // Convert fxp to float. This is included in time measurement since it is
    // expected to occur every analog read.
    NiFpga_ConvertFromFxpToFloat (AIN_FXP_INFO_VEC[idx],
                                  (uint64_t) pinValueFxp);

    // End time.
    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    // Increment idx.
    idx = (idx == AIN_FXP_RESOURCE_VEC.size () - 1) ? 0 : idx + 1;

    // Return elapsed.
    return endNs - startNs;
}

/**
 * Measure time to write to an analog out pin. If kWait is set to true, include
 * time it takes for written value to show up in a read. Loops over set of 
 * analog out pins to simulate sampling pattern we expect on a Device Node. 
 * After setting each pin, switches write value.
 *
 * @param  kSession  FPGA session.
 * @param  kWait     Wait for written value to be reflected by a read.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureAnalogOutWrite (NiFpga_Session& kSession, 
                                             bool kWait)
{
    // Max and min values to write to pin.
    static const float PIN_WRITE_MAX = 1;
    static const float PIN_WRITE_MIN = -1;

    // Permitted error between value read and value written.
    static const float ERROR_BOUND_V = .01;

    // Keep track of which pin being written to and value to write so that we
    // can loop over pins and alternate value being written between function
    // calls.
    static uint8_t idx = 0;
    static float pinWriteFloat = PIN_WRITE_MAX;
    NiFpga_Status status = NiFpga_Status_Success;

    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Convert float to write to fxp.
    uint64_t pinWriteFxp = NiFpga_ConvertFromFloatToFxp (AOUT_FXP_INFO_VEC[idx],
                                                         pinWriteFloat);

    // Write fxp.
    NiFpga_MergeStatus (&status, NiFpga_WriteU32 (kSession, 
                                                  AOUT_FXP_RESOURCE_VEC[idx], 
                                                  pinWriteFxp));
    if (status != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_WRITE, "Failed to write analog out");
    }

    // If kWait set, wait until read reflects write value.
    if (kWait == true)
    {
        while (1)
        {
            // Read fxp value.
            uint32_t pinReadFxp = 0;
            NiFpga_MergeStatus (&status, 
                                NiFpga_ReadU32 (kSession, 
                                                AIN_FXP_RESOURCE_VEC[idx],
                                                &pinReadFxp));
            if (status != NiFpga_Status_Success)
            {
                Errors::exitOnError (E_FPGA_READ, "Failed to read analog in");
            }

            // Convert fxp to float.
            float pinReadFloat = NiFpga_ConvertFromFxpToFloat (
                                                      AIN_FXP_INFO_VEC[idx],
                                                      (uint64_t) pinReadFxp);
            if (status != NiFpga_Status_Success)
            {
                Errors::exitOnError (E_FPGA_READ, "Failed to read digital in");
            }

            // If read value is within error bounds, break.
            if (std::abs (pinReadFloat - pinWriteFloat) < ERROR_BOUND_V)
            {
                break;
            }
        }
    }

    // End time.
    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    // Increment idx and switch write value at end of loop.
    if (idx == NUM_AOUT_PINS - 1)
    {
        idx = 0;
        pinWriteFloat = pinWriteFloat == PIN_WRITE_MAX 
            ? PIN_WRITE_MIN 
            : PIN_WRITE_MAX;
    }
    else
    {
        idx++;
    }

    // Return elapsed.
    return endNs - startNs;
}

void ProfileFpgaApi::main (int ac, char** av)
{
    // Set thread properties.
    ProfileHelpers::setThreadPriAndAffinity ();

    // Initialize FPGA.
    NiFpga_Session session;
    NiFpga_Status status = NiFpga_Status_Success;
    Errors::exitOnError (FPGASession::getSession (session, status),
                         "FPGA init");
    if (status != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_INIT, "FPGA init unsuccessful");
    }

    // Initialize result buffers. Static to prevent stack overflow.
    static std::vector<Time::TimeNs_t> baseline      (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> dInRead       (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> dOutWrite     (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> dOutWriteWait (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInDiff10V    (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInDiff5V     (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInDiff2V     (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInDiff1V     (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInRse10V     (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInRse5V      (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInRse2V      (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aInRse1V      (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aOutWrite     (NUM_RUNS, 0);
    static std::vector<Time::TimeNs_t> aOutWriteWait (NUM_RUNS, 0);

    // Measure baseline.
    for (uint16_t i = 0; i < NUM_RUNS; i++)
    {
        baseline[i] = ProfileHelpers::measureBaseline ();
    }

    // Initialize FPGA DIO pins to be inputs and measure time to read pins in a
    // loop.
    initDigital (session, false);
    for (uint16_t i = 0; i < NUM_RUNS; i++)
    {
        dInRead[i] = measureDigitalInRead (session);
    }

    // Initialize FPGA DIO pins to be outputs and measure time to set pins in a
    // loop, first without waiting for a read to reflect value and then with.
    initDigital (session, true);
    for (uint16_t i = 0; i < NUM_RUNS; i++)
    {
        dOutWrite[i] = measureDigitalOutWrite (session, false);
    }
    for (uint16_t i = 0; i < NUM_RUNS; i++)
    {
        dOutWriteWait[i] = measureDigitalOutWrite (session, true);
    }

    // Initialize FPGA analog in pins to have all mode and range configurations
    // and measure time to read values.
    MEASURE_AIN_READ (session, AI_MODE_DIFF, AI_RANGE_10V, aInDiff10V);
    MEASURE_AIN_READ (session, AI_MODE_DIFF, AI_RANGE_10V, aInDiff5V);
    MEASURE_AIN_READ (session, AI_MODE_DIFF, AI_RANGE_10V, aInDiff2V);
    MEASURE_AIN_READ (session, AI_MODE_DIFF, AI_RANGE_10V, aInDiff1V);
    MEASURE_AIN_READ (session, AI_MODE_RSE,  AI_RANGE_10V, aInRse10V);
    MEASURE_AIN_READ (session, AI_MODE_RSE,  AI_RANGE_10V, aInRse5V);
    MEASURE_AIN_READ (session, AI_MODE_RSE,  AI_RANGE_10V, aInRse2V);
    MEASURE_AIN_READ (session, AI_MODE_RSE,  AI_RANGE_10V, aInRse1V);

    // Initialize first four FPGA analog in pins to read the output for the four
    // analog out pins. This must also be configured in the hardware circuit. 
    // Measure time it takes to write a new value first without waiting, and 
    // then waiting for that value to be reflected in the analog in pin.
    initAnalogOut (session);
    for (uint16_t i = 0; i < NUM_RUNS; i++)
    {
        aOutWrite[i] = measureAnalogOutWrite (session, false);
    }
    for (uint16_t i = 0; i < NUM_RUNS; i++)
    {
        aOutWriteWait[i] = measureAnalogOutWrite (session, true);
    }

    std::cout << "------ Results ------" << std::endl;
    std::cout << "# of runs: " << NUM_RUNS << std::endl;
    ProfileHelpers::printVectorStats (baseline,      "\nBASELINE");
    ProfileHelpers::printVectorStats (dInRead,       "\nDIN_READ");
    ProfileHelpers::printVectorStats (dOutWrite,     "\nDOUT_WRITE");
    ProfileHelpers::printVectorStats (dOutWriteWait, "\nDOUT_WRITE_AND_WAIT");
    ProfileHelpers::printVectorStats (aInDiff10V,    "\nAIN_DIFF_10_READ");
    ProfileHelpers::printVectorStats (aInDiff5V,     "\nAIN_DIFF_5_READ");
    ProfileHelpers::printVectorStats (aInDiff2V,     "\nAIN_DIFF_2_READ");
    ProfileHelpers::printVectorStats (aInDiff1V,     "\nAIN_DIFF_1_READ");
    ProfileHelpers::printVectorStats (aOutWrite,     "\nAOUT_WRITE");
    ProfileHelpers::printVectorStats (aOutWriteWait, "\nAOUT_WRITE_AND_WAIT");
}
