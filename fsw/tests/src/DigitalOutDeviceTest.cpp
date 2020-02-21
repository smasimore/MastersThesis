/* All #include statements should come before the CppUTest include */

#include <iostream>
#include <unistd.h>
#include <memory>

#include "DataVector.hpp"
#include "Errors.hpp"
#include "NiFpga.h"
#include "NiFpga_IO.h"
#include "DigitalOutDevice.hpp"

#include "TestHelpers.hpp"

/**
 * Path to bit file on sbRIO.
 */
#define BIT_FILE_PATH "/home/admin/FlightSoftware/"

/**
 * Initialize FPGA session and Data Vector.
 */
#define INIT_SESSION_AND_DV                                                   \
    NiFpga_Session session;                                                   \
    NiFpga_Status status = NiFpga_Initialize();                               \
    NiFpga_MergeStatus(&status, NiFpga_Open(                                  \
        BIT_FILE_PATH NiFpga_IO_Bitfile,                                      \
        NiFpga_IO_Signature, "RIO0", 0, &session));                           \
    CHECK_EQUAL (NiFpga_Status_Success, status);                              \
    TestHelpers::sleepMs (1000);                                              \
    DataVector::Config_t config =                                             \
    {                                                                         \
        {                                                                     \
            {DV_REG_TEST0,                                                    \
            {                                                                 \
                DV_ADD_BOOL  (  DV_ELEM_LED_CONTROL_VAL,      false        ), \
                DV_ADD_BOOL  (  DV_ELEM_LED_FEEDBACK_VAL,     false        ), \
            }},                                                               \
        }                                                                     \
    };                                                                        \
    std::shared_ptr<DataVector> pDv;                                          \
    CHECK_SUCCESS (DataVector::createNew (config, pDv));                       


TEST_GROUP (DigitalOutDeviceTest)
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

/* Test null State Vector pointer on init. */
TEST (DigitalOutDeviceTest, NullStateVector)
{
    INIT_SESSION_AND_DV;

    DigitalOutDevice::Config_t deviceConfig = 
    {
        DV_ELEM_LED_CONTROL_VAL, 
        DV_ELEM_LED_FEEDBACK_VAL, 
        static_cast<uint8_t> (DigitalOutDevice::MIN_PIN_NUMBER),
    };
    std::unique_ptr<DigitalOutDevice> pDigitalOutDevice;
    CHECK_ERROR (Device::createNew (session, nullptr, deviceConfig, 
                                    pDigitalOutDevice),
                 E_DATA_VECTOR_NULL); 
}

/* Test invalid pinNumber in config. */
TEST (DigitalOutDeviceTest, InvalidPinNumber)
{
    INIT_SESSION_AND_DV;

    DigitalOutDevice::Config_t deviceConfig = 
    {
        DV_ELEM_LED_CONTROL_VAL, 
        DV_ELEM_LED_FEEDBACK_VAL, 
        static_cast<uint8_t> (DigitalOutDevice::MAX_PIN_NUMBER + 1),
    };
    std::unique_ptr<DigitalOutDevice> pDigitalOutDevice;
    CHECK_ERROR (Device::createNew (session, pDv, deviceConfig, 
                                    pDigitalOutDevice),
                 E_OUT_OF_BOUNDS); 

    deviceConfig.pinNumber = static_cast<uint8_t> (
            DigitalOutDevice::MIN_PIN_NUMBER - 1);
    CHECK_ERROR (Device::createNew (session, pDv, deviceConfig, 
                                    pDigitalOutDevice),
                 E_OUT_OF_BOUNDS); 
}

/* Test invalid State Vector elements config. */
TEST (DigitalOutDeviceTest, InvalidSVElems)
{
    INIT_SESSION_AND_DV;

    // Verify invalid controlVal elem.
    DigitalOutDevice::Config_t deviceConfig = 
    {
        DV_ELEM_TEST0, 
        DV_ELEM_LED_FEEDBACK_VAL, 
        static_cast<uint8_t> (DigitalOutDevice::MAX_PIN_NUMBER),
    };
    std::unique_ptr<DigitalOutDevice> pDigitalOutDevice;
    CHECK_ERROR (Device::createNew (session, pDv, deviceConfig, 
                                    pDigitalOutDevice),
                 E_INVALID_ELEM); 

    // Verify invalid feedbackVal elem.
    deviceConfig.dvElemControlVal = DV_ELEM_LED_CONTROL_VAL;
    deviceConfig.dvElemFeedbackVal = DV_ELEM_TEST0;
    CHECK_ERROR (Device::createNew (session, pDv, deviceConfig, 
                                    pDigitalOutDevice),
                 E_INVALID_ELEM); 
}

/* Verify that after the device is initialized with a low control value, the 
   pin value is low.*/
TEST (DigitalOutDeviceTest, InitialStateLow)
{
    // 1) Initialize FPGA and DV. 
    INIT_SESSION_AND_DV;

    // 2) Initialize device.
    DigitalOutDevice::Config_t deviceConfig = 
    {
        DV_ELEM_LED_CONTROL_VAL, 
        DV_ELEM_LED_FEEDBACK_VAL, 
        5
    };
    std::unique_ptr<DigitalOutDevice> pDigitalOutDevice;
    CHECK_SUCCESS (Device::createNew (session, pDv, deviceConfig, pDigitalOutDevice)); 
    TestHelpers::sleepMs (1);

    // 3) Verify state is low after initializing.
    bool feedbackVal = false;
    NiFpga_MergeStatus (&status, 
                        NiFpga_ReadBool (session, 
                                         NiFpga_IO_IndicatorBool_inDIO5,
                                         (NiFpga_Bool*) &feedbackVal));
    CHECK_EQUAL (status, NiFpga_Status_Success);
    CHECK_EQUAL (false, feedbackVal);
}

/* Verify that after the device is initialized with a high control value, the
   pin value is high.*/
TEST (DigitalOutDeviceTest, InitialStateHigh)
{
    // 1) Initialize FPGA and DV. 
    INIT_SESSION_AND_DV;

    // 2) Set control value high before initializing device.
    CHECK_SUCCESS (pDv->write (DV_ELEM_LED_CONTROL_VAL, true));

    // 3) Initialize device.
    DigitalOutDevice::Config_t deviceConfig = 
    {
        DV_ELEM_LED_CONTROL_VAL, 
        DV_ELEM_LED_FEEDBACK_VAL, 
        5
    };
    std::unique_ptr<DigitalOutDevice> pDigitalOutDevice;
    CHECK_SUCCESS (Device::createNew (session, pDv, deviceConfig, pDigitalOutDevice)); 
    TestHelpers::sleepMs (1);

    // 4) Verify state is high after initializing.
    bool feedbackVal = false;
    NiFpga_MergeStatus (&status, 
                        NiFpga_ReadBool (session, 
                                         NiFpga_IO_IndicatorBool_inDIO5,
                                         (NiFpga_Bool*) &feedbackVal));
    CHECK_EQUAL (status, NiFpga_Status_Success);
    CHECK_EQUAL (true, feedbackVal);
}

/* Test successful initialization and run of device. */
TEST (DigitalOutDeviceTest, DigitalOutOn)
{
    // 1) Initialize FPGA and DV. 
    INIT_SESSION_AND_DV;

    // 2) Loop through valid pin numbers and verify functionality.
    for (uint8_t i = DigitalOutDevice::MIN_PIN_NUMBER; 
         i <= DigitalOutDevice::MAX_PIN_NUMBER; i++)
    {
        // 2a) Initialize device.
        DigitalOutDevice::Config_t deviceConfig = 
        {
            DV_ELEM_LED_CONTROL_VAL, 
            DV_ELEM_LED_FEEDBACK_VAL, 
            i
        };
        std::unique_ptr<DigitalOutDevice> pDigitalOutDevice;
        CHECK_SUCCESS (Device::createNew (session, pDv, deviceConfig, pDigitalOutDevice)); 

        // 2b) Verify starting state.
        bool controlVal = false;
        bool feedbackVal = false;
        pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal);
        pDv->read (DV_ELEM_LED_FEEDBACK_VAL, feedbackVal);
        CHECK_EQUAL (false, controlVal);
        CHECK_EQUAL (false, feedbackVal);

        // 2c) Run, sleep, then run. Sleep since pin can take some time before 
        // reflecting new output value. Expect the feedback value to remain false.
        CHECK_SUCCESS (pDigitalOutDevice->run ());
        TestHelpers::sleepMs (1);
        CHECK_SUCCESS (pDigitalOutDevice->run ());
        pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal);
        pDv->read (DV_ELEM_LED_FEEDBACK_VAL, feedbackVal);
        CHECK_EQUAL (false, controlVal);
        CHECK_EQUAL (false, feedbackVal);

        // 2d) Set controlVal to true and verify updated state.
        pDv->write (DV_ELEM_LED_CONTROL_VAL, true);
        CHECK_SUCCESS (pDigitalOutDevice->run ());
        TestHelpers::sleepMs (1);
        CHECK_SUCCESS (pDigitalOutDevice->run ());
        pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal);
        pDv->read (DV_ELEM_LED_FEEDBACK_VAL, feedbackVal);
        CHECK_EQUAL (true, controlVal);
        CHECK_EQUAL (true, feedbackVal);

        // 2e) Set controlVal to false and verify updated state.
        pDv->write (DV_ELEM_LED_CONTROL_VAL, false);
        CHECK_SUCCESS (pDigitalOutDevice->run ());
        TestHelpers::sleepMs (1);
        CHECK_SUCCESS (pDigitalOutDevice->run ());
        pDv->read (DV_ELEM_LED_CONTROL_VAL, controlVal);
        pDv->read (DV_ELEM_LED_FEEDBACK_VAL, feedbackVal);
        CHECK_EQUAL (false, controlVal);
        CHECK_EQUAL (false, feedbackVal);
    }

    // 3) Close and finalize FPGA session.
    NiFpga_MergeStatus (&status, NiFpga_Close (session, 0));
    NiFpga_MergeStatus (&status, NiFpga_Finalize ());
}
