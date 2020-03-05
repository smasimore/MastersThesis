/**
 * Note 1: interaction between devices and the global session should be tested
 * in their own test suites (e.g. DigitalOutDeviceTest.cpp) and ONLY the global
 * session should be used. Such tests should close the session once complete
 * to ensure that this test (FpgaGlobalTest.cpp) begins with no global session
 * open.
 *
 * Note 2: this test does not test that the FPGA API is finalized on program
 * end. This can be validated with the recovery igniter test procedures
 * (RecoveryIgniterTest.cpp).
 */

#include "Fpga.hpp"
#include "TestHelpers.hpp"

/**
 * Global session and status used in tests.
 */
NiFpga_Session gSession;
NiFpga_Status gStatus;

TEST_GROUP (FpgaGlobalTest)
{
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