/**
 * Utilities for rocket GNC.
 *
 * NOTICE: This file contains functions which do not use the Error Handling
 * Framework. They do not return Error_t and consequently do no internal error
 * checking. It is the responsibility of the programmer to check for errors in
 * the results of these functions. Additionally, special attention should be
 * given to functions with warnings in their documentation.
 *
 *               ---- GUIDELINES FOR SAFETY-CRITICAL MATH ----
 *
 *   Math in flight software falls into one of two categories:
 *
 *   (1) Low-level math functions are functions for general use whose results
 *       do not impact the rocket directly. This includes basic formulas like
 *       vector rotation, PID control, or the ideal gas law. It is not necessary
 *       for low-level math functions to use the Error Handling Framework.
 *
 *       Overflow, NaN, invalid parameters (e.g. negative values which should
 *       be positive), and floating point division by zero do not have to be
 *       checked by low-level math functions. Integer division by zero, however,
 *       must be checked because it generates a signal which can stop the
 *       program.
 *
 *       Low-level math functions must document any nonobvious conditions under
 *       which they fail. For example, a vector with small enough magnitude
 *       cannot be normalized due to the constraints of floating point math.
 *
 *   (2) High-level math functions are concrete implementations of algorithms
 *       which control the rocket. These are usually implemented by Controllers
 *       and make use of low-level math functions. Examples of high-level math
 *       functions include algorithms for the RCS thrusters, actuated fins,
 *       and navigation system. High-level math functions must check for errors.
 *       These include but are not limited to:
 *
 *         (a) Floating point calculations that can result in overflow or NaN.
 *
 *         (b) STL functions with limited domains like asin() and sqrt().
 *
 *         (c) Any failure modes documented by the low-level math functions that
 *             the high-level math function makes use of.
 *
 *       In general, high-level math functions are the filter between sources
 *       of input (e.g. sensors, the programmer) and the rocket's actuators.
 *       It is the responsibility of high-level math functions to prevent
 *       dangerous values from entering the Data Vector and being consumed by
 *       Devices.
 */

#ifndef GNC_UTILS_HPP
#define GNC_UTILS_HPP

#include <limits>
#include <math.h>

#include "DataVector.hpp"
#include "Errors.hpp"

/*********************************** TYPES ************************************/

/**
 * Type for representing real numbers.
 *
 * This is intended as the single authority of floating point precision across
 * the entire rocket. GNC code should use this type in place of double or float
 * when possible.
 *
 * Rationale: Single-precision floats are sufficient for GNC purposes. The
 * maximum value is far beyond that of the largest state variable (likely
 * altitude). Non-subnormal floats have a resolution of 1.19209e-07. This is
 * negligible compared to the expected order of magnitude of process noise, so
 * doubling the level of precision provides no benefit. Limiting GNC values to
 * single precision has the additional advantage of significantly reducing Data
 * Vector size.
 */
typedef float Real_t;

/**
 * 3-vector type.
 */
struct Vector3
{
    /**
     * Vector components.
     */
    Real_t x;
    Real_t y;
    Real_t z;

    /**
     * Computes the cross product of this vector and another.
     *
     * @param   kRhs RHS vector.
     *
     * @ret     Cross product.
     */
    Vector3 cross (const Vector3& kRhs) const;

    /**
     * Gets the magnitude of this vector.
     *
     * @ret     Vector magnitude.
     */
    Real_t magnitude () const;

    /**
     * Computes the product of this vector and a scalar.
     *
     * @param   kScalar Scalar.
     *
     * @ret     Scaled vector.
     */
    template <typename T>
    Vector3 operator* (T kScalar) const
    {
        return {(Real_t) (x * kScalar),
                (Real_t) (y * kScalar),
                (Real_t) (z * kScalar)};
    }

    /**
     * Computes the sum of this vector and another.
     *
     * @param   kRhs RHS vector.
     *
     * @ret     Sum vector.
     */
    Vector3 operator+ (const Vector3& kRhs) const;
};

/**
 * Quaternion type.
 */
struct Quaternion
{
    /**
     * Quaternion components.
     */
    Real_t w;
    Real_t x;
    Real_t y;
    Real_t z;

    /**
     * Rotates a vector by this quaternion.
     *
     * WARNING: Quaternion must be normalized for a correct answer.
     *
     * @param   kVec Vector to rotate.
     *
     * @ret     Rotated vector.
     */
    Vector3 rotate (const Vector3& kVec) const;

    /**
     * Gets the magnitude of this quaternion.
     *
     * @ret     Quaternion magnitude.
     */
    Real_t magnitude () const;

    /**
     * Normalizes this quaternion.
     *
     * WARNING: If the current magnitude is 0, all components become infinity.
     * If the current magnitude is very small, subnormal rounding may prevent
     * the magnitude from becoming sufficiently close to 1. This method will
     * return an error in both cases, but the underlying quaternion will still
     * have been mutated.
     *
     * @ret     E_SUCCESS              Successfully normalized.
     *          E_NONNORMAL_QUATERNION Quaternion could not be normalized.
     */
    Error_t normalize ();

    /**
     * Gets if this quaternion is normalized (magnitude approximately 1; see
     * GNCUtils::approx for definition of approximate).
     *
     * @ret     If quaternion is normalized.
     */
    bool isNormalized () const;
};

/********************************* UTILITIES **********************************/

namespace GNCUtils
{
    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF UNIT TESTS
     *
     * The maximum absolute difference between two floating point numbers to
     * consider them weakly equal (see GNCUtils::weakApprox).
     */
    extern const Real_t WEAK_APPROX_EPSILON;

    /**
     * Gets if two real numbers are approximately equal. Uses epsilon from STL
     * limits and a scale factor based on the arguments so that the result is
     * agnostic of order of magnitude.
     *
     * This was taken from Perforce High-Integrity C++ Version 4.0.
     *
     * @param   kA Number A.
     * @param   kB Number B.
     *
     * @ret     If A is approximately B.
     */
    inline bool approx (Real_t kA, Real_t kB)
    {
        // Compute a scale factor so that epsilon has meaning in the specified
        // order of magnitude.
        Real_t scale = fabs (kA) / 2 + fabs (kB) / 2;

        return fabs (kA - kB) <= std::numeric_limits<Real_t>::epsilon () * scale;
    }

    /**
     * Gets if two numbers are approximately equal. This uses a weaker
     * definition of approximate that doesn't scale with order of magnitude:
     * A is approximately B if they are within 1/1000th of each other. Weak
     * approximation is more suitable for comparing engineering units.
     *
     * @param   kA Number A.
     * @param   kB Number B.
     *
     * @ret     If A is approximately B.
     */
    inline bool weakApprox (Real_t kA, Real_t kB)
    {
        return fabs (kA - kB) <= WEAK_APPROX_EPSILON;
    }

    /**
     * Reads a Vector3 from a Data Vector.
     *
     * @param   kPDv               Data Vector to read from.
     * @param   kVec               Vector3 to write to.
     * @param   kXElem             Element storing X component.
     * @param   kYElem             Element storing Y component.
     * @param   kZElem             Element storing Z component.
     *
     * @ret     E_SUCCESS          Read succeeded.
     *          E_DATA_VECTOR_READ Failed to read one or more components.
     */
    Error_t dvReadVector3 (std::shared_ptr<DataVector>& kPDv,
                           Vector3& kVec,
                           DataVectorElement_t kXElem,
                           DataVectorElement_t kYElem,
                           DataVectorElement_t kZElem);

    /**
     * Writes a Vector3 to a Data Vector.
     *
     * @param   kPDv                Data Vector to write to.
     * @param   kVec                Vector3 to read from.
     * @param   kXElem              Element storing X component.
     * @param   kYElem              Element storing Y component.
     * @param   kZElem              Element storing Z component.
     *
     * @ret     E_SUCCESS           Read succeeded.
     *          E_DATA_VECTOR_WRITE Failed to write one or more components.
     */
    Error_t dvWriteVector3 (std::shared_ptr<DataVector>& kPDv,
                            const Vector3& kVec,
                            DataVectorElement_t kXElem,
                            DataVectorElement_t kYElem,
                            DataVectorElement_t kZElem);

    /**
     * Reads a Quaternion from a Data Vector.
     *
     * @param   kPDv               Data Vector to read from.
     * @param   kVec               Quaternion to write to.
     * @param   kWElem             Element storing W component.
     * @param   kXElem             Element storing X component.
     * @param   kYElem             Element storing Y component.
     * @param   kZElem             Element storing Z component.
     *
     * @ret     E_SUCCESS          Read succeeded.
     *          E_DATA_VECTOR_READ Failed to read one or more components.
     */
    Error_t dvReadQuaternion (std::shared_ptr<DataVector>& kPDv,
                              Quaternion& kQuat,
                              DataVectorElement_t kWElem,
                              DataVectorElement_t kXElem,
                              DataVectorElement_t kYElem,
                              DataVectorElement_t kZElem);

    /**
     * Writes a Quaternion to a Data Vector.
     *
     * @param   kPDv                Data Vector to write to.
     * @param   kVec                Quaternion to read from.
     * @param   kWElem              Element storing W component.
     * @param   kXElem              Element storing X component.
     * @param   kYElem              Element storing Y component.
     * @param   kZElem              Element storing Z component.
     *
     * @ret     E_SUCCESS           Read succeeded.
     *          E_DATA_VECTOR_WRITE Failed to write one or more components.
     */
    Error_t dvWriteQuaternion (std::shared_ptr<DataVector>& kPDv,
                               const Quaternion& kQuat,
                               DataVectorElement_t kWElem,
                               DataVectorElement_t kXElem,
                               DataVectorElement_t kYElem,
                               DataVectorElement_t kZElem);                    
}

#endif