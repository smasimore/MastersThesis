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
    if (mPDataVector->read (mDvModeElem, mode) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    kModeRet = static_cast<Mode_t> (mode);

    return E_SUCCESS;
}

/*************************** PROTECTED FUNCTIONS ******************************/

Controller::Controller (std::shared_ptr<DataVector> kPDataVector, 
                        DataVectorElement_t kDvModeElem) :
    mPDataVector (kPDataVector),
    mDvModeElem  (kDvModeElem) {}
