/**
 * Math utilities for rocket control.
 */
# ifndef MATH_HPP
# define MATH_HPP

/**
 * Double- and single-precision pi macros.
 */
#define PI64 3.141592653589793238462643383279502884
#define PI32 3.1415927410125732421875f
/**
 * Bounds on rocket attitude in a particular axis.
 */
#define ATT_BOUND_LOW_RADS  -PI32 // Inclusive
#define ATT_BOUND_HIGH_RADS  PI32 // Exclusive

# endif
