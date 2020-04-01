/**
 * The Time module gets the local time in nanoseconds using CLOCK_REALTIME. This 
 * means that the clock represents "wall time" and is subject to corrections by
 * the NTP daemon and user-initiated system calls. 
 *
 * Time uses a singleton object pattern to ensure initialization occurs. In 
 * initialization the clock is checked to determine time until overflow and will 
 * return an error if it is within a year of overflowing. 
 *
 * How to initialize singleton:
 * Time* pTime;
 * Time::getInstance (pTime);
 *
 * How to get current time:
 * TimeNs_t timeNs;
 * pTime->getTimeNs (timeNs);
 *
 * WARNINGS
 *
 *     #1 Do not use any system calls that adjust the system time during the
 *        lifetime of this object. This will cause time to jump back/foward.
 *
 *     #2 If using ClockSync, which uses NTP, it must be run before the Time 
 *        module is initialized. Otherwise, time will jump back/foward.
 *
 *     #3 The sbRIO's are expected to use the UTC timezone and to NOT
 *        automatically adjust to Daylight Savings. This is their default
 *        configuration.
 *
 */
#include <stdint.h>

#include "Errors.hpp"

#ifndef TIME_HPP
#define TIME_HPP

class Time
{
public:

    /**
     * Unsigned long long to store time in nanoseconds
     */
    typedef uint64_t TimeNs_t;

    /**
     * Unsigned long long used to convert seconds to nanoseconds
     */
    static const uint64_t NS_IN_S = 1000000000;

    /**
     * Unsigned long long used to convert milliseconds to nanoseconds
     */
    static const uint64_t NS_IN_MS = 1000000;

    /**
     * Unsigned long long used to convert microseconds to nanoseconds
     */
    static const uint64_t NS_IN_US = 1000;

    /**
     * Unsigned long long used to convert milliseconds to microseconds
     */
    static const uint64_t US_IN_MS = 1000;

    /**
     * Unsigned long long used to convert seconds to milliseconds.
     */
    static const uint64_t MS_IN_S = 1000;

    /**
     * Returns current time in nanoseconds.
     *
     * @param   KTimeNsRet                 Time in nanoseconds.
     *
     * @ret     E_SUCCESS                  Successfully got time.
     * @ret     E_FAILED_TO_GET_TIME       clock_gettime() unsuccessful
     */
    Error_t getTimeNs (TimeNs_t& kTimeNsRet);

    /**
     * Used to initiate and access instance
     *
     * @param   kPTimeRet                  Param to fill with pointer to static
     *                                     Time instance
     *
     * @ret     E_SUCCESS                  Instance correctly accessed
     * @ret     E_FAILED_TO_INIT_TIME      clock_gettime() unsuccessful  at init
     * @ret     E_OVERFLOW_IMMINENT        Time at init less than 1 year from overflow
     */
    static Error_t getInstance (Time*& kPTimeRet);

private:

    /**
     * Max allowable seconds CLOCK_REALTIME clock can be from overflow at
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
    Time (Error_t& kErrorRet);

    /**
     * Private copy constructor to enforce singleton
     */
    Time (Time const &);

    /**
     * Private assignment operator to enforce singleton
     */
    Time& operator= (Time const &);
};

#endif /* TIMENS_HPP */
