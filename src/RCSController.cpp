#define _USE_MATH_DEFINES

#include <math.h>

#include "Math.hpp"
#include "RCSController.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

RCSController::RCSController (const RCSController::Config& kConfig,
                              std::shared_ptr<DataVector> kPDataVector,
                              DataVectorElement_t kDvModeElem) :
    Controller (kPDataVector, kDvModeElem),

    // Adopt phase channel configuration
    mCONFIG(kConfig),

    // One-time computation of all values defining the phase channel shape
    mUPPER_RL_RADS_PSEC (
        mCONFIG.rateLimitRadsPerSec
    ),
    mLOWER_RL_RADS_PSEC (
        mCONFIG.rateLimitsRatio * mUPPER_RL_RADS_PSEC
    ),
    mGRADIENT_ANGLE_LIMIT (
        (1 + mCONFIG.rateLimitsRatio) * mCONFIG.deadband
    ),
    mHYST_RL_RADS_PSEC (
        mCONFIG.hysteresisRateLimitRatio * mLOWER_RL_RADS_PSEC
    ),
    mHYST_UPPER_AL_RADS (
        mCONFIG.hysteresisGradientRatio * mGRADIENT_ANGLE_LIMIT
    ),
    mHYST_LOWER_AL_RADS (
        mGRADIENT_ANGLE_LIMIT - mHYST_UPPER_AL_RADS
    ),
    mCHANNEL_GRADIENT (
        (-mLOWER_RL_RADS_PSEC - mUPPER_RL_RADS_PSEC) / mGRADIENT_ANGLE_LIMIT
    ),
    mHYST_INTERCEPT_LOW (
        mHYST_RL_RADS_PSEC
        - mCHANNEL_GRADIENT * -mHYST_UPPER_AL_RADS
    ),
    mHYST_INTERCEPT_HIGH (
        -mHYST_INTERCEPT_LOW
    )
{
    mCurrentResponse = RCSController::Response_t::NO_FIRE;
    mAngleRads = 0;
    mRateRadsPerSec = 0;
}

Error_t RCSController::verifyConfig ()
{
    // Check finiteness
    if (!isfinite(mCONFIG.rateLimitRadsPerSec) ||
        !isfinite(mCONFIG.deadband) ||
        !isfinite(mCONFIG.rateLimitsRatio) ||
        !isfinite(mCONFIG.hysteresisGradientRatio) ||
        !isfinite(mCONFIG.hysteresisRateLimitRatio))
    {
        return E_NONFINITE_VALUE;
    }

    // Check bounds
    if (mCONFIG.rateLimitRadsPerSec <= 0 ||
        mCONFIG.deadband <= 0 ||
        mCONFIG.rateLimitsRatio <= 0 ||
        mCONFIG.rateLimitsRatio >= 1 ||
        mCONFIG.hysteresisGradientRatio <= 0 ||
        mCONFIG.hysteresisGradientRatio >= 1 ||
        mCONFIG.hysteresisRateLimitRatio <= 1)
    {
        return E_OUT_OF_BOUNDS;
    }

    // Check overflow
    if (!isfinite(mUPPER_RL_RADS_PSEC) ||
        !isfinite(mLOWER_RL_RADS_PSEC) ||
        !isfinite(mGRADIENT_ANGLE_LIMIT) ||
        !isfinite(mHYST_RL_RADS_PSEC) ||
        !isfinite(mHYST_UPPER_AL_RADS) ||
        !isfinite(mHYST_LOWER_AL_RADS) ||
        !isfinite(mCHANNEL_GRADIENT) ||
        !isfinite(mHYST_INTERCEPT_LOW) ||
        !isfinite(mHYST_INTERCEPT_HIGH))
    {
        return E_OVERFLOW;
    }

    return E_SUCCESS;
}

Error_t RCSController::runEnabled ()
{
    Error_t err = this->computeResponse ();
    return err;
}

Error_t RCSController::runSafed ()
{
    return E_SUCCESS;
}

Error_t RCSController::setAngle (float kAngle)
{
    // Validate finiteness
    if (!isfinite (kAngle))
    {
        // Bad angle; assume NO_FIRE
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
        return E_NONFINITE_VALUE;
    }
    // Validate within bounds
    else if (kAngle < ATT_BOUND_LOW_RADS ||
             kAngle >= ATT_BOUND_HIGH_RADS)
    {
        // Bad angle; assume NO_FIRE
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
        return E_OUT_OF_BOUNDS;
    }

    mAngleRads = kAngle;
    return E_SUCCESS;
}

Error_t RCSController::setRate (float kRate)
{
    if (!isfinite (kRate))
    {
        // Bad rate; assume NO_FIRE
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
        return E_NONFINITE_VALUE;
    }

    mRateRadsPerSec = kRate;
    return E_SUCCESS;
}

Error_t RCSController::getResponse (RCSController::Response_t& moment) {
    Controller::Mode_t mode;
    Error_t result = this->getMode (mode);

    if (result != E_SUCCESS)
    {
        return result;
    }

    // If the controller is not enabled, return NO_FIRE
    if (mode != Controller::Mode_t::ENABLED)
    {
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
    }

    moment = mCurrentResponse;
    return result;
}

/*************************** PRIVATE FUNCTIONS ********************************/

Error_t RCSController::computeResponse ()
{
    // Compute critical points used to identify position within the plane
    Error_t err = this->computeCriticalResponsePoints (mAngleRads);

    if (err != E_SUCCESS)
    {
        // Critical points could not be calculated; assume NO_FIRE
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
        return err;
    }

    // RCS was not firing last iteration but may need to now if the rocket left
    // the phase channel
    if (mCurrentResponse == RCSController::Response_t::NO_FIRE)
    {
        // Conditions for firing in the negative direction:
        if (
            // Moving too fast in positive direction
            (mAngleRads <= 0 &&
             mRateRadsPerSec >= mUPPER_RL_RADS_PSEC) ||
            // Recovering too slowly from positive error
            (mAngleRads >= mGRADIENT_ANGLE_LIMIT &&
             mRateRadsPerSec >= -mLOWER_RL_RADS_PSEC) ||
            // Another indication of slow recovery from positive error
            (mAngleRads >= 0 &&
             mAngleRads <= mGRADIENT_ANGLE_LIMIT &&
             mRateRadsPerSec >= mGradientLimitHigh))
        {
            mCurrentResponse = RCSController::Response_t::FIRE_NEGATIVE;
        }
        // Conditions for firing in the positive direction:
        else if (
            // Moving too fast in negative direction
            (mAngleRads >= 0 &&
             mRateRadsPerSec <= -mUPPER_RL_RADS_PSEC) ||
            // Recovering too slowly from negative error
            (mAngleRads <= 0 &&
             mAngleRads >= -mGRADIENT_ANGLE_LIMIT &&
             mRateRadsPerSec <= mGradientLimitLow) ||
            // Another indication of slow recovery from negative error
            (mAngleRads <= -mGRADIENT_ANGLE_LIMIT &&
             mRateRadsPerSec <= mLOWER_RL_RADS_PSEC))
        {
            mCurrentResponse = RCSController::Response_t::FIRE_POSITIVE;
        }
        // Otherwise, we're within the phase channel
        else
        {
            mCurrentResponse = RCSController::Response_t::NO_FIRE;
        }
    }
    // RCS was firing last iteration and will either continue firing or stop
    // firing if the rocket reentered the phase channel and crossed the
    // hysteresis lines
    else
    {
        // Conditions for continuing to fire in the positive direction:
        if (
            // Recovering too slowly from negative error or diverging
            (mAngleRads <= -mHYST_UPPER_AL_RADS &&
             mRateRadsPerSec <= mHYST_RL_RADS_PSEC) ||
            // Another indication of slow recovery from negative error
            (mAngleRads <= mHYST_LOWER_AL_RADS &&
             mAngleRads >= -mHYST_UPPER_AL_RADS &&
             mRateRadsPerSec <= mHystLimitLow) ||
            // Moving too fast in negative direction
            (mAngleRads >= mHYST_LOWER_AL_RADS &&
             mRateRadsPerSec <= -mUPPER_RL_RADS_PSEC))
        {
            mCurrentResponse = RCSController::Response_t::FIRE_POSITIVE;
        }
        // Conditions for continuing to fire in negative direction:
        else if (
            // Moving too fast in positive direction
            (mAngleRads <= -mHYST_LOWER_AL_RADS &&
             mRateRadsPerSec >= mUPPER_RL_RADS_PSEC) ||
            // Recovering too slowly from positive error or diverging
            (mAngleRads >= -mHYST_LOWER_AL_RADS &&
             mAngleRads <= mHYST_UPPER_AL_RADS &&
             mRateRadsPerSec >= mHystLimitHigh) ||
            // Another indication of slow recovery from positive error
            (mAngleRads >= mHYST_UPPER_AL_RADS &&
             mRateRadsPerSec >= -mHYST_RL_RADS_PSEC))
        {
            mCurrentResponse = RCSController::Response_t::FIRE_NEGATIVE;
        }
        // Otherwise, rocket has stabilized in this axis
        else
        {
            mCurrentResponse = RCSController::Response_t::NO_FIRE;
        }
    }

    return E_SUCCESS;
}

Error_t RCSController::computeCriticalResponsePoints (float kAngleRads)
{
    mRateLimitOffset = kAngleRads * (mLOWER_RL_RADS_PSEC + mUPPER_RL_RADS_PSEC)
                       / -mGRADIENT_ANGLE_LIMIT;
    mGradientLimitLow  = -mUPPER_RL_RADS_PSEC + mRateLimitOffset;
    mGradientLimitHigh = mUPPER_RL_RADS_PSEC + mRateLimitOffset;
    mHystLimitLow = mCHANNEL_GRADIENT * kAngleRads + mHYST_INTERCEPT_LOW;
    mHystLimitHigh = mCHANNEL_GRADIENT * kAngleRads + mHYST_INTERCEPT_HIGH;

    // Check critical points for overflow
    if (!isfinite(mRateLimitOffset) ||
        !isfinite(mGradientLimitLow) ||
        !isfinite(mGradientLimitHigh) ||
        !isfinite(mHystLimitLow) ||
        !isfinite(mHystLimitHigh))
    {
        return E_OVERFLOW;
    }

    return E_SUCCESS;
}
