#include <cstdlib>

#include "Fpga.hpp"

/**
 * Parent directory of FPGA bit file on sbRIO.
 */
#define BIT_FILE_PATH "/home/admin/FlightSoftware/"

/**
 * Global session, accessible only through Fpga::getSession.
 */
NiFpga_Session gFpgaSession;

/**
 * Global status, accessible only through Fpga::getStatus.
 */
NiFpga_Status gFpgaStatus;

/**
 * Whether or not a session is currently open.
 */
bool gSessionOpen = false;

/**
 * Whether or not the FPGA API has been initialized.
 */
bool gNiFpgaInitialized = false;

/**
 * Called at program exit to close global session and finalize the FPGA API.
 */
void finalizeFpgaAtExit ();

Error_t Fpga::getSession (NiFpga_Session& kSessionRet)
{
    // First call--initialize the FPGA API.
    if (!gNiFpgaInitialized)
    {
        gFpgaStatus = NiFpga_Initialize ();
        
        if (gFpgaStatus != NiFpga_Status_Success)
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
        NiFpga_MergeStatus (&gFpgaStatus, NiFpga_Open (
            BIT_FILE_PATH NiFpga_IO_Bitfile,
            NiFpga_IO_Signature, "RIO0", 0, &gFpgaSession));

        if (gFpgaStatus != NiFpga_Status_Success)
        {
            // Session init failed.
            return E_FPGA_SESSION_INIT;
        }

        gSessionOpen = true;
    }

    kSessionRet = gFpgaSession;
    return E_SUCCESS;
}

Error_t Fpga::getStatus (NiFpga_Status& kRet)
{
    if (!gSessionOpen)
    {
        return E_FPGA_NO_SESSION;
    }

    kRet = gFpgaStatus;
    return E_SUCCESS;
}

Error_t Fpga::closeSession ()
{
    if (!gSessionOpen)
    {
        return E_FPGA_NO_SESSION;
    }

    NiFpga_MergeStatus (&gFpgaStatus, NiFpga_Close (gFpgaSession, 0));
    gSessionOpen = false;

    return E_SUCCESS;
}

void finalizeFpgaAtExit ()
{
    if (gNiFpgaInitialized)
    {
        // Potential errors are disregarded. The program is ending, so if
        // closing and finalizing the session fails, there is nothing to be
        // done.
        Fpga::closeSession ();
        NiFpga_MergeStatus (&gFpgaStatus, NiFpga_Finalize ());
    }
}