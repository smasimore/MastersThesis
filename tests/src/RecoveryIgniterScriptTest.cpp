#include "NiFpga.h"
#include "NiFpga_IO.h"
#include "RecoveryIgniterTest.hpp"

#include "TestHelpers.hpp"
#include "CppUTest/TestHarness.h"

/**
 * Converts a float to char*, ie command line arg. Operates on the same pointer,
 * and so destroys the results of previous calls.
 *
 * @param   kF Float to convert.
 *
 * @ret     Command line arg.
 */
static char* floatToArg (float kF)
{
    static char pStr[128];
    sprintf (pStr, "%f", kF);
    return pStr;
}

TEST_GROUP(RecoveryIgniterScriptTest)
{
};

/**
 * Ignition delay input is validated correctly.
 */
TEST(RecoveryIgniterScriptTest, InputValidation)
{
    // No delay.
    const char* avNone[] = {"runIgniterTest"};
    CHECK_EQUAL (E_WRONG_ARGC, RecoveryIgniterTest::validateInput (1, avNone));

    // Non-numeric delay.
    const char* avNonNum[] =
    {
        "runIgniterTest",
        "a"
    };
    CHECK_EQUAL (E_INVALID_ARGUMENT,
                 RecoveryIgniterTest::validateInput (2,
                                                     (const char**) avNonNum));

    // Delay is below lower bound.
    const char* avTooLow[] =
    {
        "runIgniterTest",
        (const char*) floatToArg (
                RecoveryIgniterTest::gIGNITION_DELAY_LOWER_S - 0.01)
    };
    CHECK_EQUAL (E_OUT_OF_BOUNDS,
                 RecoveryIgniterTest::validateInput (2,
                                                     (const char**) avTooLow));

    // Delay is above upper bound.
    const char* avTooHigh[] =
    {
        "runIgniterTest",
        (const char*) floatToArg (
                RecoveryIgniterTest::gIGNITION_DELAY_UPPER_S + 0.01)
    };
    CHECK_EQUAL (E_OUT_OF_BOUNDS,
                 RecoveryIgniterTest::validateInput (2,
                                                     (const char**) avTooHigh));

    // Delay is OK.
    const char* avOk[] =
    {
        "runIgniterTest",
        (const char*) floatToArg (
                (RecoveryIgniterTest::gIGNITION_DELAY_LOWER_S +
                 RecoveryIgniterTest::gIGNITION_DELAY_UPPER_S) / 2)
    };
    CHECK_SUCCESS (RecoveryIgniterTest::validateInput (2,
                                                       (const char**) avOk));
}
