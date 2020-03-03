/**
 * Time Module Header
 *
 * Note: Needs testing for multiple cores
 *
 * Purpose: gets time in nanoseconds since initialization
 *
 * How to initialize singleton:
 * std::shared_ptr<TimeNs> pTimeNs;
 * TimeNs::getInstance (pTimeNs);
 *
 * How to get elapsed time:
 * TimeNs_t elapsedTime;
 * pTimeNs->getTimeSinceInit (elapsedTime);
 * elapsedTime now contains time between init and system call in ns.
 */
#include <stdint.h>

#include "Errors.hpp"

#ifndef TIMENS_HPP
#define TIMENS_HPP

class TimeNs
{
public:

    /**
     * Unsigned long long to store time in nanoseconds
     */
    typedef uint64_t TimeNs_t;

    /**
     * Unsigned long long used to convert seconds to nanoseconds
     */
    static const uint64_t NS_IN_SECOND = 1000000000;

    /**
     * Unsigned long long used to convert microseconds to nanoseconds
     */
    static const uint64_t NS_IN_US = 1000;

    /**
     * Returns time elapsed since TimeNs module initialized.
     *
     * @param   KElapsedTimeRet            Calculated elapsed time
     *
     * @ret     E_SUCCESS                  Non-zero elapsed time calculated
     * @ret     E_FAILED_TO_GET_TIME       clock_gettime() unsuccessful
     * @ret     E_OVERFLOW                 timespec.tv_sec overflow
     */
    Error_t getTimeSinceInit (TimeNs_t& kElapsedTimeRet);

    /**
     * Used to initiate and access instance
     *
     * @param   kPTimeNsRet                Param to fill with pointer to static
     *                                     TimeNs Instance
     *
     * @ret     E_SUCCESS                  Instance correctly accessed
     * @ret     E_FAILED_TO_INIT_TIME      clock_gettime() unsuccessful  at init
     * @ret     E_OVERFLOW_IMMINENT        Time at init less than 1 year from overflow
     */
    static Error_t getInstance (TimeNs*& kPTimeNsRet);

private:

    /**
     * Max allowable seconds CLOCK_MONOTONIC_RAW clock can be from overflow at
     * initialization time for initialization to succeed. 1 year.
     */
    const int32_t SECONDS_AWAY_FROM_OVERFLOW_TO_INIT =  365 * 24 * 60 * 60;

    /**
     * Stores initialization time; defined at init
     */
    TimeNs_t mTimeAtInit;

    /**
     * Private constructor
     */
    TimeNs (Error_t& kErrorRet);

    /**
     * Private copy constructor to enforce singleton
     */
    TimeNs (TimeNs const &);

    /**
     * Private assignment operator to enforce singleton
     */
    TimeNs& operator= (TimeNs const &);
};

#endif /* TIMENS_HPP */
