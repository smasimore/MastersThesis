#include <iostream>
#include <sstream>
#include <string>

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
char* floatToArg (float kF)
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
    char* avNone[] =
    {
        "runIgniterTest"
    };
    CHECK_EQUAL (E_WRONG_ARGC, validateInput (1, avNone));

    // Non-numeric delay.
    char* avNonNum[] =
    {
        "runIgniterTest",
        "a"
    };
    CHECK_EQUAL (E_INVALID_ARGUMENT, validateInput (2, avNonNum));

    // Delay is below lower bound.
    char* avTooLow[] =
    {
        "runIgniterTest",
        floatToArg (gIGNITION_DELAY_LOWER_S - 0.01)
    };
    CHECK_EQUAL (E_OUT_OF_BOUNDS, validateInput (2, avTooLow));

    // Delay is above upper bound.
    char* avTooHigh[] =
    {
        "runIgniterTest",
        floatToArg (gIGNITION_DELAY_UPPER_S + 0.01)
    };
    CHECK_EQUAL (E_OUT_OF_BOUNDS, validateInput (2, avTooHigh));

    // Delay is OK.
    char* avOk[] =
    {
        "runIgniterTest",
        floatToArg ((gIGNITION_DELAY_LOWER_S + gIGNITION_DELAY_UPPER_S) / 2)
    };
    CHECK_SUCCESS (validateInput (2, avOk));
}
