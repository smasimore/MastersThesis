#include <cstdlib>
#include <time.h>

#include "FPGASession.hpp"

/**
 * Parent directory of FPGA bit file on sbRIO.
 */
#define BIT_FILE_PATH "/home/admin/FlightSoftware/"

/**
 * Global session, accessible only through FPGASession::getSession().
 */
static NiFpga_Session gFpgaSession;

/**
 * Whether or not a session is currently open.
 */
static bool gSessionOpen = false;

/**
 * Whether or not the FPGA API has been initialized.
 */
static bool gNiFpgaInitialized = false;

/**
 * Called at program exit to close global session and finalize the FPGA API.
 */
static void finalizeFpgaAtExit ();

Error_t FPGASession::getSession (NiFpga_Session& kSessionRet,
                                 NiFpga_Status& kStatusRet)
{
    // Zero the status to clear garbage.
    kStatusRet = NiFpga_Status_Success;

    // First call--initialize the FPGA API.
    if (!gNiFpgaInitialized)
    {
        kStatusRet = NiFpga_Initialize ();
    
        if (kStatusRet != NiFpga_Status_Success)
        {
            return E_FPGA_INIT;
        }

        gNiFpgaInitialized = true;

        // Register finalize handler to be called at program exit.
        std::atexit (finalizeFpgaAtExit);
    }

    // No session open--create one.
    if (!gSessionOpen)
    {
        NiFpga_MergeStatus (&kStatusRet, NiFpga_Open (
            BIT_FILE_PATH NiFpga_IO_Bitfile,
            NiFpga_IO_Signature, "RIO0", 0, &gFpgaSession));

        if (kStatusRet != NiFpga_Status_Success)
        {
            return E_FPGA_SESSION_INIT;
        }

        gSessionOpen = true;

        // Sleep for a bit while the physical FPGA initializes.
        timespec timeToSleep = {};
        timeToSleep.tv_sec = 1;
        nanosleep (&timeToSleep, nullptr);
    }

    kSessionRet = gFpgaSession;
    return E_SUCCESS;
}

Error_t FPGASession::closeSession (NiFpga_Status& kStatusRet)
{
    if (!gSessionOpen)
    {
        return E_FPGA_NO_SESSION;
    }

    // Zero the status to clear garbage.
    kStatusRet = NiFpga_Status_Success;
    
    NiFpga_MergeStatus (&kStatusRet, NiFpga_Close (gFpgaSession, 0));
    gSessionOpen = false;
    if (kStatusRet != NiFpga_Status_Success)
    {
        return E_FPGA_CLOSE_SESSION;
    }

    return E_SUCCESS;
}

void finalizeFpgaAtExit ()
{
    if (gNiFpgaInitialized)
    {
        // Potential errors are disregarded. The program is ending, so if
        // closing and finalizing the session fails, there is nothing to be
        // done.
        NiFpga_Status status;
        FPGASession::closeSession (status);
        NiFpga_Finalize ();
    }
}