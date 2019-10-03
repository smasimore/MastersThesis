#include "Controller.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

Controller::~Controller () {}

Error_t Controller::run ()
{
    switch (this->Mode)
    {
        case Controller::Mode_t::ENABLED:
            return runEnabled();
        case Controller::Mode_t::SAFED:
            return runSafed();
        default:
            return E_INVALID_ENUM;
    }
}

Error_t Controller::getMode (Controller::Mode_t &modeRet)
{
    modeRet = this->Mode;
    return E_SUCCESS;
}

Error_t Controller::setMode (Controller::Mode_t newMode)
{
    // Verify valid enum.
    if (newMode >= Controller::Mode_t::LAST)
    {
        return E_INVALID_ENUM;
    }

    this->Mode = newMode;

    return E_SUCCESS;
}

/*************************** PROTECTED FUNCTIONS ******************************/

Controller::Controller () :
    Mode (Controller::Mode_t::SAFED) {}
