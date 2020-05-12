/**
 * NOTE: This file contains basic tests for the AnalogInDevice class but does
 * not validate its analog read capabilities. This is instead tested by
 * fsw/scripts/src/AnalogInDeviceTest.cpp.
 */

#include "AnalogInDevice.hpp"
#include "FPGASession.hpp"
#include "TestHelpers.hpp"

/**
 * Initializes the FPGA session, Data Vector, test device pointer, and test
 * device config.
 */
#define INIT_TEST                                                              \
    NiFpga_Session session;                                                    \
    NiFpga_Status status;                                                      \
    CHECK_SUCCESS (FPGASession::getSession (session, status));                 \
    CHECK_EQUAL (NiFpga_Status_Success, status);                               \
    std::shared_ptr<DataVector> pDv = nullptr;                                 \
    CHECK_SUCCESS (DataVector::createNew (gDvConfig, pDv));                    \
    std::unique_ptr<AnalogInDevice> pDevice = nullptr;                         \
    AnalogInDevice::Config_t config = gDeviceConfig;

/**
 * DV config for test device.
 */
static DataVector::Config_t gDvConfig =
{
    // Region
    {DV_REG_TEST0,

    // Elements
    {
        DV_ADD_FLOAT  (DV_ELEM_TEST0, 0),
        DV_ADD_FLOAT  (DV_ELEM_TEST1, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST2, 0),
    }},
};

/**
 * Transfer function that always succeeds.
 *
 * @param   kV        Voltage.
 * @param   kEngrRet  Engineering unit.
 *
 * @ret     E_SUCCESS Always succeeds.
 */
static Error_t successTransferFunc (float kV, float& kEngrRet)
{
    kEngrRet = kV;
    return E_SUCCESS;
}

/**
 * Transfer function that generates an error.
 *
 * @param   kV           Voltage.
 * @param   kEngrRet     Engineering unit.
 *
 * @ret     E_TEST_ERROR Always returned.
 */
static Error_t errorTransferFunc (float kV, float& kEngrRet)
{
    return E_TEST_ERROR;
}

/**
 * Example valid device config.
 */
static AnalogInDevice::Config_t gDeviceConfig =
{
    DV_ELEM_TEST0,
    DV_ELEM_TEST1,
    AnalogInDevice::MIN_PIN_NUMBER,
    (AnalogInDevice::TransferFunc_t*) &successTransferFunc,
    AnalogInDevice::RANGE_10V,
    AnalogInDevice::MODE_DIFF,
};

TEST_GROUP (AnalogInDeviceTest)
{
};

/**
 * Tests invalid DV output elems in device config.
 */
TEST (AnalogInDeviceTest, InvalidOutputDvElem)
{
    INIT_TEST;

    // Voltage elem is wrong type.
    config.dvElemOutputVolts = DV_ELEM_TEST2;
    CHECK_ERROR (E_INVALID_ELEM, Device::createNew (session, pDv, config,
                                                    pDevice));

    // Voltage elem does not exist in DV.
    config.dvElemOutputVolts = DV_ELEM_TEST3;
    CHECK_ERROR (E_INVALID_ELEM, Device::createNew (session, pDv, config,
                                                    pDevice));

    // Engineering unit elem is wrong type.
    config.dvElemOutputVolts = gDeviceConfig.dvElemOutputVolts;
    config.dvElemOutputEngr = DV_ELEM_TEST2;
    CHECK_ERROR (E_INVALID_ELEM, Device::createNew (session, pDv, config,
                                                    pDevice));

    // Engineering unit elem does not exist in DV.
    config.dvElemOutputEngr = DV_ELEM_TEST3;
    CHECK_ERROR (E_INVALID_ELEM, Device::createNew (session, pDv, config,
                                                    pDevice));
}

/**
 * Tests out-of-bounds pin numbers in device config.
 */
TEST (AnalogInDeviceTest, InvalidPinNumber)
{
    INIT_TEST;

    // Pin number too low (this will underflow, but that's ok).
    config.pinNumber = (uint8_t) (AnalogInDevice::MIN_PIN_NUMBER - 1);
    CHECK_ERROR (E_OUT_OF_BOUNDS, Device::createNew (session, pDv, config,
                                                     pDevice));

    // Pin number too high.
    config.pinNumber = (uint8_t) (AnalogInDevice::MAX_PIN_NUMBER + 1);
    CHECK_ERROR (E_OUT_OF_BOUNDS, Device::createNew (session, pDv, config,
                                                     pDevice));
}

/**
 * Tests null transfer function pointer in device config.
 */
TEST (AnalogInDeviceTest, NullTransferFunc)
{
    INIT_TEST;

    config.pTransferFunc = nullptr;
    CHECK_ERROR (E_INVALID_POINTER, Device::createNew (session, pDv, config,
                                                       pDevice));
}

/**
 * Tests invalid input range and mode in device config.
 */
TEST (AnalogInDeviceTest, InvalidRangeOrMode)
{
    INIT_TEST;

    // Invalid range.
    config.range = AnalogInDevice::RANGE_LAST;
    CHECK_ERROR (E_INVALID_ENUM, Device::createNew (session, pDv, config,
                                                    pDevice));

    // Invalid mode.
    config.range = gDeviceConfig.range;
    config.mode = AnalogInDevice::MODE_LAST;
    CHECK_ERROR (E_INVALID_ENUM, Device::createNew (session, pDv, config,
                                                    pDevice));
}

/**
 * Tests configuring a differential device on an invalid pin.
 */
TEST (AnalogInDeviceTest, DiffInvalidPin)
{
    INIT_TEST;

    config.pinNumber = 8;
    CHECK_ERROR (E_PIN_NOT_CONFIGURED, Device::createNew (session, pDv, config,
                                                          pDevice));
}

/**
 * Tests creating device with valid config.
 */
TEST (AnalogInDeviceTest, ValidConfig)
{
    INIT_TEST;

    CHECK_SUCCESS (Device::createNew (session, pDv, config, pDevice));
    CHECK_SUCCESS (pDevice->run ());
}

/**
 * Tests that transfer function errors are surfaced.
 */
TEST (AnalogInDeviceTest, ErrorTransferFunc)
{
    INIT_TEST;

    config.pTransferFunc = (AnalogInDevice::TransferFunc_t*) &errorTransferFunc;
    CHECK_SUCCESS (Device::createNew (session, pDv, config, pDevice));
    CHECK_ERROR (E_TEST_ERROR, pDevice->run ());
}