/**
 * Note 1: interaction between devices and the global session should be tested
 * in their own test suites (e.g. DigitalOutDeviceTest.cpp) and ONLY the global
 * session should be used. Such tests should close the session once complete
 * to ensure that this test (FpgaGlobalTest.cpp) begins with no global session
 * open.
 *
 * Note 2: this test does not verify that the FPGA API is finalized on program
 * end. This can be validated with recovery igniter test procedures 03 and 04,
 * detailed in RecoveryIgniterTest.cpp.
 */

#include "FPGASession.hpp"
#include "TestHelpers.hpp"

TEST_GROUP (FPGASessionTest)
{
};

/**
 * Global FPGA session and status can be accessed correctly. Correct errors are
 * generated when trying to close or get status of session when none exists.
 */
TEST (FPGASessionTest, GetSessionAndStatus)
{
    NiFpga_Session session;
    NiFpga_Status status;

    // No session open; trying to close errs.
    CHECK_ERROR (E_FPGA_NO_SESSION, FPGASession::closeSession (status));

    // Create a new session.
    CHECK_SUCCESS (FPGASession::getSession (session, status));
    CHECK_EQUAL (NiFpga_Status_Success, status);

    // Getting another session returns the same session.
    NiFpga_Session oldSession = session;
    CHECK_SUCCESS (FPGASession::getSession (session, status));
    CHECK_EQUAL (oldSession, session);

    // Close the session.
    CHECK_SUCCESS (FPGASession::closeSession (status));
    CHECK_EQUAL (NiFpga_Status_Success, status);

    // No session open, closing fails.
    CHECK_ERROR (E_FPGA_NO_SESSION, FPGASession::closeSession (status));
}