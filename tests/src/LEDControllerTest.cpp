/* All #include statements should come before the CppUTest include */
#include "Errors.h"
#include "Log.hpp"
#include "LEDController.hpp"

#include "TestHelpers.hpp"

static StateVector::StateVectorConfig_t gSvConfig = 
{
    {SV_REG_TEST0,
    {
        SV_ADD_UINT8 (SV_ELEM_LED_CONTROLLER_MODE, Controller::Mode_t::SAFED),
        SV_ADD_BOOL  (SV_ELEM_LED_CONTROL_VAL,     false                    ),
    }},
};

static LEDController::Config_t gValidLEDConfig = 
{
    SV_ELEM_LED_CONTROL_VAL,
};

TEST_GROUP (LEDControllerTest)
{

};

/* Test initialization of controller with a valid config. */
TEST (LEDControllerTest, InitValidConfig)
{
    INIT_STATE_VECTOR (gSvConfig);

    std::unique_ptr<LEDController> pLEDController = nullptr;
    CHECK_SUCCESS (Controller::createNew<LEDController> (
                       gValidLEDConfig, pSv,
                       SV_ELEM_LED_CONTROLLER_MODE,
                       pLEDController));
}

/* Test initialization of controller with an invalid config. */
TEST (LEDControllerTest, InitInvalidConfig)
{
    INIT_STATE_VECTOR (gSvConfig);

    std::unique_ptr<LEDController> pInvalid = nullptr;
    LEDController::Config_t config = 
    {
        SV_ELEM_TEST0,
    }; 
    CHECK_ERROR (Controller::createNew<LEDController> (
                     config, pSv,
                     SV_ELEM_LED_CONTROLLER_MODE,
                     pInvalid),
                 E_INVALID_ELEM);
    POINTERS_EQUAL (pInvalid.get (), nullptr);
}

/* Test running controller in ENABLED and SAFED modes. */
TEST (LEDControllerTest, Run)
{
    INIT_STATE_VECTOR (gSvConfig);

    std::unique_ptr<LEDController> pLEDController = nullptr;
    CHECK_SUCCESS (Controller::createNew<LEDController> (
                       gValidLEDConfig, pSv,
                       SV_ELEM_LED_CONTROLLER_MODE,
                       pLEDController));

    // Verify initial state.
    bool controlVal = false;
    CHECK_SUCCESS (pSv->read (SV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (false, controlVal);

    // Expect this to call runSafed
    CHECK_SUCCESS (pLEDController->run ());
    CHECK_SUCCESS (pSv->read (SV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (false, controlVal);

    // Expect this to call runEnabled
    CHECK_SUCCESS (pSv->write (SV_ELEM_LED_CONTROLLER_MODE, 
                               (uint8_t) Controller::Mode_t::ENABLED));
    CHECK_SUCCESS (pLEDController->run ());
    CHECK_SUCCESS (pSv->read (SV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (true, controlVal);

    // Expect this to call runSafed
    CHECK_SUCCESS (pSv->write (SV_ELEM_LED_CONTROLLER_MODE, 
                               (uint8_t) Controller::Mode_t::SAFED));
    CHECK_SUCCESS (pLEDController->run ());
    CHECK_SUCCESS (pSv->read (SV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (false, controlVal);
}
