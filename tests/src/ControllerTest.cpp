/* All #include statements should come before the CppUTest include */
#include "Errors.h"
#include "Log.hpp"
#include "TestController.hpp"

#include "CppUTest/TestHarness.h"

/* Test globals. */
Log *PExpectedLog = nullptr;
Log *PTestLog = nullptr;
std::unique_ptr<TestController> PTest = nullptr;

TEST_GROUP(ControllerTest)
{
    void setup()
    {
        Error_t ret;

        // Create TestController and logs.
        struct TestController::TestConfig config; 
        config.valid = true;
        ret = Controller::createNew<TestController, 
                                    struct TestController::TestConfig> (config, 
                                                                        PTest);
        CHECK_EQUAL (E_SUCCESS, ret);
        PExpectedLog = new Log (ret);
        CHECK_EQUAL (E_SUCCESS, ret);
        PTestLog = new Log (ret);
        CHECK_EQUAL (E_SUCCESS, ret);
    }

    void teardown()
    {
        // Clean up TestController and logs.
        PTest.reset (nullptr);
        delete PExpectedLog;
        delete PTestLog;
    }
};

/* Test initialization of controller with a valid config. */
TEST(ControllerTest, InitValidConfig)
{
    Error_t ret;
    Controller::Mode_t stateRet;

    ret = PTest->getMode (stateRet);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (Controller::Mode_t::SAFED, stateRet);
}

/* Test initialization of controller with an invalid config. */
TEST(ControllerTest, InitInvalidConfig)
{
    Error_t ret;

    // Create invalid config.
    std::unique_ptr<TestController> pInvalid = nullptr;
    struct TestController::TestConfig config; 
    config.valid = false;
    ret = Controller::createNew<TestController, 
                                struct TestController::TestConfig> (config, 
                                                                    pInvalid);

    // Verify failure.
    CHECK_EQUAL (ret, E_INVALID_CONFIG);
    POINTERS_EQUAL (pInvalid.get (), nullptr);
}

/* Test mode setters and getters. */
TEST(ControllerTest, SetMode)
{
    Controller::Mode_t modeRet;
    Error_t ret;

    // Controller should initialize SAFED.
    ret = PTest->getMode (modeRet);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (Controller::Mode_t::SAFED, modeRet);

    // Set mode as ENABLED and verify.
    ret = PTest->setMode (Controller::Mode_t::ENABLED);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = PTest->getMode (modeRet);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (Controller::Mode_t::ENABLED, modeRet);
}

/* Test running controller in ENABLED and SAFED modes. */
TEST(ControllerTest, Run)
{
    Error_t ret;

    // Expect this to call runSafed
    ret = PTest->run ();
    CHECK_EQUAL (E_SUCCESS, ret);

    // Expect this to call runEnabled
    PTest->setMode (Controller::Mode_t::ENABLED);
    ret = PTest->run ();
    CHECK_EQUAL (E_SUCCESS, ret);

    // Expect this to call runSafed
    PTest->setMode (Controller::Mode_t::SAFED);
    ret = PTest->run ();
    CHECK_EQUAL (E_SUCCESS, ret);

    // Build expected log.
    PExpectedLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_SAFED, 0);
    PExpectedLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_ENABLED, 0);
    PExpectedLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_SAFED, 0);

    // Verify.
    bool logsEqual = false; 
    ret = Log::verify (*PExpectedLog, *PTestLog, logsEqual);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_TRUE (logsEqual); 
}
