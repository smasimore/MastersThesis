/**
 * FPGA API constants for use in scripts.
 */

#ifndef FPGA_CONSTANTS_HPP
#define FPGA_CONSTANTS_HPP

/**
 * Analog input mode and range constants.
 */
static const uint8_t AI_MODE_DIFF = 0;
static const uint8_t AI_MODE_RSE  = 1;
static const uint8_t AI_RANGE_10V = 0;
static const uint8_t AI_RANGE_5V  = 1;
static const uint8_t AI_RANGE_2V  = 2;
static const uint8_t AI_RANGE_1V  = 3;

/**
 * Vector mapping analog input range constants -> value in volts.
 */
static const std::vector<float> AI_RANGES_V = {
    10,
    5,
    2,
    1,
};

/**
 * # of analog out pins.
 */
static const uint32_t NUM_AOUT_PINS = 4;

/**
 * Vector of digital output enable identifiers. Currently only DIO 5 - 27 are
 * configured in the generic FPGA bit file.
 */
static const std::vector<int32_t> DOUT_ENABLE_VEC = {
    NiFpga_IO_ControlBool_outputEnableDIO5,
    NiFpga_IO_ControlBool_outputEnableDIO6,
    NiFpga_IO_ControlBool_outputEnableDIO7,
    NiFpga_IO_ControlBool_outputEnableDIO8,
    NiFpga_IO_ControlBool_outputEnableDIO9,
    NiFpga_IO_ControlBool_outputEnableDIO10,
    NiFpga_IO_ControlBool_outputEnableDIO11,
    NiFpga_IO_ControlBool_outputEnableDIO12,
    NiFpga_IO_ControlBool_outputEnableDIO13,
    NiFpga_IO_ControlBool_outputEnableDIO14,
    NiFpga_IO_ControlBool_outputEnableDIO15,
    NiFpga_IO_ControlBool_outputEnableDIO16,
    NiFpga_IO_ControlBool_outputEnableDIO17,
    NiFpga_IO_ControlBool_outputEnableDIO18,
    NiFpga_IO_ControlBool_outputEnableDIO19,
    NiFpga_IO_ControlBool_outputEnableDIO20,
    NiFpga_IO_ControlBool_outputEnableDIO21,
    NiFpga_IO_ControlBool_outputEnableDIO22,
    NiFpga_IO_ControlBool_outputEnableDIO23,
    NiFpga_IO_ControlBool_outputEnableDIO24,
    NiFpga_IO_ControlBool_outputEnableDIO25,
    NiFpga_IO_ControlBool_outputEnableDIO26,
    NiFpga_IO_ControlBool_outputEnableDIO27,
};

/**
 * Vector of digital input indicator identifiers.
 */
static const std::vector<int32_t> DIN_INDICATOR_VEC =
{
    NiFpga_IO_IndicatorBool_inDIO5,
    NiFpga_IO_IndicatorBool_inDIO6,
    NiFpga_IO_IndicatorBool_inDIO7,
    NiFpga_IO_IndicatorBool_inDIO8,
    NiFpga_IO_IndicatorBool_inDIO9,
    NiFpga_IO_IndicatorBool_inDIO10,
    NiFpga_IO_IndicatorBool_inDIO11,
    NiFpga_IO_IndicatorBool_inDIO12,
    NiFpga_IO_IndicatorBool_inDIO13,
    NiFpga_IO_IndicatorBool_inDIO14,
    NiFpga_IO_IndicatorBool_inDIO15,
    NiFpga_IO_IndicatorBool_inDIO16,
    NiFpga_IO_IndicatorBool_inDIO17,
    NiFpga_IO_IndicatorBool_inDIO18,
    NiFpga_IO_IndicatorBool_inDIO19,
    NiFpga_IO_IndicatorBool_inDIO20,
    NiFpga_IO_IndicatorBool_inDIO21,
    NiFpga_IO_IndicatorBool_inDIO22,
    NiFpga_IO_IndicatorBool_inDIO23,
    NiFpga_IO_IndicatorBool_inDIO24,
    NiFpga_IO_IndicatorBool_inDIO25,
    NiFpga_IO_IndicatorBool_inDIO26,
    NiFpga_IO_IndicatorBool_inDIO27,
};

/**
 * Vector of analog input mode identifiers.
 */
static const std::vector<int32_t> AIN_MODE_VEC =
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
 * Vector of analog input range identifiers.
 */
static const std::vector<int32_t> AIN_RANGE_VEC =
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
 * Vector of analog input fxp resource identifiers.
 */
static const std::vector<uint32_t> AIN_FXP_RESOURCE_VEC =
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
 * Vector of analog input fxp type info identifiers.
 */
static const std::vector<NiFpga_FxpTypeInfo> AIN_FXP_INFO_VEC =
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

/**
 * Digital out control indicators.
 */
static const std::vector<int32_t> DOUT_CONTROL_VEC =
{
    NiFpga_IO_ControlBool_outDIO5,
    NiFpga_IO_ControlBool_outDIO6,
    NiFpga_IO_ControlBool_outDIO7,
    NiFpga_IO_ControlBool_outDIO8,
    NiFpga_IO_ControlBool_outDIO9,
    NiFpga_IO_ControlBool_outDIO10,
    NiFpga_IO_ControlBool_outDIO11,
    NiFpga_IO_ControlBool_outDIO12,
    NiFpga_IO_ControlBool_outDIO13,
    NiFpga_IO_ControlBool_outDIO14,
    NiFpga_IO_ControlBool_outDIO15,
    NiFpga_IO_ControlBool_outDIO16,
    NiFpga_IO_ControlBool_outDIO17,
    NiFpga_IO_ControlBool_outDIO18,
    NiFpga_IO_ControlBool_outDIO19,
    NiFpga_IO_ControlBool_outDIO20,
    NiFpga_IO_ControlBool_outDIO21,
    NiFpga_IO_ControlBool_outDIO22,
    NiFpga_IO_ControlBool_outDIO23,
    NiFpga_IO_ControlBool_outDIO24,
    NiFpga_IO_ControlBool_outDIO25,
    NiFpga_IO_ControlBool_outDIO26,
    NiFpga_IO_ControlBool_outDIO27,
};

/**
 * Analog out fxp resource indicators.
 */
static const std::vector<uint32_t> AOUT_FXP_RESOURCE_VEC =
{
    NiFpga_IO_ControlFxp_outputAO0_Resource,
    NiFpga_IO_ControlFxp_outputAO1_Resource,
    NiFpga_IO_ControlFxp_outputAO2_Resource,
    NiFpga_IO_ControlFxp_outputAO3_Resource,
};

/**
 * Analog out fxp type info used to convert fxp to float.
 */
static const std::vector<NiFpga_FxpTypeInfo> AOUT_FXP_INFO_VEC =
{
    NiFpga_IO_ControlFxp_outputAO0_TypeInfo,
    NiFpga_IO_ControlFxp_outputAO1_TypeInfo,
    NiFpga_IO_ControlFxp_outputAO2_TypeInfo,
    NiFpga_IO_ControlFxp_outputAO3_TypeInfo,
};

#endif