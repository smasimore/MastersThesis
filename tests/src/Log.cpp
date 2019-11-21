#include <iostream>

#include "Log.hpp"

Log::Log (Error_t &ret) : log ()
{
    // Initialize lock.
    if (pthread_mutex_init (&this->lock, NULL) != 0)
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
    pthread_mutex_destroy (&this->lock);
}

Error_t Log::logEvent (const LogEvent_t event, const LogInfo_t info)
{
    // Verify event param.
    if (event >= Log::LogEvent_t::LAST)
    {
        return E_INVALID_ENUM;
    }

    // Create row.
    struct Log::LogRow row = {event, info};

    // Lock log.
    if (pthread_mutex_lock (&this->lock) != 0)
    {
        return E_FAILED_TO_LOCK;
    }

    // Add to log.
    this->log.push_back (row);

    // Unlock log.
    if (pthread_mutex_unlock (&this->lock) != 0)
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

    // Lock logs.
    if ((pthread_mutex_lock (&logOne.lock) != 0) || 
        (pthread_mutex_lock (&logTwo.lock) != 0))
    {
        return E_FAILED_TO_LOCK;
    }

    // Check if the logs have the same size and set areEqual to false if they 
    // are not.
    if (pLogOneVec->size () != pLogTwoVec->size ())
    {
        areEqual = false;
        std::cout << "\nLog sizes not equal: " << pLogOneVec->size () << 
            " vs. " << pLogTwoVec->size () << std::endl;
        logOne.printLog ();
        logTwo.printLog ();
    }

    // If they are the same size, check if any of their rows are different.
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

        // Print logs for debugging.
        if (areEqual == false)
        {
            logOne.printLog ();
            logTwo.printLog ();
        }
    }

    // Unlock logs.
    if ((pthread_mutex_unlock (&logOne.lock) != 0) ||
        (pthread_mutex_unlock (&logTwo.lock) != 0))
    {
        return E_FAILED_TO_UNLOCK;
    }

    return E_SUCCESS;
}

void Log::printLog ()
{
    std::vector<Log::LogRow>* pLogVec = &this->log;

    std::cout << "\n Log" << std::endl;
    std::cout << "-------" << std::endl;
    for (uint32_t i = 0; i < pLogVec->size (); i++)
    {
        std::cout << (int32_t) pLogVec->at (i).event << ", "
            << (int32_t) pLogVec->at (i).info << std::endl;
    }
    std::cout << "-------" << std::endl;
}
