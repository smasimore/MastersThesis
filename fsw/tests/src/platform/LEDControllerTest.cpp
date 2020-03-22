/* All #include statements should come before the CppUTest include */
#include "Errors.hpp"
#include "Log.hpp"
#include "LEDController.hpp"

#include "TestHelpers.hpp"

static DataVector::Config_t gDvConfig = 
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT8  ( DV_ELEM_LED_CONTROLLER_MODE,  MODE_SAFED ),
        DV_ADD_BOOL   ( DV_ELEM_LED_CONTROL_VAL,      false      ),
    }},
};

static LEDController::Config_t gValidLEDConfig = 
{
    DV_ELEM_LED_CONTROL_VAL,
};

TEST_GROUP (LEDControllerTest)
{

};

/* Test initialization of controller with a valid config. */
TEST (LEDControllerTest, InitValidConfig)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<LEDController> pLEDController = nullptr;
    CHECK_SUCCESS (Controller::createNew<LEDController> (
                       gValidLEDConfig, pDv,
                       DV_ELEM_LED_CONTROLLER_MODE,
                       pLEDController));
}

/* Test initialization of controller with an invalid config. */
TEST (LEDControllerTest, InitInvalidConfig)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<LEDController> pInvalid = nullptr;
    LEDController::Config_t config = 
    {
        DV_ELEM_TEST0,
    }; 
    CHECK_ERROR (Controller::createNew<LEDController> (
                     config, pDv,
                     DV_ELEM_LED_CONTROLLER_MODE,
                     pInvalid),
                 E_INVALID_ELEM);
    POINTERS_EQUAL (pInvalid.get (), nullptr);
}

/* Test running controller in ENABLED and SAFED modes. */
TEST (LEDControllerTest, Run)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<LEDController> pLEDController = nullptr;
    CHECK_SUCCESS (Controller::createNew<LEDController> (
                       gValidLEDConfig, pDv,
                       DV_ELEM_LED_CONTROLLER_MODE,
                       pLEDController));

    // Verify initial state.
    bool controlVal = false;
    CHECK_SUCCESS (pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (false, controlVal);

    // Expect this to call runSafed
    CHECK_SUCCESS (pLEDController->run ());
    CHECK_SUCCESS (pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (false, controlVal);

    // Expect this to call runEnabled
    CHECK_SUCCESS (pDv->write (DV_ELEM_LED_CONTROLLER_MODE, 
                               (uint8_t) MODE_ENABLED));
    CHECK_SUCCESS (pLEDController->run ());
    CHECK_SUCCESS (pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (true, controlVal);

    // Expect this to call runSafed
    CHECK_SUCCESS (pDv->write (DV_ELEM_LED_CONTROLLER_MODE, 
                               (uint8_t) MODE_SAFED));
    CHECK_SUCCESS (pLEDController->run ());
    CHECK_SUCCESS (pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal));
    CHECK_EQUAL (false, controlVal);
}
