/**
 * Macros and functions used across test files.
 *
 * Note: Because this includes the cpputest header, this must be included last
 *       in tests.
 */

# ifndef TEST_HELPERS_HPP
# define TEST_HELPERS_HPP

#include <string>

#include "Errors.h"
#include "ThreadManager.hpp"
#include "Log.hpp"

#include "CppUTest/TestHarness.h"

/********************************* MACROS *************************************/


/**
 * Fails the ongoing test if some expression does not evaluate to E_SUCCESS.
 *
 * @param   kExpr Expression to evaluate.
 */
#define CHECK_SUCCESS(kExpr)                                                   \
{                                                                              \
    Error_t macroErr = kExpr;                                                  \
    if (macroErr != E_SUCCESS)                                                 \
    {                                                                          \
        std::string macroFailStr = "Call produced error " +                    \
                                   std::to_string (macroErr) +                 \
                                   " when success was expected";               \
        FAIL (macroFailStr.c_str ());                                          \
    }                                                                          \
}

/**
 * Fails the ongoing test if some expression does not evaluate to err.
 *
 * @param   kExpr Expression to evaluate.
 * @param   err   Expected error.
 */
#define CHECK_ERROR(kExpr, err)                                                \
{                                                                              \
    Error_t macroErr = kExpr;                                                  \
    if (macroErr != err)                                                       \
    {                                                                          \
        std::string macroFailStr = "Call produced error " +                    \
                                   std::to_string (macroErr) + " when " +      \
                                   std::to_string (err) + " was expected";     \
        FAIL (macroFailStr.c_str());                                           \
    }                                                                          \
}

/**
 * Initializes pThreadManager, expectedLog, and actualLog as local variables to
 * be used in a test.
 */
#define INIT_THREAD_MANAGER_AND_LOGS                                           \
    Error_t ret = E_SUCCESS;                                                   \
    ThreadManager *pThreadManager = nullptr;                                   \
    ret = ThreadManager::getInstance (&pThreadManager);                        \
    CHECK_EQUAL(E_SUCCESS, ret);                                               \
    Log expectedLog = Log (ret);                                               \
    Log testLog = Log (ret);

/**
 * Verifies local variables expectedLog == actualLog.
 */
#define VERIFY_LOGS                                                            \
{                                                                              \
    bool logsEqual = false;                                                    \
    ret = Log::verify (expectedLog, testLog, logsEqual);                       \
    CHECK_EQUAL (E_SUCCESS, ret);                                              \
    CHECK_TRUE (logsEqual);                                                    \
}

/**
 * Fails the ongoing test if the absolute difference between two integral types
 * is greater than specified bound
 *
 * Note: llabs() used to get absolute value
 *
 * @param   kExp Expected value.
 * @param   kVal Actual value.
 * @param   kBound Bound
 */
#define CHECK_IN_BOUND(kExp, kVal, kBound)                                     \
{                                                                              \
    if (llabs (kExp - kVal) > kBound)                                          \
    {                                                                          \
        std::string macroFailStr = std::to_string(kExp) + " != " +             \
                                   std::to_string (kVal);                      \
        FAIL (macroFailStr.c_str ());                                          \
    }                                                                          \
}

/******************************* FUNCTIONS ************************************/

namespace TestHelpers
{
    /**
     * Sleep for the specified number of milliseconds.
     *
     * @param     ms    Milliseconds to sleep.
     */
    void sleepMs (uint32_t ms);

}

# endif
