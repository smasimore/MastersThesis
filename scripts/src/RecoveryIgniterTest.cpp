/**
 * ATTENTION: THIS SCRIPT AND THE ACCOMPANYING CIRCUIT ARE DESIGNED TO ACTUATE
 * BLACK POWDER. IGNITION TEST OPERATORS SHOULD BE TRAINED BY A MEMBER OF THE
 * RECOVERY AVIONICS TEAM BEFORE USE. BEFORE A TEST WITH BLACK POWDER, TEST
 * OPERATORS MUST RUN THROUGH THE TEST PROCEDURES BELOW TO VERIFY CIRCUIT AND
 * SCRIPT CORRECTNESS.
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
 *     * Change gLINE_RAISE_DURATION_S to 5 and recompile. This increases the
 *       duration of the line raise to 5 seconds, making the ignition easier
 *       to interrupt.
 *     * Run the script with any ignition delay in the valid range.
 *     * When the countdown hits 0, quickly abort the script by pressing ENTER.
 *     * Revert the change to gLINE_RAISE_DURATION_S.
 *   Expected Behavior:
 *     * Program exits immediately on abort with "TEST ABORTED BY USER".
 *     * Multimeter reads 1 A when the countdown hits 0 and then 0 A the moment
 *       of abort. The 1 A is not sustained for anywhere near 5 seconds.
 *
 * Test 04 - Mid-raise interrupt
 *   Procedure:
 *     * Change gLINE_RAISE_DURATION_S to 5 and recompile. This increases the
 *       duration of the line raise to 5 seconds, making the ignition easier
 *       to interrupt.
 *     * Run the script with any ignition delay in the valid range.
 *     * When the countdown hits 0, quickly interrupt the script with CTRL + C.
 *     * Revert the change to gLINE_RAISE_DURATION_S.
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
#include <stdexcept>
#include <stdio.h>

#include "DigitalOutDevice.hpp"
#include "NiFpga.h"
#include "NiFpga_IO.h"
#include "RecoveryIgniterTest.hpp"
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
 * Lowers the igniter DIO line and exits the program with a message if an
 * expression does not evaluate to E_SUCCESS.
 *
 * @param   kExpr Expression to evaluate.
 */
#define EXIT_ON_ERR(kExpr)                                                     \
{                                                                              \
    Error_t err = kExpr;                                                       \
    if (err != E_SUCCESS)                                                      \
    {                                                                          \
        lowerLine ();                                                          \
        ERROR ("Program failed with error %d", err);                           \
    }                                                                          \
}

/**
 * Path to bit file on sbRIO.
 */
#define BIT_FILE_PATH "/home/admin/FlightSoftware/"

/**
 * Pin number for igniter line. This should probably be different from the DIO
 * pin raised in the DigitalOutDevice unit test, otherwise accidentally running
 * the unit test script can cause ignition.
 */
#define IGNITER_DIO_PIN_NUM 5

/********************************** GLOBALS ***********************************/

/**
 * Number of seconds that the DIO line is raised for during ignition.
 */
const double gLINE_RAISE_DURATION_S = 0.75;

/**
 * Lock for synchronizing FPGA calls, which may be made by both the ignition and
 * abort threads.
 */
pthread_mutex_t gFPGALock;

/**
 * Whether or not an abort was triggered by an interrupt. Written by the SIGINT
 * handler, which executes in an arbitrary thread, and read by the ignition
 * thread.
 */
volatile bool gAbortPending = false;

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
double gIgnitionDelayS = std::numeric_limits<double>::infinity ();

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
 * Raises the DIO line and lowers it gLINE_RAISE_DURATION_S milliseconds later.
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

    // Block until ignition duration has elapsed, checking for aborts by
    // interrupt all the while.
    const double T_IGNITE_START_S = ScriptHelpers::timeS ();
    while (ScriptHelpers::timeS () - T_IGNITE_START_S < gLINE_RAISE_DURATION_S)
    {
        if (gAbortPending)
        {
            ABORT ("\nTEST INTERRUPTED BY USER");
        }
    }

    lowerLine ();
}

void raiseLine ()
{
    // If device has yet to be initialized, don't bother.
    if (gPIgniterDev == nullptr)
    {
        return;
    }

    pthread_mutex_lock (&gFPGALock);

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

    pthread_mutex_unlock (&gFPGALock);
}

void lowerLine ()
{
    // If device has yet to be initialized, don't bother.
    if (gPIgniterDev == nullptr)
    {
        return;
    }

    pthread_mutex_lock (&gFPGALock);

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

    pthread_mutex_unlock (&gFPGALock);
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
        return E_WRONG_ARGC;
    }

    // Try parsing the ignition delay, and catch non-numeric input.
    try
    {
        gIgnitionDelayS = std::stof (kAv[1]);
    }
    catch (const std::invalid_argument& e)
    {
        return E_INVALID_ARGUMENT;
    }

    // Verify delay is in valid range.
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

    return E_SUCCESS;
}

Error_t initDevice ()
{
    // Initialize FPGA lock.
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init (&attr) != 0 ||
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
        return E_FAILED_TO_INIT_LOCK;
    }
    if (pthread_mutex_init (&gFPGALock, &attr) != 0)
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

    // Run device to ensure line is brought low.
    gPIgniterDev->run ();

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

    return E_SUCCESS;
}

Error_t waitForConclusion ()
{
    Error_t err;
    return gPThreadManager->waitForThread (gIgnitionThread, err);
}

/******************************** ENTRY POINT *********************************/

void RecoveryIgniterTest::main (int kAc, char** kAv)
{
    // Validate user-specified ignition delay.
    if (validateInput (kAc, kAv) != E_SUCCESS)
    {
        ABORT ("Usage: %s [IGNITION DELAY IN SECONDS]\n"
               "Ignition delay must be between %.1f and %.1f seconds", kAv[0],
               gIGNITION_DELAY_LOWER_S, gIGNITION_DELAY_UPPER_S);
    }

    // Install SIGINT handler for lowering DIO line on program interrupt.
    struct sigaction action = {};
    action.sa_handler = sigIntHandler;
    if (sigaction (SIGINT, &action, nullptr) != 0)
    {
        ABORT ("Error: failed to install signal handler");
    }

    // Clear terminal so output is more evident to test operator.
    system ("clear");

    // Run test.
    EXIT_ON_ERR (initFPGA          ());
    EXIT_ON_ERR (initDevice        ());
    EXIT_ON_ERR (initThreads       ());
    EXIT_ON_ERR (waitForConclusion ());
}
