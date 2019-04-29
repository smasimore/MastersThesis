#include "Log.hpp"

Log::Log () : log ()
{
}

Error_t Log::logEvent (const LogEvent_t event, const LogInfo_t info)
{
    if (event >= Log::LogEvent_t::LAST)
    {
        return E_INVALID_ENUM;
    }

    return E_SUCCESS;
}