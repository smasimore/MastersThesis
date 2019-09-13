#include <math.h>

#include "RCSController.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

RCSController::RCSController (const RCSController::Config& kConfig) :
    // Adopt phase channel configuration
    mCONFIG(kConfig),

    // One-time computation of all values defining the phase channel shape
    mUPPER_RATE_LIMIT (
        mCONFIG.rateLimit
    ),
    mLOWER_RATE_LIMIT (
        mCONFIG.rateLimitsRatio * mUPPER_RATE_LIMIT
    ),
    mGRADIENT_ANGLE_LIMIT (
        (1 + mCONFIG.rateLimitsRatio) * mCONFIG.deadband
    ),
    mHYSTERESIS_RATE_LIMIT (
        mCONFIG.hysteresisRateLimitRatio * mLOWER_RATE_LIMIT
    ),
    mHYSTERESIS_UPPER_ANGLE_LIMIT (
        mCONFIG.hysteresisGradientRatio * mGRADIENT_ANGLE_LIMIT
    ),
    mHYSTERESIS_LOWER_ANGLE_LIMIT (
        mGRADIENT_ANGLE_LIMIT - mHYSTERESIS_UPPER_ANGLE_LIMIT
    ),
    mCHANNEL_GRADIENT (
        (-mLOWER_RATE_LIMIT - mUPPER_RATE_LIMIT) / mGRADIENT_ANGLE_LIMIT
    ),
    mHYSTERESIS_INTERCEPT_LOW (
        mHYSTERESIS_RATE_LIMIT
        - mCHANNEL_GRADIENT * -mHYSTERESIS_UPPER_ANGLE_LIMIT
    ),
    mHYSTERESIS_INTERCEPT_HIGH (
        -mHYSTERESIS_INTERCEPT_LOW
    )
{
    mCurrentResponse = RCSController::Response_t::NO_FIRE;
    mAngle = 0;
    mRate = 0;
}

Error_t RCSController::verifyConfig (bool& kValid)
{
             // Constraints on rate limit
    kValid = isfinite(mCONFIG.rateLimit) &&
             mCONFIG.rateLimit > 0 &&
             // Constraints on deadband
             isfinite(mCONFIG.deadband) &&
             mCONFIG.deadband > 0 &&
             // Constraints on rate limit ratio
             isfinite(mCONFIG.rateLimitsRatio) &&
             mCONFIG.rateLimitsRatio > 0 &&
             mCONFIG.rateLimitsRatio < 1 &&
             // Constraints on hysteresis gradient ratio
             isfinite(mCONFIG.hysteresisGradientRatio) &&
             mCONFIG.hysteresisGradientRatio > 0 &&
             mCONFIG.hysteresisGradientRatio < 1 &&
             // Constrains on hysteresis rate limit ratio
             isfinite(mCONFIG.hysteresisRateLimitRatio) &&
             mCONFIG.hysteresisRateLimitRatio > 1;

    return E_SUCCESS;
}

Error_t RCSController::runEnabled ()
{
    Error_t err = this->computePhasePlaneMoment ();
    return err;
}

Error_t RCSController::runSafed ()
{
    return E_SUCCESS;
}

Error_t RCSController::setAngle (float kAngle)
{
    if (!isfinite (kAngle))
    {
        // Bad angle; assume NO_FIRE
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
        return E_NONFINITE_VALUE;
    }

    mAngle = kAngle;
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

    mRate = kRate;
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
    // If the response has somehow taken on a value not enumerated in
    // Response_t, forcibly override it to NO_FIRE
    else if (mCurrentResponse <= RCSController::Response_t::FIRST ||
             mCurrentResponse >= RCSController::Response_t::LAST)
    {
        mCurrentResponse = RCSController::Response_t::NO_FIRE;
        result = E_INVALID_ENUM;
    }

    moment = mCurrentResponse;
    return result;
}

/*************************** PRIVATE FUNCTIONS ********************************/

Error_t RCSController::computePhasePlaneMoment ()
{
    // Compute distance to gradient limits in the event we're in the gradient
    const float RATE_LIMIT_OFFSET =
        mAngle * (mLOWER_RATE_LIMIT + mUPPER_RATE_LIMIT)
        / -mGRADIENT_ANGLE_LIMIT;
    // Compute upper and lower rate limits of current angle
    const float GRADIENT_LIMIT_LOW = -mUPPER_RATE_LIMIT + RATE_LIMIT_OFFSET;
    const float GRADIENT_LIMIT_HIGH = mUPPER_RATE_LIMIT + RATE_LIMIT_OFFSET;
    // Compute upper and lower hysteresis limits of current angle
    const float HYSTERESIS_LIMIT_LOW =
        mCHANNEL_GRADIENT * mAngle + mHYSTERESIS_INTERCEPT_LOW;
    const float HYSTERESIS_LIMIT_HIGH =
        mCHANNEL_GRADIENT * mAngle + mHYSTERESIS_INTERCEPT_HIGH;

    // RCS was not firing last iteration but may need to now if the rocket left
    // the phase channel
    if (mCurrentResponse == RCSController::Response_t::NO_FIRE)
    {
        // Conditions for firing in the negative direction:
    	if (
            // Moving too fast in positive direction
            (mAngle <= 0 &&
             mRate >= mUPPER_RATE_LIMIT) ||
            // Recovering too slowly from positive error
    	    (mAngle >= mGRADIENT_ANGLE_LIMIT &&
             mRate >= -mLOWER_RATE_LIMIT) ||
            // Another indication of slow recovery from positive error
    		(mAngle >= 0 &&
             mAngle <= mGRADIENT_ANGLE_LIMIT &&
             mRate >= GRADIENT_LIMIT_HIGH))
        {
            mCurrentResponse = RCSController::Response_t::FIRE_NEGATIVE;
        }
        // Conditions for firing in the positive direction:
        else if (
            // Moving too fast in negative direction
            (mAngle >= 0 &&
             mRate <= -mUPPER_RATE_LIMIT) ||
            // Recovering too slowly from negative error
            (mAngle <= 0 &&
             mAngle >= -mGRADIENT_ANGLE_LIMIT &&
             mRate <= GRADIENT_LIMIT_LOW) ||
            // Another indication of slow recovery from negative error
            (mAngle <= -mGRADIENT_ANGLE_LIMIT &&
             mRate <= mLOWER_RATE_LIMIT))
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
            (mAngle <= -mHYSTERESIS_UPPER_ANGLE_LIMIT &&
             mRate <= mHYSTERESIS_RATE_LIMIT) ||
            // Another indication of slow recovery from negative error
            (mAngle <= mHYSTERESIS_LOWER_ANGLE_LIMIT &&
             mAngle >= -mHYSTERESIS_UPPER_ANGLE_LIMIT &&
             mRate <= HYSTERESIS_LIMIT_LOW) ||
            // Moving too fast in negative direction
            (mAngle >= mHYSTERESIS_LOWER_ANGLE_LIMIT &&
             mRate <= -mUPPER_RATE_LIMIT))
        {
            mCurrentResponse = RCSController::Response_t::FIRE_POSITIVE;
        }
        // Conditions for continuing to fire in negative direction:
        else if (
            // Moving too fast in positive direction
            (mAngle <= -mHYSTERESIS_LOWER_ANGLE_LIMIT &&
             mRate >= mUPPER_RATE_LIMIT) ||
            // Recovering too slowly from positive error or diverging
            (mAngle >= -mHYSTERESIS_LOWER_ANGLE_LIMIT &&
             mAngle <= mHYSTERESIS_UPPER_ANGLE_LIMIT &&
             mRate >= HYSTERESIS_LIMIT_HIGH) ||
            // Another indication of slow recovery from positive error
            (mAngle >= mHYSTERESIS_UPPER_ANGLE_LIMIT &&
             mRate >= -mHYSTERESIS_RATE_LIMIT))
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
