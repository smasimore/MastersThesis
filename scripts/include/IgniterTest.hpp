# ifndef IGNITER_TEST_HPP
# define IGNITER_TEST_HPP

#include "Errors.h"

/**
 * Upper and lower bounds on the ignition delay.
 */
const double gIGNITION_DELAY_LOWER_S = 5;
const double gIGNITION_DELAY_UPPER_S = 10;

/**
 * Validates the command line arguments against stupidity.
 *
 * @param   kAc             Argument count.
 * @param   kAv             Argument vector.
 *
 * @ret     E_SUCCESS       Arguments are valid.
 *          E_WRONG_ARGC    Argument count was wrong.
 *          E_OUT_OF_BOUNDS Ignition delay was outside valid range.
 */
Error_t validateInput (int kAc, char** kAv);

namespace IgniterTest
{
    /**
     * Entry point.
     */
    void main (int kAc, char** kAv);
}

# endif
