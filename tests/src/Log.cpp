#include "Log.hpp"

Log::Log (Error_t &ret) : log ()
{
    if (pthread_mutex_init (this->pLock, NULL) != 0)
    {
        ret = E_FAILED_TO_INIT_LOCK;
    }
    else
    {
        ret = E_SUCCESS;
    }
}

Log::~Log ()
{
    // Don't check return since there's not an easy way to surface an error and
    // the consequence of a floating mutex is minimal.
    pthread_mutex_destroy (this->pLock);
}

Error_t Log::logEvent (const LogEvent_t event, const LogInfo_t info)
{
    if (event >= Log::LogEvent_t::LAST)
    {
        return E_INVALID_ENUM;
    }

    // Create row.
    struct Log::LogRow row = {event, info};

    // Add to log.
    if (pthread_mutex_lock (this->pLock) != 0)
    {
        return E_FAILED_TO_LOCK;
    }
    this->log.push_back (row);
    if (pthread_mutex_unlock (this->pLock) != 0)
    {
        return E_FAILED_TO_UNLOCK;
    }

    return E_SUCCESS;
}

Error_t Log::verify (Log &logOne, Log &logTwo, bool &areEqual)
{
    std::vector<Log::LogRow> *pLogOneVec = &logOne.log;
    std::vector<Log::LogRow> *pLogTwoVec = &logTwo.log;

    // Start with areEqual being true and look for a case where the vectors are
    // not equal.
    areEqual = true;

    // Lock vectors.
    if (pthread_mutex_lock (logOne.pLock) != 0)
    {
        return E_FAILED_TO_LOCK;
    }
    if (pthread_mutex_lock (logTwo.pLock) != 0)
    {
        return E_FAILED_TO_LOCK;
    }

    if (pLogOneVec->size () != pLogTwoVec->size ())
    {
        areEqual = false;
    }
    else
    {
        for (uint32_t i = 0; i < pLogOneVec->size (); i++)
        {
            if ((pLogOneVec->at (i).event != pLogTwoVec->at (i).event) ||
                (pLogOneVec->at (i).info != pLogTwoVec->at (i).info))
            {
                areEqual = false;
            }
        }
    }

    // Unlock vectors.
    if (pthread_mutex_unlock (logOne.pLock) != 0)
    {
        return E_FAILED_TO_UNLOCK;
    }
    if (pthread_mutex_unlock (logTwo.pLock) != 0)
    {
        return E_FAILED_TO_UNLOCK;
    }

    return E_SUCCESS;
}
