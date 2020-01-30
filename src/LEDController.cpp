#include "Device.hpp"
#include "LEDController.hpp"

LEDController::LEDController (const LEDController::Config_t& kConf,
                              std::shared_ptr<DataVector> kPDataVector,
                              DataVectorElement_t kDvModeElem) :
    Controller (kPDataVector, kDvModeElem)
{
    this->mDvElemControlVal = kConf.dvElemControlVal;
}

Error_t LEDController::verifyConfig ()
{
    // Verify DV elems exist.
    Error_t ret = mPDataVector->elementExists (this->mDvElemControlVal);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    return E_SUCCESS;
}

Error_t LEDController::runEnabled ()
{
    return this->setLED (true);
}

Error_t LEDController::runSafed ()
{
    return this->setLED (false);
}

Error_t LEDController::setLED (bool kControlVal)
{
    Error_t err = E_SUCCESS;

    // Write control value to DV, which will inform the DIO device whether to
    // turn LED on or off.
    if (mPDataVector->write (this->mDvElemControlVal, kControlVal) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return err;
}
