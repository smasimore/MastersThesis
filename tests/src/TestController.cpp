#include "Errors.h"
#include "Log.hpp"
#include "TestController.hpp"

Error_t TestController::runEnabled ()
{
    PTestLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_ENABLED, 0);
    return E_SUCCESS;
}

Error_t TestController::runSafed ()
{
    PTestLog->logEvent (Log::LogEvent_t::CONTROLLER_RAN_SAFED, 0);
    return E_SUCCESS;
}

Error_t TestController::verifyConfig ()
{
    return this->Config.valid ? E_SUCCESS : E_OUT_OF_BOUNDS;
}

TestController::TestController (struct TestController::TestConfig config) :
    Controller (),
    Config (config) {}
