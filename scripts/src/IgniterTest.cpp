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
 * following any changes.
 *
 * These test procedures require an LED be wired to the sbRIO via breakout
 * board, on the DIO pin specified by IGNITER_DIO_PIN_NUM. The script should
 * NOT be tested with the actual igniter circuit.
 *
 * Test 00 - Invalid ignition delay
 *   Procedure:
 *     * Run the script with no ignition delay, non-numeric ignition delay, and
 *       out-of-range ignition delay.
 *   Expected Behavior:
 *     * Program exits immediately and reports the error.
 *     * LED does not light up at any point during the test.
 *
 * Test 01 - Early abort
 *   Procedure:
 *     * Run the script with any ignition delay in the valid range.
 *     * Press ENTER on the keyboard before the countdown hits 0.
 *   Expected Behavior:
 *     * Program exits immediately on abort with "TEST ABORTED BY USER".
 *     * LED does not light up at any point during the test.
 *
 * Test 02 - Early interrupt
 *   Procedure:
 *     * Run the script with any ignition delay in the valid range.
 *     * Issue a CTRL + C interrupt before the countdown hits 0.
 *   Expected Behavior:
 *     * Program exits immediately on interrupt with "TEST INTERRUPTED BY USER".
 *     * LED does not light up at any point during the test.
 *
 * Test 03 - Mid-raise abort
 *   Procedure:
 *     * Change LINE_RAISE_DURATION_MS to 5000 and recompile. This increases
 *       the duration of the line raise to 5 seconds, making the LED flash
 *       easier to interrupt.
 *     * Run the script with any ignition delay in the valid range.
 *     * When the LED lights up, quickly abort the script by pressing ENTER.
 *     * Revert the change to LINE_RAISE_DURATION_MS.
 *   Expected Behavior:
 *     * Program exits immediately on abort with "TEST ABORTED BY USER".
 *     * LED turns on when the countdown hits 0 and then off the moment of
 *       abort. It is not on for anywhere near 5 seconds.
 *
 * Test 04 - Mid-raise interrupt
 *   Procedure:
 *     * Change LINE_RAISE_DURATION_MS to 5000 and recompile. This increases
 *       the duration of the line raise to 5 seconds, making the LED flash
 *       easier to interrupt.
 *     * Run the script with any ignition delay in the valid range.
 *     * When the LED lights up, quickly interrupt the script with CTRL + C.
 *     * Revert the change to LINE_RAISE_DURATION_MS.
 *   Expected Behavior:
 *     * Program exits immediately on interrupt with "TEST INTERRUPTED BY USER".
 *     * LED turns on when the countdown hits 0 and then off the moment of
 *       interrupt. It is not on for anywhere near 5 seconds.
 *
 * Test 05 - Full duration test
 *   Procedure:
 *     * Run the script to completion with any ignition delay and without
 *       aborting or interrupting.
 *   Expected Behavior:
 *     * Program exits with "TEST CONCLUDED" when the countdown hits 0.
 *     * LED flashes briefly the moment the countdown hits 0.
 *     * Test concludes with LED off.
 */

#include <csignal>
#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include "DigitalOutDevice.hpp"
#include "IgniterTest.hpp"
#include "NiFpga.h"
#include "NiFpga_IO.h"
#include "ScriptHelpers.hpp"
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
 * Path to bit file on sbRIO.
 */
#define BIT_FILE_PATH "/home/admin/FlightSoftware/"

/**
 * Pin number for igniter DIO line.
 */
#define IGNITER_DIO_PIN_NUM 5

/********************************** GLOBALS ***********************************/

/**
 * Upper and lower bounds on the ignition delay.
 */
static const double IGNITION_DELAY_LOWER_S = 5;
static const double IGNITION_DELAY_UPPER_S = 10;

/**
 * Number of milliseconds that the DIO line is raised for during ignition.
 */
static const uint32_t LINE_RAISE_DURATION_MS = 100;

/**
 * Lock for synchronizing FPGA calls, which may be made by both the ignition and
 * abort threads.
 */
pthread_mutex_t gLineLock;

/**
 * Ignition delay in seconds. Written once by main thread and then read once
 * by ignition thread.
 */
double gIgnitionDelayS;

/**
 * DIO device for igniter line.
 */
std::unique_ptr<DigitalOutDevice> gPIgniterDev;

/**
 * State vector used in DIO device configuration.
 */
std::shared_ptr<StateVector> gPSv;

/******************************** PROTOTYPES **********************************/

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
 * Raises the DIO line and lowers it LINE_RAISE_DURATION_MS milliseconds later.
 * Program is slept for the duration of the raise and may safely be interrupted
 * during this time without the line remaining high.
 */
void ignite ();

/**
 * Initializes the igniter line DIO device.
 */
void initLine ();

/**
 * Raises the DIO line.
 */
void raiseLine ();

/**
 * Lowers the DIO line.
 */
void lowerLine ();

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
    ScriptHelpers::sleepMs (LINE_RAISE_DURATION_MS);
    lowerLine ();
}

void initLine ()
{
    // Initialize FPGA session.
    NiFpga_Session session;
    NiFpga_Status status = NiFpga_Initialize ();
    NiFpga_MergeStatus (&status, NiFpga_Open (
        BIT_FILE_PATH NiFpga_IO_Bitfile,
        NiFpga_IO_Signature, "RIO0", 0, &session));
    if (status != NiFpga_Status_Success)
    {
        ERROR ("Error: FPGA initialized with unsuccessful status %d", status);
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
        ERROR("Error: failed to initialize state vector");
    }

    // Initialize igniter DIO device.
    DigitalOutDevice::Config_t deviceConfig =
    {
        SV_ELEM_IGNTEST_CONTROL_VAL,
        SV_ELEM_IGNTEST_FEEDBACK_VAL,
        IGNITER_DIO_PIN_NUM
    };
    err = Device::createNew (session, gPSv, deviceConfig, gPIgniterDev);
    if (err != E_SUCCESS)
    {
        ERROR("Error: failed to initialize DIO device");
    }
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
        ABORT ("Error: failed to raise DIO line");
    }

    err = gPIgniterDev->run ();
    if (err != E_SUCCESS)
    {
        ABORT ("Error: failed to update DIO device");
    }

    pthread_mutex_unlock (&gLineLock);
}

void sigIntHandler (int kSignum)
{
    ABORT ("\nTEST INTERRUPTED BY USER");
}

/******************************** ENTRY POINT *********************************/

void IgniterTest::main (int ac, char** av)
{
    // Enforce correct usage.
    if (ac != 2)
    {
        ERROR ("Usage: %s [IGNITION DELAY IN SECONDS]", av[0]);
    }

    // Validate the user-specified ignition delay. Generates an exception on
    // non-numeric input.
    gIgnitionDelayS = std::stof (av[1]);
    if (gIgnitionDelayS < IGNITION_DELAY_LOWER_S ||
        gIgnitionDelayS > IGNITION_DELAY_UPPER_S)
    {
        ERROR ("Error: ignition delay must be between %.1f and %.1f seconds",
               IGNITION_DELAY_LOWER_S, IGNITION_DELAY_UPPER_S);
    }

    // Install SIGINT handler for lowering DIO line on program exit.
    struct sigaction action;
    action.sa_handler = sigIntHandler;
    if (sigaction (SIGINT, &action, nullptr) != 0)
    {
        ERROR ("Error: failed to install signal handler");
    }

    // Initialize DIO line lock.
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init (&attr) != 0 ||
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
        ERROR ("Error: failed to initialize lock attributes");
    }
    if (pthread_mutex_init (&gLineLock, &attr) != 0)
    {
        ERROR ("Error: failed to initialize lock");
    }

    // Initialize igniter DIO device.
    initLine ();

    // Clear terminal so output is more evident to test operator.
    system ("clear");

    // Create the thread manager.
    ThreadManager* pThreadManager = nullptr;
    Error_t err = ThreadManager::getInstance (&pThreadManager);
    if (err != E_SUCCESS)
    {
        ERROR ("Error: failed to create thread manager");
    }

    // Create the abort thread that stops the countdown when enter is pressed.
    pthread_t abortThread;
    ThreadManager::ThreadFunc_t* pAbortThreadFunc =
            (ThreadManager::ThreadFunc_t*) &abortThreadFunc;
    err = pThreadManager->createThread (abortThread, pAbortThreadFunc, nullptr,
                                        0,
                                        ThreadManager::MAX_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0);
    if (err != E_SUCCESS)
    {
        ERROR ("Error: failed to create abort thread");
    }

    // Create the ignition thread that counts down and raises the DIO line. This
    // thread has a lower priority than the abort thread.
    pthread_t ignitionThread;
    ThreadManager::ThreadFunc_t* pIgnitionThreadFunc =
            (ThreadManager::ThreadFunc_t*) &ignitionThreadFunc;
    err = pThreadManager->createThread (ignitionThread, pIgnitionThreadFunc,
                                        nullptr, 0,
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::CORE_0);
    if (err != E_SUCCESS)
    {
        ERROR ("Error: failed to create ignition thread");
    }

    // Block until test conclusion.
    pThreadManager->waitForThread (ignitionThread, err);
}
