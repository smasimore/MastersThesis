/**
 * ATTENTION: THIS SCRIPT AND THE ACCOMPANYING CIRCUIT ARE DESIGNED TO ACTUATE
 * BLACK POWDER. IGNITION TEST OPERATORS SHOULD BE TRAINED BY A MEMBER OF THE
 * RECOVERY AVIONICS TEAM BEFORE USE.
 *
 *
 * SUMMARY:
 *
 * Script for pyro ignition tests conducted by the deployment team. The program
 * accepts a countdown length in seconds from the command line, counts down,
 * raises the igniter circuit DIO line for a brief moment, and then exits.
 * Pressing ENTER or CTRL + C prior to or during ignition will lower the DIO
 * line and abort the test.
 *
 *
 * TESTING PROCEDURES:
 *
 * The following manual test procedures should be performed to test the script
 * following any changes. Prior to this, all unit tests must pass.
 *
 * These test procedures require the sbRIO be wired to the igniter circuit via
 * breakout board, loaded with a multimeter instead of an actual igniter. The
 * multimeter should be configured to display amperage.
 *
 * Test 01 - Early abort
 *   Procedure:
 *     * Run the script with any ignition delay in the valid range.
 *     * Press ENTER on the keyboard before the countdown hits 0.
 *   Expected Behavior:
 *     * Program exits immediately on abort with "TEST ABORTED BY USER".
 *     * Multimeter does not exceed 0 A at any point throughout the test.
 *
 * Test 02 - Early interrupt
 *   Procedure:
 *     * Run the script with any ignition delay in the valid range.
 *     * Issue a CTRL + C interrupt before the countdown hits 0.
 *   Expected Behavior:
 *     * Program exits immediately on interrupt with "TEST INTERRUPTED BY USER".
 *     * Multimeter does not exceed 0 A at any point throughout the test.
 *
 * Test 03 - Mid-raise abort
 *   Procedure:
 *     * Change LINE_RAISE_DURATION_MS to 5000 and recompile. This increases
 *       the duration of the line raise to 5 seconds, making the ignition
 *       easier to interrupt.
 *     * Run the script with any ignition delay in the valid range.
 *     * When the countdown hits 0, quickly abort the script by pressing ENTER.
 *     * Revert the change to LINE_RAISE_DURATION_MS.
 *   Expected Behavior:
 *     * Program exits immediately on abort with "TEST ABORTED BY USER".
 *     * Multimeter reads 1 A when the countdown hits 0 and then 0 A the moment
 *       of abort. The 1 A is not sustained for anywhere near 5 seconds.
 *
 * Test 04 - Mid-raise interrupt
 *   Procedure:
 *     * Change LINE_RAISE_DURATION_MS to 5000 and recompile. This increases
 *       the duration of the line raise to 5 seconds, making the ignition
 *       easier to interrupt.
 *     * Run the script with any ignition delay in the valid range.
 *     * When the countdown hits 0, quickly interrupt the script with CTRL + C.
 *     * Revert the change to LINE_RAISE_DURATION_MS.
 *   Expected Behavior:
 *     * Program exits immediately on interrupt with "TEST INTERRUPTED BY USER".
 *     * Multimeter reads 1 A when the countdown hits 0 and then 0 A the moment
 *       of interrupt. The 1 A is not sustained for anywhere near 5 seconds.
 *
 * Test 05 - Full duration test
 *   Procedure:
 *     * Run the script to completion with any ignition delay and without
 *       aborting or interrupting.
 *   Expected Behavior:
 *     * Program exits with "TEST CONCLUDED" when the countdown hits 0.
 *     * Multimeter briefly reads 1 A when the countdown hits 0 and then 0 A.
 *     * Test concludes with multimeter reading 0 A.
 */

#include <csignal>
#include <iostream>
#include <limits>
#include <memory>
#include <pthread.h>
#include <stdio.h>

#include "DigitalOutDevice.hpp"
#include "IgniterTest.hpp"
#include "NiFpga.h"
#include "NiFpga_IO.h"
#include "ScriptHelpers.hpp"
#include "StateVector.hpp"
#include "ThreadManager.hpp"
#include "TimeNs.hpp"

/**
 * Lowers the igniter DIO line and errs.
 *
 * @param   kFmt    Exit message.
 *          [other] Format arguments.
 */
#define ABORT(kFmt, ...)                                                       \
{                                                                              \
    lowerLine ();                                                              \
    ERROR (kFmt, ##__VA_ARGS__);                                               \
}

/**
 * Exits the program with a message if an expression does not evaluate to
 * E_SUCCESS
 *
 * @param   kExpr Expression to evaluate.
 */
#define EXIT_ON_ERR(kExpr)                                                     \
{                                                                              \
    Error_t err = kExpr;                                                       \
    if (err != E_SUCCESS)                                                      \
    {                                                                          \
        ERROR ("Program failed with error %d", err);                           \
    }                                                                          \
}

/**
 * Aborts the program if the igniter line value is different from expected.
 *
 * @param   kExpectedVal True if line should be high, false otherwise.
 */
#define VERIFY_LINE(kExpectedVal)                                              \
{                                                                              \
    bool kActualVal;                                                           \
    Error_t err = getLineVal (kActualVal);                                     \
    if (err != E_SUCCESS)                                                      \
    {                                                                          \
        ABORT ("Failed to query igniter line value");                          \
    }                                                                          \
    if (kActualVal != kExpectedVal)                                            \
    {                                                                          \
        ABORT ("Igniter line value mismatch: expected %d", kExpectedVal);      \
    }                                                                          \
}

/**
 * Path to bit file on sbRIO.
 */
#define BIT_FILE_PATH "/home/admin/FlightSoftware/"

/**
 * Pin number for igniter line and corresponding NiFpga enum. THESE MUST
 * CORRESPOND.
 */
#define IGNITER_DIO_PIN_NUM       5
#define IGNITER_DIO_PIN_NIFPGA_IO NiFpga_IO_IndicatorBool_inDIO5

/********************************** GLOBALS ***********************************/

/**
 * Number of milliseconds that the DIO line is raised for during ignition.
 */
const uint32_t gLINE_RAISE_DURATION_MS = 750;

/**
 * Lock for synchronizing FPGA calls, which may be made by both the ignition and
 * abort threads.
 */
pthread_mutex_t gLineLock;

/**
 * Whether or not an abort was triggered. Written by the abort thread, read
 * by the ignition thread.
 */
bool gAbortPending = false;

/**
 * DIO device for igniter line.
 */
std::unique_ptr<DigitalOutDevice> gPIgniterDev = nullptr;

/**
 * State vector used in DIO device configuration.
 */
std::shared_ptr<StateVector> gPSv = nullptr;

/**
 * Thread manager and the two threads it creates during the test.
 */
ThreadManager* gPThreadManager = nullptr;
pthread_t gAbortThread;
pthread_t gIgnitionThread;

/**
 * Ignition delay in seconds. Written once by main thread and then read once
 * by ignition thread.
 *
 * "SENSIBLE DEFAULTS" - Alison Norman
 */
double gIgnitionDelayS = std::numeric_limits<double>::infinity();

/**
 * FPGA session and status.
 */
NiFpga_Session gSession;
NiFpga_Status gStatus;

/******************************** PROTOTYPES **********************************/

/**
 * Initializes the FPGA session.
 *
 * @ret     E_SUCCESS   Init succeeded.
 *          E_FPGA_INIT Init failed.
 */
Error_t initFPGA ();

/**
 * Initializes the igniter DIO device.
 *
 * @ret     E_SUCCESS             Init succeeded.
 *          E_FAILED_TO_INIT_LOCK DIO line lock failed to initialize.
 *          [other]               Error from state vector or device creation.
 */
Error_t initDevice ();

/**
 * Creates the thread manager and kicks off ignition and abort threads.
 *
 * @ret     E_SUCCESS Init succeeded.
 *          [other]   Error from thread manager or thread creation.
 */
Error_t initThreads ();

/**
 * Blocks until test conclusion, i.e. ignition thread exits.
 *
 * @ret     E_SUCCESS Success.
 *          [other]   Error from thread manager.
 */
Error_t waitForConclusion ();

/**
 * Raises the DIO line.
 */
void raiseLine ();

/**
 * Lowers the DIO line.
 */
void lowerLine ();

/**
 * Gets whether or not the line is high from the FPGA session.
 *
 * @param   kVal        Becomes true if high or potentially floating, false
 *                      otherwise.
 *
 * @ret     E_SUCCESS   Checked line successfully.
 *          E_FPGA_READ Failed to get line value.
 *
 */
Error_t getLineVal (bool& kVal);

/**
 * Function run by the abort thread. Thread blocks while waiting for a line of
 * input from stdin and kills the test if received.
 *
 * @param   kUnused Unused.
 *
 * @ret     Unused.
 */
void* abortThreadFunc (void* kUnused);

/**
 * Function run by the ignition thread. Prints a coundown timer, raises the
 * DIO line at the end, and returns.
 *
 * @param   kUnused Unused.
 *
 * @ret     Unused.
 */
void* ignitionThreadFunc (void* kUnused);

/**
 * Raises the DIO line and lowers it gLINE_RAISE_DURATION_MS milliseconds later.
 * Program is slept for the duration of the raise and may safely be interrupted
 * during this time without the line remaining high.
 */
void ignite ();

/**
 * Signal handler that ensures the DIO line is lowered if the program is
 * interrupted.
 *
 * @param   kSignum Signal number.
 */
void sigIntHandler (int kSignum);

/********************************* FUNCTIONS **********************************/

void* abortThreadFunc (void* kUnused)
{
    std::string in;
    std::getline (std::cin, in);
    ABORT ("\nTEST ABORTED BY USER");
    return nullptr; // Not reached.
}

void* ignitionThreadFunc (void* kUnused)
{
    const double T_IGNITION_DELAY_S = gIgnitionDelayS;
    const double T_COUNTDOWN_START_S = ScriptHelpers::timeS ();
    bool igniterLit = false;

    printf ("BEGINNING IGNITION COUNTDOWN...\n");

    while (!igniterLit)
    {
        // Check if an abort was triggered.
        if (gAbortPending)
        {
            ABORT ("\nTEST INTERRUPTED BY USER");
        }

        // Igniter line should be low at this point.
        VERIFY_LINE (false);

        // Compute the time elapsed since the start of the countdown.
        double tCurrentTimeS = ScriptHelpers::timeS ();
        double tElapsedS = tCurrentTimeS - T_COUNTDOWN_START_S;

        // Only run ignition logic if the abort thread has not disabled it.
        if (tElapsedS > T_IGNITION_DELAY_S)
        {
            igniterLit = true;
            ignite ();
        }
        else
        {
            printf ("\rIgnition in %05.2f seconds",
                    T_IGNITION_DELAY_S - tElapsedS);
        }

        // Sleep for a bit to make the timer print a bit smoother.
        ScriptHelpers::sleepMs(1);
    }

    printf ("\nTEST CONCLUDED\n");
    return nullptr;
}

void ignite ()
{
    raiseLine ();
    ScriptHelpers::sleepMs (gLINE_RAISE_DURATION_MS);
    lowerLine ();
}

void raiseLine ()
{
    pthread_mutex_lock (&gLineLock);

    Error_t err = gPSv->write (SV_ELEM_IGNTEST_CONTROL_VAL, true);
    if (err != E_SUCCESS)
    {
        ABORT ("Error: failed to raise DIO line");
    }

    err = gPIgniterDev->run ();
    if (err != E_SUCCESS)
    {
        ABORT ("Error: failed to update DIO device");
    }

    pthread_mutex_unlock (&gLineLock);
}

void lowerLine ()
{
    pthread_mutex_lock (&gLineLock);

    Error_t err = gPSv->write (SV_ELEM_IGNTEST_CONTROL_VAL, false);
    if (err != E_SUCCESS)
    {
        ABORT ("Error: failed to lower DIO line");
    }

    err = gPIgniterDev->run ();
    if (err != E_SUCCESS)
    {
        ABORT ("Error: failed to update DIO device");
    }

    pthread_mutex_unlock (&gLineLock);
}

Error_t getLineVal (bool& kVal)
{
    NiFpga_MergeStatus (&gStatus,
                        NiFpga_ReadBool (gSession,
                                         IGNITER_DIO_PIN_NIFPGA_IO,
                                         (NiFpga_Bool*) &kVal));
    if (gStatus != NiFpga_Status_Success)
    {
        return E_FPGA_READ;
    }

    return E_SUCCESS;
}

void sigIntHandler (int kSignum)
{
    gAbortPending = true;
}

Error_t validateInput (int kAc, char** kAv)
{
    // Enforce correct usage.
    if (kAc != 2)
    {
        return E_TEST_ERROR;
    }

    gIgnitionDelayS = std::stof (kAv[1]);
    if (gIgnitionDelayS < gIGNITION_DELAY_LOWER_S ||
        gIgnitionDelayS > gIGNITION_DELAY_UPPER_S)
    {
        return E_OUT_OF_BOUNDS;
    }

    return E_SUCCESS;
}

Error_t initFPGA ()
{
    gStatus = NiFpga_Initialize ();
    NiFpga_MergeStatus (&gStatus, NiFpga_Open (
        BIT_FILE_PATH NiFpga_IO_Bitfile,
        NiFpga_IO_Signature, "RIO0", 0, &gSession));
    if (gStatus != NiFpga_Status_Success)
    {
        return E_FPGA_INIT;
    }

    // Igniter line should be low at this point.
    ScriptHelpers::sleepMs (10);
    VERIFY_LINE (false);

    return E_SUCCESS;
}

Error_t initDevice ()
{
    // Initialize DIO line lock.
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init (&attr) != 0 ||
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
        return E_FAILED_TO_INIT_LOCK;
    }
    if (pthread_mutex_init (&gLineLock, &attr) != 0)
    {
        return E_FAILED_TO_INIT_LOCK;
    }

    // Initialize state vector.
    StateVector::StateVectorConfig_t config
    {
        {
            {SV_REG_TEST0,
            {
                SV_ADD_BOOL(SV_ELEM_IGNTEST_CONTROL_VAL,  false),
                SV_ADD_BOOL(SV_ELEM_IGNTEST_FEEDBACK_VAL, false)
            }}
        }
    };
    Error_t err = StateVector::createNew (config, gPSv);
    if (err != E_SUCCESS)
    {
        return err;
    }

    // Initialize igniter DIO device.
    DigitalOutDevice::Config_t deviceConfig =
    {
        SV_ELEM_IGNTEST_CONTROL_VAL,
        SV_ELEM_IGNTEST_FEEDBACK_VAL,
        IGNITER_DIO_PIN_NUM
    };
    err = Device::createNew (gSession, gPSv, deviceConfig, gPIgniterDev);
    if (err != E_SUCCESS)
    {
        return err;
    }

    // Igniter line should be low at this point.
    ScriptHelpers::sleepMs (10);
    VERIFY_LINE (false);

    return E_SUCCESS;
}

Error_t initThreads ()
{
    // Create the thread manager.
    Error_t err = ThreadManager::getInstance (&gPThreadManager);
    if (err != E_SUCCESS)
    {
        return err;
    }

    // Create the abort thread that stops the countdown when enter is pressed.
    ThreadManager::ThreadFunc_t* pAbortThreadFunc =
            (ThreadManager::ThreadFunc_t*) &abortThreadFunc;
    err = gPThreadManager->createThread (
            gAbortThread, pAbortThreadFunc, nullptr, 0,
            ThreadManager::MAX_NEW_THREAD_PRIORITY,
            ThreadManager::Affinity_t::CORE_0);
    if (err != E_SUCCESS)
    {
        return err;
    }

    // Create the ignition thread that counts down and raises the DIO line. This
    // thread has a lower priority than the abort thread.
    ThreadManager::ThreadFunc_t* pIgnitionThreadFunc =
            (ThreadManager::ThreadFunc_t*) &ignitionThreadFunc;
    err = gPThreadManager->createThread (
            gIgnitionThread, pIgnitionThreadFunc, nullptr, 0,
            ThreadManager::MIN_NEW_THREAD_PRIORITY,
            ThreadManager::Affinity_t::CORE_0);
    if (err != E_SUCCESS)
    {
        return err;
    }
}

Error_t waitForConclusion ()
{
    Error_t err;
    return gPThreadManager->waitForThread (gIgnitionThread, err);
}

/******************************** ENTRY POINT *********************************/

void IgniterTest::main (int kAc, char** kAv)
{
    if (validateInput (kAc, kAv) != E_SUCCESS)
    {
        ERROR ("Usage: %s [IGNITION DELAY IN SECONDS]", kAv[0]);
    }

    // Clear terminal so output is more evident to test operator.
    system ("clear");

    // Run test.
    EXIT_ON_ERR(initFPGA          ());
    EXIT_ON_ERR(initDevice        ());
    EXIT_ON_ERR(initThreads       ());
    EXIT_ON_ERR(waitForConclusion ());
}
