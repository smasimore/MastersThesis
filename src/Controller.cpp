#include "Controller.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

Controller::~Controller () {}

Error_t Controller::run ()
{
    Mode_t mode = Controller::Mode_t::SAFED;
    Error_t ret = this->getMode (mode);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    switch (mode)
    {
        case Controller::Mode_t::SAFED:
            return runSafed();
        case Controller::Mode_t::ENABLED:
            return runEnabled();
        default:
            return E_INVALID_ENUM;
    }
}

Error_t Controller::getMode (Controller::Mode_t& kModeRet)
{
    uint8_t mode;
    if (mPStateVector->read (mSvModeElem, mode) != E_SUCCESS)
    {
        return E_STATE_VECTOR_READ;
    }

    kModeRet = static_cast<Mode_t> (mode);

    return E_SUCCESS;
}

/*************************** PROTECTED FUNCTIONS ******************************/

Controller::Controller (std::shared_ptr<StateVector> kPStateVector, 
                        StateVectorElement_t kSvModeElem) :
    mPStateVector (kPStateVector),
    mSvModeElem   (kSvModeElem) {}
