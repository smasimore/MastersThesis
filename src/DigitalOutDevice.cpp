#include "DigitalOutDevice.hpp"
#include "NiFpga_IO.h"

const uint8_t DigitalOutDevice::MIN_PIN_NUMBER = 5;
const uint8_t DigitalOutDevice::MAX_PIN_NUMBER = 27;

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t DigitalOutDevice::run ()
{
    Error_t ret = updateFpgaControlValue ();
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Read current FPGA pin value. Note, there is some delay between writing
    // a new value to digital out and the value being reflected in the indicator
    // value. So immediately after a new write this value will often not match
    // the new written value.
    NiFpga_Status status = NiFpga_Status_Success;
    bool feedbackVal = false;
    NiFpga_MergeStatus (&status, 
                        NiFpga_ReadBool (mSession, 
                                         mFpgaIndicator, 
                                         (NiFpga_Bool*) &feedbackVal));
    if (status != NiFpga_Status_Success)
    {
        return E_FPGA_READ;
    }

    // Write current pin value to DV. Note, there is some delay between writing
    // a new value to digital out and the value being reflected in the indicator
    // value.
    if (mPDataVector->write (mDvElemFeedbackVal, feedbackVal) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

Error_t DigitalOutDevice::verifyConfig (Config_t& kConfig)
{
    // Verify pinNumber within bounds. sbRIO has 28 DIO pins.
    uint8_t pinNumber = kConfig.pinNumber;
    if (pinNumber > MAX_PIN_NUMBER || pinNumber < MIN_PIN_NUMBER)
    {
        return E_OUT_OF_BOUNDS;
    }

    // Verify elements are in DV.
    bool value = false;
    if (mPDataVector->read (kConfig.dvElemControlVal, value) != E_SUCCESS ||
        mPDataVector->read (kConfig.dvElemFeedbackVal, value) != E_SUCCESS)
    {
        return E_INVALID_ELEM;
    }

    return E_SUCCESS;
}

DigitalOutDevice::DigitalOutDevice (NiFpga_Session& kSession, 
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

    // Store config data in class globals.
    mDvElemControlVal = kConfig.dvElemControlVal;
    mDvElemFeedbackVal = kConfig.dvElemFeedbackVal;
    // Get pin config data.
    uint32_t fpgaOutputEnable = 0;
    switch (kConfig.pinNumber)
    {
        // DIO 0 - 4 not configured in FPGA bit file.
        case 5:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO5;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO5;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO5;
            break;
        case 6:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO6;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO6;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO6;
            break;
        case 7:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO7;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO7;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO7;
            break;
        case 8:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO8;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO8;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO8;
            break;
        case 9:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO9;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO9;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO9;
            break;
        case 10:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO10;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO10;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO10;
            break;
        case 11:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO11;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO11;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO11;
            break;
        case 12:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO12;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO12;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO12;
            break;
        case 13:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO13;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO13;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO13;
            break;
        case 14:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO14;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO14;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO14;
            break;
        case 15:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO15;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO15;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO15;
            break;
        case 16:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO16;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO16;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO16;
            break;
        case 17:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO17;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO17;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO17;
            break;
        case 18:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO18;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO18;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO18;
            break;
        case 19:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO19;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO19;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO19;
            break;
        case 20:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO20;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO20;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO20;
            break;
        case 21:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO21;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO21;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO21;
            break;
        case 22:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO22;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO22;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO22;
            break;
        case 23:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO23;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO23;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO23;
            break;
        case 24:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO24;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO24;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO24;
            break;
        case 25:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO25;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO25;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO25;
            break;
        case 26:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO26;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO26;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO26;
            break;
        case 27:
            mFpgaControl   = NiFpga_IO_ControlBool_outDIO27;
            mFpgaIndicator = NiFpga_IO_IndicatorBool_inDIO27;
            fpgaOutputEnable = NiFpga_IO_ControlBool_outputEnableDIO27;
            break;
        default:
            kRet = E_PIN_NOT_CONFIGURED;
            return;
    }

    kRet = updateFpgaControlValue ();
    if (kRet != E_SUCCESS)
    {
        return;
    }

    // Configure pin as output. This is done after setting the initial control
    // value so that the pin outputs the desired value as soon as it is set as
    // an output pin.
    NiFpga_Status status = NiFpga_Status_Success;
    NiFpga_MergeStatus (&status, NiFpga_WriteBool (kSession, fpgaOutputEnable,
                                                   NiFpga_True));
    if (status != NiFpga_Status_Success)
    {
        kRet = E_FPGA_WRITE;
        return;
    }

    kRet = E_SUCCESS;
}

/*************************** PRIVATE FUNCTIONS ********************************/

Error_t DigitalOutDevice::updateFpgaControlValue ()
{
    // Read output value from DV.
    bool outputVal = false;
    if (mPDataVector->read (mDvElemControlVal, outputVal) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Write output value to FPGA.
    NiFpga_Status status = NiFpga_Status_Success;
    NiFpga_MergeStatus (&status, 
                        NiFpga_WriteBool (mSession, 
                                          mFpgaControl, 
                                          static_cast<NiFpga_Bool> (outputVal)));
    if (status != NiFpga_Status_Success)
    {
        return E_FPGA_WRITE;
    }

    return E_SUCCESS;
}
