/**
 * See AnalogInDeviceTest.hpp for instructions on running test.
 */

#include <ctime>
#include <stdio.h>
#include <math.h>

#include "AnalogInDevice.hpp"
#include "AnalogInDeviceTest.hpp"
#include "FPGAConstants.hpp"
#include "FPGASession.hpp"
#include "ProfileHelpers.hpp"
#include "Time.hpp"

/**
 * First AIN pin to test. The script tests AnalogInDevices on pins
 * AIN_START_PIN, AIN_START_PIN + 1, AIN_START_PIN + 2, and AIN_START_PIN + 3.
 */
#define AIN_START_PIN 0

/**
 * Generates a random float in a range.
 *
 * @param   kLow  Lower bound.
 * @param   kHigh Upper bound.
 *
 * @ret     Random float on [kLow, kHigh).
 */
#define RANDOM_RANGE(kLow, kHigh)                                              \
    (kLow + (float) (rand ()) / ((float) (RAND_MAX / (kHigh - kLow))))

/********************************** GLOBALS ***********************************/

static NiFpga_Session gSession;
static NiFpga_Status  gStatus;

static DataVector::Config_t gDvConfig =
{
    // Region
    {DV_REG_TEST0,

    // Elements
    {
        DV_ADD_FLOAT (DV_ELEM_TEST0, 0),
        DV_ADD_FLOAT (DV_ELEM_TEST1, 0),
    }},
};

static Time* gPTime = nullptr;

static std::shared_ptr<DataVector> gPDv = nullptr;

/******************************* HELPER FUNCS *********************************/

/**
 * Transfer function for test device. Function is y=2x.
 *
 * @param   kV        Voltage.
 * @param   kEngrRet  Engineering unit.
 *
 * @ret     E_SUCCESS Always succeeds.
 */
static Error_t testTransferFunc (float kV, float& kEngrRet)
{
    kEngrRet = 2 * kV;
    return E_SUCCESS;
}

/**
 * Writes a voltage to an analog out pin.
 *
 * @param   kPinNumber AOUT pin number.
 * @param   kVoltage   Voltage to write.
 */
static void writeAnalogOut (uint8_t kPinNumber, float kVoltage)
{
    uint32_t fxp = NiFpga_ConvertFromFloatToFxp (AOUT_FXP_INFO_VEC[kPinNumber],
                                                 kVoltage);
    NiFpga_MergeStatus (&gStatus, NiFpga_WriteU32 (gSession, 
                        AOUT_FXP_RESOURCE_VEC[kPinNumber], fxp));
    if (gStatus != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_WRITE, "Failed to write analog out.");
    }
}

/**
 * Runs an AnalogInDevice read test. The test writes a specified voltage to an
 * AOUT pin and then spins until a device on the corresponding AIN pin
 * correctly reads the signal or too much time elapses.
 *
 * @param   kPinNumber    Device pin number.
 * @param   kRange        Input voltage range FPGA constant.
 * @param   kMode         Input mode FPGA constant.
 * @param   kVoltage      Voltage to test.
 * @param   kElapsedNsVec Vector to append test elapsed time to.
 */
static void runDeviceTest (uint8_t kPinNumber, uint8_t kRange, uint8_t kMode,
                           float kVoltage,
                           std::vector<Time::TimeNs_t>& kElapsedNsVec)
{
    // Timeout on waiting for expected output from device.
    static const Time::TimeNs_t READ_TIMEOUT_NS = 1 * Time::NS_IN_S;

    // How long device must sustain expected output for test to pass.
    static const Time::TimeNs_t OUTPUT_SUSTAIN_NS = 10 * Time::NS_IN_MS;

    // Acceptable error bound in device voltage measurement.
    static const float ERROR_BOUND_V = 0.005;

    // AOUT pin wired to kPinNumber.
    const uint8_t AOUT_PIN_NUM = kPinNumber - AIN_START_PIN;

    // Verify specified voltage is in range.
    if (fabs (kVoltage) > AI_RANGES_V[kRange])
    {
        Errors::exitOnError (E_INVALID_ARGUMENT, "Voltage out of range.");
    }

    // Configure and create device.
    AnalogInDevice::Config_t config =
    {
        DV_ELEM_TEST0,
        DV_ELEM_TEST1,
        kPinNumber,
        (AnalogInDevice::TransferFunc_t*) &testTransferFunc,
        (AnalogInDevice::Range_t) kRange,
        (AnalogInDevice::Mode_t) kMode,
    };
    std::unique_ptr<AnalogInDevice> pDevice;
    Errors::exitOnError (Device::createNew (gSession, gPDv, config, pDevice),
                         "Failed to create device.");

    // True if device output matched expectations last step of the loop. Used to
    // check that the device sustains the expected output over a period of time.
    bool outputMatch = false;

    // Time of last false -> true flip of outputMatch.
    Time::TimeNs_t tOutputMatchNs = 0;

    float deviceOutputV       = 0;
    float deviceOutputEngr    = 0;
    float expectedOutputEngr  = 0;
    bool gotExpectedOutput    = false;
    Time::TimeNs_t tElapsedNs = 0;

    // Write voltage to AOUT pin and record test start time.
    writeAnalogOut (AOUT_PIN_NUM, kVoltage);

    Time::TimeNs_t tStartNs = 0;
    Errors::exitOnError (gPTime->getTimeNs (tStartNs), "Failed to get time.");

    // Spin until the expected device output is seen or test times out.
    while (!gotExpectedOutput)
    {
        // Compute elapsed time and timeout if necessary.
        Time::TimeNs_t tCurrentNs = 0;
        Errors::exitOnError (gPTime->getTimeNs (tCurrentNs),
                             "Failed to get time.");
        tElapsedNs = tCurrentNs - tStartNs;
        if (tElapsedNs > READ_TIMEOUT_NS)
        {
            break;
        }

        // Run device.
        Errors::exitOnError (pDevice->run (), "Failed to run device.");

        // Determine if voltage output matches expected.
        Errors::exitOnError (gPDv->read (DV_ELEM_TEST0, deviceOutputV),
                             "Failed to read DV.");
        bool matchV = fabs (deviceOutputV - kVoltage) <= ERROR_BOUND_V;

        // Determine if engineering unit output matches expected.
        Errors::exitOnError (gPDv->read (DV_ELEM_TEST1, deviceOutputEngr),
                             "Failed to read DV.");
        expectedOutputEngr = 0;
        Errors::exitOnError (testTransferFunc (deviceOutputV, expectedOutputEngr),
                             "Transfer function failed.");
        bool matchEngr = (expectedOutputEngr == deviceOutputEngr);

        bool outputMatchLast = outputMatch;
        outputMatch = matchV && matchEngr;

        // If the output just began matching expectations, record the time.
        if (outputMatch && !outputMatchLast)
        {
            tOutputMatchNs = tCurrentNs;
        }
        // If the output has continued to match expectations, compute how
        // long the match has lasted and exit loop if sufficient for test pass.
        else if (outputMatch)
        {
            Time::TimeNs_t matchLengthNs = tCurrentNs - tOutputMatchNs;
            if (matchLengthNs >= OUTPUT_SUSTAIN_NS)
            {
                gotExpectedOutput = true;
            }
        }
    }

    // Print results.
    if (gotExpectedOutput)
    {
        printf ("Test passed in %010lluns", tElapsedNs);
    }
    else
    {
        printf ("%-27s", "TEST TIMED OUT");
    }

    printf (" | Pin %02d, Range %02d, Mode %d"
            " | Expected output: %08.4fV (%08.4f engr)"
            " | Actual output: %08.4fV (%08.4f engr)\n",
            kPinNumber, kRange, kMode, kVoltage, expectedOutputEngr,
            deviceOutputV, deviceOutputEngr);

    kElapsedNsVec.push_back (tElapsedNs);
    
    // Lower pin.
    writeAnalogOut (AOUT_PIN_NUM, 0);
}

/**
 * Runs device tests across a voltage range in 1V increments. For a voltage
 * range +/- R, this will test voltages -R, -R + 1, ..., R - 1, R.
 *
 * @param   kPinNumber    Device pin number.
 * @param   kRange        Input voltage range FPGA constant.
 * @param   kMode         Input mode FPGA constant.
 * @param   kElapsedNsVec Vector to append test elapsed time to.
 */
static void runRangeTests (uint8_t kPinNumber, uint8_t kRange, uint8_t kMode,
                           std::vector<Time::TimeNs_t>& kElapsedNsVec)
{
    // Determine numeric voltage range.
    float rangeV = AI_RANGES_V[kRange];

    // Step across range in 1V increments and run tests.
    uint8_t steps = (uint8_t) (rangeV * 2);
    for (uint8_t i = 0; i <= steps; i++)
    {
        float voltage = -rangeV + i;
        runDeviceTest (kPinNumber, kRange, kMode, voltage, kElapsedNsVec);
    }
}

/**
 * Run device tests with random voltages in the specified range.
 *
 * @param   kPinNumber    Device pin number.
 * @param   kRange        Input voltage range FPGA constant.
 * @param   kMode         Input mode FPGA constant.
 * @param   kElapsedNsVec Vector to append test elapsed time to.
 */
static void runRandomTests (uint8_t kPinNumber, uint8_t kRange, uint8_t kMode,
                            std::vector<Time::TimeNs_t>& kElapsedNsVec)
{
    // Number of tests to run.
    static const uint8_t NUM_RAND_TESTS = 10;

    // Determine numeric voltage range.
    float rangeV = AI_RANGES_V[kRange];

    // Run tests with randomly generated voltages in range.
    for (uint8_t i = 0; i < NUM_RAND_TESTS; i++)
    {
        float voltage = RANDOM_RANGE (-rangeV, rangeV);
        runDeviceTest (kPinNumber, kRange, kMode, voltage, kElapsedNsVec);
    }
}

/******************************** ENTRY POINT *********************************/

void AnalogInDeviceTest::main (int ac, char** av)
{
    // Validate starting pin.
    if (AIN_START_PIN < 0 || AIN_START_PIN > 12)
    {
        Errors::exitOnError (E_INVALID_ARGUMENT, "Invalid starting pin.");
    }

    // Init FPGA session.
    Errors::exitOnError (FPGASession::getSession (gSession, gStatus),
                         "Failed to get FPGA session.");
    if (gStatus != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_INIT, "Failed to init FPGA session.");
    }

    // Init DV.
    Errors::exitOnError (DataVector::createNew (gDvConfig, gPDv),
                         "Failed to create DV.");

    // Init Time.
    Errors::exitOnError (Time::getInstance (gPTime), "Failed to create Time.");

    // Seed RNG.
    srand (std::time (0));

    // Run tests for the 4 selected pins.
    std::vector<Time::TimeNs_t> elapsedNsVec;
    for (uint8_t pinNum = AIN_START_PIN; pinNum < AIN_START_PIN + 4; pinNum++)
    {
        runRangeTests (pinNum, AI_RANGE_10V, AI_MODE_RSE, elapsedNsVec);
        runRangeTests (pinNum, AI_RANGE_5V,  AI_MODE_RSE, elapsedNsVec);
        runRangeTests (pinNum, AI_RANGE_2V,  AI_MODE_RSE, elapsedNsVec);
        runRangeTests (pinNum, AI_RANGE_1V,  AI_MODE_RSE, elapsedNsVec);

        runRandomTests (pinNum, AI_RANGE_10V, AI_MODE_RSE, elapsedNsVec);
        runRandomTests (pinNum, AI_RANGE_5V,  AI_MODE_RSE, elapsedNsVec);
        runRandomTests (pinNum, AI_RANGE_2V,  AI_MODE_RSE, elapsedNsVec);
        runRandomTests (pinNum, AI_RANGE_1V,  AI_MODE_RSE, elapsedNsVec);

        // Only run differential tests on pins < 8 (see note (1) at the top of
        // AnalogInDevice.hpp).
        if (pinNum < 8)
        {
            runRangeTests (pinNum, AI_RANGE_10V, AI_MODE_DIFF, elapsedNsVec);
            runRangeTests (pinNum, AI_RANGE_5V,  AI_MODE_DIFF, elapsedNsVec);
            runRangeTests (pinNum, AI_RANGE_2V,  AI_MODE_DIFF, elapsedNsVec);
            runRangeTests (pinNum, AI_RANGE_1V,  AI_MODE_DIFF, elapsedNsVec);

            runRandomTests (pinNum, AI_RANGE_10V, AI_MODE_DIFF, elapsedNsVec);
            runRandomTests (pinNum, AI_RANGE_5V,  AI_MODE_DIFF, elapsedNsVec);
            runRandomTests (pinNum, AI_RANGE_2V,  AI_MODE_DIFF, elapsedNsVec);
            runRandomTests (pinNum, AI_RANGE_1V,  AI_MODE_DIFF, elapsedNsVec);
        }
    }

    ProfileHelpers::printVectorStats (elapsedNsVec,
                                      "---- TEST DURATION (NS) ----");
}