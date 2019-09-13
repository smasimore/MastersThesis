/**
 * Control algorithm for a single axis of the RCS. There will be three instances
 * of this controller running on the rocket during free flight; one to correct
 * each of roll, pitch, and yaw.
 *
 * RCSController uses a phase plane to relate rocket angle and rate (angular
 * velocity) to RCS response signs (-1, 0, or 1). The phase plane design is
 * based on that of NASA's Ares I. It uses drift regions to discourage
 * egregious propellant usage as well as hysteresis lines to reduce thruster
 * duty cycles (high-frequency on-off switching). For more details see
 * "Design and Stability of... Thrusters.pdf" in 02_GNC on the TREL drive.
 */

# ifndef RCS_CONTROLLER_HPP
# define RCS_CONTROLLER_HPP

#include <memory>
#include <stdint.h>

#include "Controller.hpp"
#include "Errors.h"

class RCSController final : public Controller
{
public:
    /**
     * RCS controller responds with signs.
     */
    typedef enum Response : int8_t
    {
        FIRST = -2,
        FIRE_NEGATIVE,
        NO_FIRE,
        FIRE_POSITIVE,
        LAST
    } Response_t;

    /**
     * Parameters defining phase channel geometry.
     */
    struct Config
    {
        /**
    	 * The maximum allowed magnitude of angular velocity.
    	 */
        float rateLimit;
        /**
    	 * Half the width of the angled (gradient) portion of the phase channel.
    	 */
        float deadband;
        /**
    	 * Ratio of lower rate limit : upper rate limit. Defines the height of
    	 * the drift channels.
    	 */
        float rateLimitsRatio;
        /**
    	 * Ratio of hysteresis upper angle limit : gradient angle limit. Defines
    	 * where the corners in the hysteresis lines lie between the rate axis
    	 * and the corners of the channel boundary.
    	 */
        float hysteresisGradientRatio;
        /**
    	 * Ratio of hysteresis rate limit : lower rate limit. Defines where the
    	 * hysteresis line parallel to the angle axis lies in the drift channel.
    	 */
        float hysteresisRateLimitRatio;
    };

    /**
      * Controllers begin with response NO_FIRE.
      *
      * @param   kConfig Configuration for phase channel geometry.
      */
     RCSController (const RCSController::Config& kConfig);

    /**
     * Validates the phase channel configuration provided at construction.
     *
     * @param   kValid    Result.
     *
     * @ret     E_SUCCESS Config successfully validated, though not necessarily
     *                    correct.
     */
    Error_t verifyConfig (bool& kValid);

    /**
     * Computes a new response based on current angle and rate.
     *
     * @ret     E_SUCCESS Computation succeeded.
     */
     Error_t runEnabled ();

     /**
      * No controller activity. Response is perpetually NO_FIRE.
      *
      * @ret     E_SUCCESS Run succeeded.
      */
     Error_t runSafed ();

    /**
     * Sets the angle to be used in the next response calculation.
     *
     * @param   kAngle            The current angle of the rocket in the axis
     *                            being controlled.
     *
     * @ret     E_SUCCESS         Successfully updated angle.
     *          E_NONFINITE_VALUE Provided angle was NaN or infinite. The
     *                            current angle remains unchanged, and the
     *                            current response becomes NO_FIRE.
     */
    Error_t setAngle (float kAngle);

    /**
     * Sets the rate to be used in the next response calculation.
     *
     * @param   kRate             The current angular velocity of the rocket
     *                            in the axis being controlled.
     *
     * @ret     E_SUCCESS         Successfully updated rate.
     *          E_NONFINITE_VALUE Provided rate was NaN or infinite. The
     *                            current rate remains unchanged, and the
     *                            current response becomes NO_FIRE.
     */
    Error_t setRate (float kRate);

    /**
     * Gets the response computed during the last run call.
     *
     * @param   kResponse      Result.
     *
     * @ret     E_SUCCESS      Access succeeded.
     *          E_INVALID_ENUM Current response was not an allowed value. It is
     *                         overridden to NO_FIRE. The result is also
     *                         populated with NO_FIRE.
     */
    Error_t getResponse (RCSController::Response_t& kResponse);

private:
    /**
     * Phase channel geometry configuration.
     */
    const Config mCONFIG;
	/**
	 * The last computed RCS response.
	 */
	RCSController::Response_t mCurrentResponse;
    /**
     * Current angle in the axis being controlled.
     */
    float mAngle;
    /**
     * Current angular velocity in the axis being controlled.
     */
    float mRate;

    /**
     * All of the following const floats are critical values defining the phase
     * channel geometry. These are computed from mCONFIG in the initializer
     * list.
     */

    /**
    * Magnitude of the upper drift channel bound on the rate axis.
    */
    const float mUPPER_RATE_LIMIT;
    /**
     * Magnitude of the lower drift channel bound on the rate axis.
     */
    const float mLOWER_RATE_LIMIT;
	/**
	 * Magnitude of points on either end of the channel gradient where the
	 * angled boundary meet the horizontal boundary.
	 */
	const float mGRADIENT_ANGLE_LIMIT;
	/**
	 * Magnitude of the horizontal hysteresis lines along the rate axis.
	 */
	const float mHYSTERESIS_RATE_LIMIT;
	/**
	 * Magnitude of points on the angle axis where the angled hysteresis lines
	 * meet the horizontal hysteresis lines.
	 */
	const float mHYSTERESIS_UPPER_ANGLE_LIMIT;
	/**
	 * Magnitude of points on the angle axis where the angled hysteresis lines
	 * meet the upper rate limits.
	 */
	const float mHYSTERESIS_LOWER_ANGLE_LIMIT;
	/**
	 * Slope of the channel gradient.
	 */
	const float mCHANNEL_GRADIENT;
	/**
	 * Point where lower hysteresis line intercepts the rate axis.
	 */
	const float mHYSTERESIS_INTERCEPT_LOW;
	/**
	 * Point where upper hysteresis line intercepts the rate axis.
	 */
	const float mHYSTERESIS_INTERCEPT_HIGH;

    /**
     * Updates the RCS response according to the currently set rate and angle.
     *
     * This method is only called when the controller is running in enabled
     * mode.
     *
     * @ret     E_SUCCESS Computation was successful.
     */
    Error_t computePhasePlaneMoment ();
};

# endif
