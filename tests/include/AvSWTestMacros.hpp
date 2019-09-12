/**
 * Some testing macros specific to the avionics platform.
 */

# ifndef AVSW_TEST_MACROS_HPP
# define AVSW_TEST_MACROS_HPP

#include <string>

#include "CppUTest/TestHarness.h"
#include "Errors.h"

/**
 * Scratch variables for testing macros.
 */
static std::string gFailstr;
static Error_t gErr;

/**
 * Fails the ongoing test if the absolute difference between two integral types
 * is greater than some very small number.
 *
 * Note: floats should be checked for finiteness before being passed.
 *
 * @param   kExp Expected value.
 * @param   kVal Actual value.
 */
#define CHECK_APPROX(kExp, kVal)                                               \
if (fabs(kExp - kVal) > 1e-6)                                                  \
{                                                                              \
    gFailstr = std::to_string(kExp) + " !~ " + std::to_string(kVal);           \
    FAIL(gFailstr.c_str());                                                    \
}

/**
 * Fails the ongoing test if some expression does not evaluate to E_SUCCESS.
 *
 * @param   kExpr Expression to evaluate.
 */
#define CHECK_SUCCESS(kExpr)                                                   \
gErr = kExpr;                                                                  \
if (gErr != E_SUCCESS)                                                         \
{                                                                              \
    gFailstr = "Call produced error " + std::to_string(gErr) +                 \
               " when success was expected";                                   \
    FAIL(gFailstr.c_str());                                                    \
}

# endif
