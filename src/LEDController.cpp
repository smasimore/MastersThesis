#include "Device.hpp"
#include "LEDController.hpp"

LEDController::LEDController (const LEDController::Config_t& kConf,
                              std::shared_ptr<StateVector> kPStateVector,
                              StateVectorElement_t kSvModeElem) :
    Controller (kPStateVector, kSvModeElem)
{
    this->mSvElemControlVal = kConf.svElemControlVal;
}

Error_t LEDController::verifyConfig ()
{
    // Verify SV elems exist.
    Error_t ret = mPStateVector->elementExists (this->mSvElemControlVal);
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

    // Write control value to SV, which will inform the DIO device whether to
    // turn LED on or off.
    if (mPStateVector->write (this->mSvElemControlVal, kControlVal) != E_SUCCESS)
    {
        return E_STATE_VECTOR_WRITE;
    }

    return err;
}
