/* All #include statements should come before the CppUTest include */
#include "Errors.h"
#include "Log.hpp"
#include "TestController.hpp"

#include "TestHelpers.hpp"

/* Test globals. */
Log *gPExpectedLog = nullptr;
Log *gPTestLog = nullptr;

static DataVector::Config_t gDvConfig = 
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT8 (DV_ELEM_TEST_CONTROLLER_MODE, Controller::Mode_t::SAFED),
    }},
};

TEST_GROUP (ControllerTest)
{
    void setup()
    {
        // Create logs.
        
        Error_t ret = E_SUCCESS;
        gPExpectedLog = new Log (ret);
        CHECK_EQUAL (E_SUCCESS, ret);
        gPTestLog = new Log (ret);
        CHECK_EQUAL (E_SUCCESS, ret);
    }

    void teardown()
    {
        // Clean up logs.
        delete gPExpectedLog;
        delete gPTestLog;
    }
};

/* Test initialization of controller with a valid config. */
TEST (ControllerTest, InitValidConfig)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<TestController> pTestController = nullptr;
    TestController::Config_t conConfig = {true};
    CHECK_SUCCESS (Controller::createNew<TestController> (
                       conConfig, pDv,
                       DV_ELEM_TEST_CONTROLLER_MODE,
                       pTestController));

    Controller::Mode_t stateRet;
    CHECK_SUCCESS (pTestController->getMode (stateRet));
    CHECK_EQUAL (Controller::Mode_t::SAFED, stateRet);
}

/* Test initialization of controller with an invalid config. */
TEST (ControllerTest, InitInvalidConfig)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Create invalid config.
    std::unique_ptr<TestController> pInvalid = nullptr;
    TestController::Config_t config = {false}; 
    CHECK_ERROR (Controller::createNew<TestController> (
                     config, pDv,
                     DV_ELEM_TEST_CONTROLLER_MODE,
                     pInvalid),
                 E_OUT_OF_BOUNDS);
    POINTERS_EQUAL (pInvalid.get (), nullptr);
}

/* Test initialization of controller with a null DV. */
TEST (ControllerTest, InitInvalidDV)
{
    std::unique_ptr<TestController> pInvalid = nullptr;
    TestController::Config_t config = {true}; 
    CHECK_ERROR (Controller::createNew<TestController> (
                     config, nullptr,
                     DV_ELEM_TEST_CONTROLLER_MODE,
                     pInvalid),
                 E_DATA_VECTOR_NULL);
    POINTERS_EQUAL (pInvalid.get (), nullptr);
}

/* Test initialization of controller with an invalid DV elem. */
TEST (ControllerTest, InitInvalidDVElem)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<TestController> pInvalid = nullptr;
    TestController::Config_t config = {true}; 
    CHECK_ERROR (Controller::createNew<TestController> (
                     config, pDv,
                     DV_ELEM_RCS_CONTROLLER_MODE,
                     pInvalid),
                 E_INVALID_ELEM);
    POINTERS_EQUAL (pInvalid.get (), nullptr);
}

/* Test mode setters and getters. */
TEST (ControllerTest, SetMode)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<TestController> pTestController = nullptr;
    TestController::Config_t conConfig = {true};
    CHECK_SUCCESS (Controller::createNew<TestController> (
                       conConfig, pDv,
                       DV_ELEM_TEST_CONTROLLER_MODE,
                       pTestController));

    // Controller should initialize SAFED.
    Controller::Mode_t modeRet;
    CHECK_SUCCESS (pTestController->getMode (modeRet));
    CHECK_EQUAL (Controller::Mode_t::SAFED, modeRet);

    // Set mode as ENABLED and verify.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST_CONTROLLER_MODE, 
                               (uint8_t) Controller::Mode_t::ENABLED));
    CHECK_SUCCESS (pTestController->getMode (modeRet));
    CHECK_EQUAL (Controller::Mode_t::ENABLED, modeRet);
}

/* Test running controller in ENABLED and SAFED modes. */
TEST (ControllerTest, Run)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<TestController> pTestController = nullptr;
    TestController::Config_t conConfig = {true};
    CHECK_SUCCESS (Controller::createNew<TestController> (
                       conConfig, pDv,
                       DV_ELEM_TEST_CONTROLLER_MODE,
                       pTestController));

    // Expect this to call runSafed
    CHECK_SUCCESS (pTestController->run ());

    // Expect this to call runEnabled
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST_CONTROLLER_MODE, 
                               (uint8_t) Controller::Mode_t::ENABLED));
    CHECK_SUCCESS (pTestController->run ());

    // Expect this to call runSafed
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST_CONTROLLER_MODE, 
                               (uint8_t) Controller::Mode_t::SAFED));
    CHECK_SUCCESS (pTestController->run ());

    // Build expected log.
    gPExpectedLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_SAFED, 0);
    gPExpectedLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_ENABLED, 0);
    gPExpectedLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_SAFED, 0);

    // Verify.
    bool logsEqual = false; 
    CHECK_SUCCESS (Log::verify (*gPExpectedLog, *gPTestLog, logsEqual));
    CHECK_TRUE (logsEqual); 
}
