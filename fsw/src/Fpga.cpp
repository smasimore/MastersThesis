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

Error_t Fpga::getSession (NiFpga_Session& kSessionRet)
{
    // No session open--create one.
    if (!gSessionOpen)
    {
        gFpgaStatus = NiFpga_Initialize ();
        NiFpga_MergeStatus (&gFpgaStatus, NiFpga_Open (
            BIT_FILE_PATH NiFpga_IO_Bitfile,
            NiFpga_IO_Signature, "RIO0", 0, &gFpgaSession));

        if (gFpgaStatus != NiFpga_Status_Success)
        {
            // FPGA init failed.
            return E_FPGA_INIT;
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
    NiFpga_MergeStatus (&gFpgaStatus, NiFpga_Finalize ());
    gSessionOpen = false;

    return E_SUCCESS;
}
