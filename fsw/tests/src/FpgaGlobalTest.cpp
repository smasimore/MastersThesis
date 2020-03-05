/**
 * Note: interaction between devices and the global session should be tested in
 * their own test suites (e.g. DigitalOutDeviceTest.cpp) and ONLY the global
 * session should be used. Such tests should close the session once complete
 * to ensure that this test begins with no global session open.
 */

#include "Fpga.hpp"
#include "TestHelpers.hpp"

static NiFpga_Session gSession;
static NiFpga_Status gStatus;

TEST_GROUP (FpgaGlobalTest)
{
    /**
     * Turn off memory leak detection due to undiagnosed memory leak caused
     * by FPGA C API usage. This is a known NI issue and will only cause memory 
     * issues in production code if the FPGA is initialized more than once.
     *
     * http://www.ni.com/product-documentation/55093/en/#660205_by_Date
     */
    void setup ()
    {
        MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
    }

    /**
     * Turn memory leak detection back on.
     */
    void teardown()
    {
        MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    }
};

/**
 * Global FPGA session and status can be accessed correctly. Correct errors are
 * generated when trying to close or get status of session when none exists.
 */
TEST (FpgaGlobalTest, GetSessionAndStatus)
{
    // No session open; trying to close or get status errs.
    CHECK_ERROR (E_FPGA_NO_SESSION, Fpga::getStatus (gStatus));
    CHECK_ERROR (E_FPGA_NO_SESSION, Fpga::closeSession ());

    // Create a new session.
    CHECK_SUCCESS (Fpga::getSession (gSession));
    CHECK_SUCCESS (Fpga::getStatus (gStatus));
    CHECK_EQUAL (NiFpga_Status_Success, gStatus);

    // Getting another session returns the same session.
    NiFpga_Session oldSession = gSession;
    CHECK_SUCCESS (Fpga::getSession (gSession));
    CHECK_EQUAL (oldSession, gSession);

    // Close the session.
    CHECK_SUCCESS (Fpga::closeSession ());

    // No session open, closing or status queries again fail.
    CHECK_ERROR (E_FPGA_NO_SESSION, Fpga::getStatus (gStatus));
    CHECK_ERROR (E_FPGA_NO_SESSION, Fpga::closeSession ());
}