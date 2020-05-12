#include "AnalogInDevice.hpp"

/**
 * Array mapping pin numbers -> FPGA API analog input mode identifiers.
 */
static const int32_t AIN_MODE_ARR[] =
{
    NiFpga_IO_ControlU8_modeAI0,
    NiFpga_IO_ControlU8_modeAI1,
    NiFpga_IO_ControlU8_modeAI2,
    NiFpga_IO_ControlU8_modeAI3,
    NiFpga_IO_ControlU8_modeAI4,
    NiFpga_IO_ControlU8_modeAI5,
    NiFpga_IO_ControlU8_modeAI6,
    NiFpga_IO_ControlU8_modeAI7,
    NiFpga_IO_ControlU8_modeAI8,
    NiFpga_IO_ControlU8_modeAI9,
    NiFpga_IO_ControlU8_modeAI10,
    NiFpga_IO_ControlU8_modeAI11,
    NiFpga_IO_ControlU8_modeAI12,
    NiFpga_IO_ControlU8_modeAI13,
    NiFpga_IO_ControlU8_modeAI14,
    NiFpga_IO_ControlU8_modeAI15,
};

/**
 * Array mapping pin numbers -> FPGA API analog input range identifiers.
 */
static const int32_t AIN_RANGE_ARR[] =
{
    NiFpga_IO_ControlU8_rangeAI0,
    NiFpga_IO_ControlU8_rangeAI1,
    NiFpga_IO_ControlU8_rangeAI2,
    NiFpga_IO_ControlU8_rangeAI3,
    NiFpga_IO_ControlU8_rangeAI4,
    NiFpga_IO_ControlU8_rangeAI5,
    NiFpga_IO_ControlU8_rangeAI6,
    NiFpga_IO_ControlU8_rangeAI7,
    NiFpga_IO_ControlU8_rangeAI8,
    NiFpga_IO_ControlU8_rangeAI9,
    NiFpga_IO_ControlU8_rangeAI10,
    NiFpga_IO_ControlU8_rangeAI11,
    NiFpga_IO_ControlU8_rangeAI12,
    NiFpga_IO_ControlU8_rangeAI13,
    NiFpga_IO_ControlU8_rangeAI14,
    NiFpga_IO_ControlU8_rangeAI15,
};

/**
 * Array mapping pin numbers -> FPGA API analog input fxp resource identifiers.
 */
static const uint32_t AIN_FXP_RESOURCE_ARR[] =
{
    NiFpga_IO_IndicatorFxp_inputAI0_Resource,
    NiFpga_IO_IndicatorFxp_inputAI1_Resource,
    NiFpga_IO_IndicatorFxp_inputAI2_Resource,
    NiFpga_IO_IndicatorFxp_inputAI3_Resource,
    NiFpga_IO_IndicatorFxp_inputAI4_Resource,
    NiFpga_IO_IndicatorFxp_inputAI5_Resource,
    NiFpga_IO_IndicatorFxp_inputAI6_Resource,
    NiFpga_IO_IndicatorFxp_inputAI7_Resource,
    NiFpga_IO_IndicatorFxp_inputAI8_Resource,
    NiFpga_IO_IndicatorFxp_inputAI9_Resource,
    NiFpga_IO_IndicatorFxp_inputAI10_Resource,
    NiFpga_IO_IndicatorFxp_inputAI11_Resource,
    NiFpga_IO_IndicatorFxp_inputAI12_Resource,
    NiFpga_IO_IndicatorFxp_inputAI13_Resource,
    NiFpga_IO_IndicatorFxp_inputAI14_Resource,
    NiFpga_IO_IndicatorFxp_inputAI15_Resource,
};

/**
 * Array mapping pin numbers -> FPGA API analog input fxp type info identifiers.
 */
static const NiFpga_FxpTypeInfo AIN_FXP_INFO_ARR[] =
{
    NiFpga_IO_IndicatorFxp_inputAI0_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI1_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI2_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI3_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI4_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI5_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI6_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI7_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI8_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI9_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI10_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI11_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI12_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI13_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI14_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI15_TypeInfo,
};

/****************************** PUBLIC MEMBERS ********************************/

const uint8_t AnalogInDevice::MIN_PIN_NUMBER = 0;
const uint8_t AnalogInDevice::MAX_PIN_NUMBER = 15;

AnalogInDevice::AnalogInDevice (NiFpga_Session& kSession,
                                std::shared_ptr<DataVector> kPDataVector,
                                Config_t& kConfig,
                                Error_t& kRet) :
    Device (kSession, kPDataVector)
{
    // Verify config.
    kRet = this->verifyConfig (kConfig);
    if (kRet != E_SUCCESS)
    {
        return;
    }

    mDvElemOutputVolts = kConfig.dvElemOutputVolts;
    mDvElemOutputEngr = kConfig.dvElemOutputEngr;
    mPTransferFunc = kConfig.pTransferFunc;

    // Set the voltage range.
    NiFpga_Status status = NiFpga_Status_Success;
    NiFpga_MergeStatus (&status, NiFpga_WriteU8 (mSession,
                        AIN_RANGE_ARR[kConfig.pinNumber],
                        (uint8_t) (kConfig.range)));
    if (status != NiFpga_Status_Success)
    {
        kRet = E_FPGA_WRITE;
        return;
    }

    // Set the input mode.
    NiFpga_MergeStatus (&status, NiFpga_WriteU8 (mSession,
                        AIN_MODE_ARR[kConfig.pinNumber],
                        (uint8_t) (kConfig.mode)));
    if (status != NiFpga_Status_Success)
    {
        kRet = E_FPGA_WRITE;
        return;
    }

    // Assign the fxp resource and type info IDs for converting input signals
    // from U32s to floating point voltages.
    mFxpResourceId = AIN_FXP_RESOURCE_ARR[kConfig.pinNumber];
    mFxpTypeInfoId = AIN_FXP_INFO_ARR[kConfig.pinNumber];
}

Error_t AnalogInDevice::verifyConfig (Config_t& kConfig)
{
    // Check that DV output elem exists and is correct type.
    float value = 0;
    if (mPDataVector->read (kConfig.dvElemOutputVolts, value) != E_SUCCESS ||
        mPDataVector->read (kConfig.dvElemOutputEngr,  value) != E_SUCCESS)
    {
        return E_INVALID_ELEM;
    }
    
    // Check that pin number is in range.
    if (kConfig.pinNumber < MIN_PIN_NUMBER ||
        kConfig.pinNumber > MAX_PIN_NUMBER)
    {
        return E_OUT_OF_BOUNDS;
    }

    // Check that transfer function pointer is non-null.
    if (kConfig.pTransferFunc == nullptr)
    {
        return E_INVALID_POINTER;
    }

    // Check that voltage range is valid.
    if (kConfig.range >= Range_t::RANGE_LAST)
    {
        return E_INVALID_ENUM;
    }

    // Check that input mode is valid.
    if (kConfig.mode >= Mode_t::MODE_LAST)
    {
        return E_INVALID_ENUM;
    }

    // Check that differential mode is only used on pins < 8.
    if (kConfig.mode == MODE_DIFF && kConfig.pinNumber >= 8)
    {
        return E_PIN_NOT_CONFIGURED;
    }

    return E_SUCCESS;
}

Error_t AnalogInDevice::run ()
{
    // Read fxp value.
    NiFpga_Status status = NiFpga_Status_Success;
    uint32_t fxpVal = 0;
    NiFpga_MergeStatus (&status, NiFpga_ReadU32 (mSession, mFxpResourceId,
                                                 &fxpVal));
    if (status != NiFpga_Status_Success)
    {
        return E_FPGA_READ;
    }

    // Convert fxp to floating point voltage.
    float voltage = NiFpga_ConvertFromFxpToFloat (mFxpTypeInfoId, fxpVal);

    // Write voltage to Data Vector.
    if (mPDataVector->write (mDvElemOutputVolts, voltage) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    // Convert voltage to engineering unit.
    float engrUnit = 0;
    Error_t convertErr = (*mPTransferFunc) (voltage, engrUnit);
    if (convertErr != E_SUCCESS)
    {
        return convertErr;
    }

    // Write engineering unit to Data Vector.
    if (mPDataVector->write (mDvElemOutputEngr, engrUnit) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}