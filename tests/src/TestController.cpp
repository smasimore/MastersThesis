#include "Errors.h"
#include "Log.hpp"
#include "TestController.hpp"

Error_t TestController::runEnabled ()
{
    gPTestLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_ENABLED, 0);
    return E_SUCCESS;
}

Error_t TestController::runSafed ()
{
    gPTestLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_SAFED, 0);
    return E_SUCCESS;
}

Error_t TestController::verifyConfig ()
{
    return this->mConfig.valid ? E_SUCCESS : E_OUT_OF_BOUNDS;
}

TestController::TestController (TestController::Config_t kConfig,
                                std::shared_ptr<StateVector> kPStateVector,
                                StateVectorElement_t kSvModeElem) :
    Controller (kPStateVector, kSvModeElem),
    mConfig (kConfig) {}
