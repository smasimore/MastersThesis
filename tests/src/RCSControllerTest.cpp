#include <limits.h>
#include <map>
#include <math.h>
#include <iostream>
#include <utility>

#include "AvSWTestMacros.hpp"
#include "CppUTest/TestHarness.h"
#include "RCSController.hpp"

/**
 * Phase channel configuration used in tests. It is a known valid configuration,
 * being identical to that of NASA's Ares I. A visualization of the phase plane
 * it creates can be seen on page 5 of "Design and Stability of...
 * Thrusters.pdf" in 02_GNC, hereafter referred to as the RCS techdoc.
 */
static const RCSController::Config PHASE_CHANNEL_TEST_CONFIG =
{
    1.5,
    3.0,
    0.6,
    0.86,
    1.33
};

/**
 * Shape of the inner phase plane produced by the above channel configuration
 * with a prior NO_FIRE response (resulting in no hysteresis).
 *
 * In the final test, these planes are used as ground truth for controller
 * responses. This represents a potential bias, because these planes
 * themselves were produced by RCSController. To counteract this, the test prior
 * spot checks critical responses within the plane using values derived from the
 * RCS techdoc. The channel shown in the techdoc and the test phase channel are
 * identical and so should respond identically.
 */
static const std::string PHASE_CHANNEL_TEST_NO_HYSTERESIS =
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 - - - - - - - - - - - - - - \n"
"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 - - - - - - - - - - - - - - \n"
"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + 0 0 0 0 0 0 - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + + 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
"+ + + + + + + + + + + + + + + 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
"+ + + + + + + + + + + + + + + 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n";

/**
* Shape of the inner phase plane produced by the above channel configuration
* with nonzero prior responses (resulting in hysteresis and a thinner channel).
 */
static const std::string PHASE_CHANNEL_TEST_HYSTERESIS =
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n"
"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 - - - - - - - - - - - - - - - \n"
"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 - - - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + 0 0 0 - - - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + 0 0 0 0 - - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + 0 0 0 - - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + 0 0 0 - - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + 0 0 0 0 - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + 0 0 0 - - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + 0 0 0 0 - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + + 0 0 0 - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + + 0 0 0 - - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + + 0 0 0 0 - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + + + 0 0 0 - - - - - - - - - - - \n"
"+ + + + + + + + + + + + + + + + 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
"+ + + + + + + + + + + + + + + + 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n"
"+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + \n";

/**
 * (angle, rate) controller inputs.
 */
typedef std::pair<float, float> RCSInputs_t;

/**
 * Used in the final test to visualize the phase channel.
 */
static const std::map<RCSController::Response_t, std::string>
	gRESPONSE_SYMBOLS
{
    { RCSController::Response_t::FIRE_NEGATIVE, "- " },
    { RCSController::Response_t::NO_FIRE,       "0 " },
    { RCSController::Response_t::FIRE_POSITIVE, "+ " }
};

/**
 * Force a controller to have a zero response with inputs at the center of
 * the phase channel.
 *
 * @param   pController Controller to zero.
 *
 * @ret     E_SUCCESS   Success.
 */
Error_t makeZeroResponse(std::unique_ptr<RCSController>& pController)
{
    // Pass in (0, 0) which lies at the center of the channel
    Error_t err = pController->setAngle (0);

    if (err != E_SUCCESS)
    {
        return err;
    }

    err = pController->setRate (0);

    if (err != E_SUCCESS)
    {
        return err;
    }

    // Run the controller to compute the new response
    CHECK_SUCCESS (pController->run ());

    // Verify that the response is now zero
    RCSController::Response_t resp;
    CHECK_SUCCESS (pController->getResponse (resp));
    CHECK_TRUE (resp == RCSController::Response_t::NO_FIRE);

    return E_SUCCESS;
}

/**
 * Forces a controller to have a nonzero response with inputs well outside
 * the phase channel.
 *
 * @param   pController Controller to make nonzero.
 *
 * @ret     E_SUCCESS   Success.
 */
Error_t makeNonzeroResponse (std::unique_ptr<RCSController>& pController)
{
    // Pass in a state well beyond the boundaries of any rational channel
    // config
    Error_t err = pController->setAngle (1e9);

    if (err != E_SUCCESS)
    {
        return err;
    }

    err = pController->setRate (1e9);

    if (err != E_SUCCESS)
    {
        return err;
    }

    // Run the controller to compute the new response
    CHECK_SUCCESS (pController->run ());

    // Verify that the response is now nonzero
    RCSController::Response_t resp;
    CHECK_SUCCESS (pController->getResponse (resp));
    CHECK_TRUE (resp != RCSController::Response_t::NO_FIRE);

    return E_SUCCESS;
}

TEST_GROUP (RCSController)
{
};

/**
 * Creating a controller with a valid channel configuration, eliciting a nonzero
 * response, and then safing the controller causes its response to become
 * NO_FIRE.
 */
TEST (RCSController, NoFireOnDisableOrSafe)
{
    std::unique_ptr<RCSController> rcs;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        PHASE_CHANNEL_TEST_CONFIG, rcs);
    CHECK_EQUAL(E_SUCCESS, err);
    CHECK_SUCCESS (rcs->setMode (Controller::Mode_t::ENABLED));

    // Elicit an active response
    CHECK_SUCCESS (makeNonzeroResponse (rcs));
    RCSController::Response_t response;
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_TRUE (response != RCSController::Response_t::NO_FIRE);

    // Place controller in SAFED mode
    CHECK_SUCCESS (rcs->setMode (Controller::Mode_t::SAFED));
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_EQUAL (RCSController::Response_t::NO_FIRE, response);

    // Reenable
    CHECK_SUCCESS (rcs->setMode (Controller::Mode_t::ENABLED));

    // Elicit an active response
    CHECK_SUCCESS (makeNonzeroResponse (rcs));
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_TRUE (response != RCSController::Response_t::NO_FIRE);
}

/**
 * Configuring a bad rate limit fails.
 */
TEST (RCSController, ConfigBadRateLimit)
{
    RCSController::Config badConfig = PHASE_CHANNEL_TEST_CONFIG;
    std::unique_ptr<RCSController> rcs;

    // Nonfinite
    badConfig.rateLimit = NAN;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);

    // Finite, but outside allowed range
    badConfig.rateLimit = -1.5;
    err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);
}

/**
 * Configuring a bad deadband fails.
 */
TEST (RCSController, ConfigBadDeadband)
{
    RCSController::Config badConfig = PHASE_CHANNEL_TEST_CONFIG;
    std::unique_ptr<RCSController> rcs;

    // Nonfinite
    badConfig.deadband = NAN;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);

    // Finite, but outside allowed range
    badConfig.deadband = -1.5;
    err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);
}

/**
 * Configuring a bad rate limit ratio fails.
 */
TEST (RCSController, ConfigBadRateLimitRatio)
{
    RCSController::Config badConfig = PHASE_CHANNEL_TEST_CONFIG;
    std::unique_ptr<RCSController> rcs;

    // Nonfinite
    badConfig.rateLimitsRatio = NAN;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);

    // Finite, but outside allowed range
    badConfig.rateLimitsRatio = 1.5;
    err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);
}

/**
 * Configuring a bad hysteresis gradient ratio fails.
 */
TEST (RCSController, ConfigBadHysteresisGradientRatio)
{
    RCSController::Config badConfig = PHASE_CHANNEL_TEST_CONFIG;
    std::unique_ptr<RCSController> rcs;

    // Nonfinite
    badConfig.hysteresisGradientRatio = NAN;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);

    // Finite, but outside allowed range
    badConfig.hysteresisGradientRatio = 1.5;
    err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);
}

/**
 * Configuring a bad hysteresis rate limit ratio fails.
 */
TEST (RCSController, ConfigBadHysteresisRateLimitRatio)
{
    RCSController::Config badConfig = PHASE_CHANNEL_TEST_CONFIG;
    std::unique_ptr<RCSController> rcs;

    // Nonfinite
    badConfig.hysteresisRateLimitRatio = NAN;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);

    // Finite, but outside allowed range
    badConfig.hysteresisRateLimitRatio = 0.5;
    err = Controller::createNew<RCSController, RCSController::Config> (
        badConfig, rcs);
    CHECK_EQUAL(E_INVALID_CONFIG, err);
}

/**
 * Correct returns on both valid and invalid angle sets.
 */
TEST (RCSController, SetAngle)
{
    // Configure a new controller
    std::unique_ptr<RCSController> rcs;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        PHASE_CHANNEL_TEST_CONFIG, rcs);
    CHECK_EQUAL(E_SUCCESS, err);

    // Finite
    CHECK_SUCCESS (rcs->setAngle (10.0));
    // Infinite
    CHECK_EQUAL (E_NONFINITE_VALUE, rcs->setAngle (INFINITY));
    // NaN
    CHECK_EQUAL (E_NONFINITE_VALUE, rcs->setAngle (NAN));
}

/**
 * Correct returns on both valid and invalid rate sets.
 */
TEST (RCSController, SetRate)
{
    // Configure a new controller
    std::unique_ptr<RCSController> rcs;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        PHASE_CHANNEL_TEST_CONFIG, rcs);
    CHECK_EQUAL(E_SUCCESS, err);

    // Finite
    CHECK_SUCCESS (rcs->setRate (10.0));
    // Infinite
    CHECK_EQUAL (E_NONFINITE_VALUE, rcs->setRate (INFINITY));
    // NaN
    CHECK_EQUAL (E_NONFINITE_VALUE, rcs->setRate (NAN));
}

/**
 * Giving bad inputs to a controller with a nonzero response causes it to
 * zero its response.
 */
TEST (RCSController, NoFireFailsafes)
{
    // Configure and enable a new controller
    std::unique_ptr<RCSController> rcs;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        PHASE_CHANNEL_TEST_CONFIG, rcs);
    CHECK_EQUAL(E_SUCCESS, err);
    CHECK_SUCCESS (rcs->setMode (Controller::Mode_t::ENABLED));

    // Elicit an active response
    CHECK_SUCCESS (makeNonzeroResponse (rcs));
    RCSController::Response_t response;
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_TRUE (response != RCSController::Response_t::NO_FIRE);

    // Bad angle
    CHECK_SUCCESS (rcs->setRate (PHASE_CHANNEL_TEST_CONFIG.rateLimit * 2));
    CHECK_EQUAL (E_NONFINITE_VALUE, rcs->setAngle (NAN));
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_EQUAL (response, RCSController::Response_t::NO_FIRE);

    // Elicit an active response
    CHECK_SUCCESS (makeNonzeroResponse (rcs));
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_TRUE (response != RCSController::Response_t::NO_FIRE);

    // Bad rate
    CHECK_SUCCESS (rcs->setAngle (10));
    CHECK_EQUAL (E_NONFINITE_VALUE, rcs->setRate (NAN));
    CHECK_SUCCESS (rcs->getResponse (response));
    CHECK_EQUAL (response, RCSController::Response_t::NO_FIRE);
}

/**
 * Entering and exiting the phase channel and hysteresis regions from every
 * possible direction produces the correct response responses.
 *
 * The correct responses for this test were derived from the RCS techdoc.
 */
TEST (RCSController, PlaneResponses)
{
    // Configure and enable a new controller
    std::unique_ptr<RCSController> rcs;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        PHASE_CHANNEL_TEST_CONFIG, rcs);
    CHECK_EQUAL(E_SUCCESS, err);
    CHECK_SUCCESS (rcs->setMode (Controller::Mode_t::ENABLED));

    // Mapping of RCS inputs -> correct responses
    std::map<RCSInputs_t, RCSController::Response_t> noHysteresisTests =
    {
        // Positions within the channel and hysteresis lines
        { RCSInputs_t (0, 0),     RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (-6, 1.33), RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (6, -1.33), RCSController::Response_t::NO_FIRE       },
        // Positions in the channel but outside the hysteresis lines
        { RCSInputs_t (0, 1),     RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (0, -1),    RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (-6, 1),    RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (6, -1),    RCSController::Response_t::NO_FIRE       },
        // Positions above the channel
        { RCSInputs_t (-4, 2),    RCSController::Response_t::FIRE_NEGATIVE },
        { RCSInputs_t (8, -0.5),  RCSController::Response_t::FIRE_NEGATIVE },
        { RCSInputs_t (0, 2),     RCSController::Response_t::FIRE_NEGATIVE },
        // Positions below the channel
        { RCSInputs_t (4, -2),    RCSController::Response_t::FIRE_POSITIVE },
        { RCSInputs_t (-8, 0.5),  RCSController::Response_t::FIRE_POSITIVE },
        { RCSInputs_t (0, -2),    RCSController::Response_t::FIRE_POSITIVE }
    };

    // Run all I/O pairs
    for (const auto& test : noHysteresisTests)
    {
        const RCSInputs_t& input = test.first;
        const RCSController::Response_t& correctResponse = test.second;
        RCSController::Response_t response;

        // Zero the controller response so hysteresis is not enforced and pass
        // in the inputs
        CHECK_SUCCESS (makeZeroResponse (rcs));
        CHECK_SUCCESS (rcs->setAngle (input.first));
        CHECK_SUCCESS (rcs->setRate (input.second));
        CHECK_SUCCESS (rcs->run ());

        // Compare outputs
        CHECK_SUCCESS (rcs->getResponse (response));
        CHECK_EQUAL (correctResponse, response);
    }

    // Mapping of RCS inputs -> correct responses
    std::map<RCSInputs_t, RCSController::Response_t> hysteresisTests =
    {
        // Positions within the channel and hysteresis lines
        { RCSInputs_t (0, 0),     RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (-6, 1.33), RCSController::Response_t::NO_FIRE       },
        { RCSInputs_t (6, -1.33), RCSController::Response_t::NO_FIRE       },
        // Positions in the channel but outside the hysteresis lines
        { RCSInputs_t (0, 1),     RCSController::Response_t::FIRE_NEGATIVE },
        { RCSInputs_t (0, -1),    RCSController::Response_t::FIRE_POSITIVE },
        { RCSInputs_t (-6, 1),    RCSController::Response_t::FIRE_POSITIVE },
        { RCSInputs_t (6, -1),    RCSController::Response_t::FIRE_NEGATIVE },
        // Positions above the channel
        { RCSInputs_t (-4, 2),    RCSController::Response_t::FIRE_NEGATIVE },
        { RCSInputs_t (8, -0.5),  RCSController::Response_t::FIRE_NEGATIVE },
        { RCSInputs_t (0, 2),     RCSController::Response_t::FIRE_NEGATIVE },
        // Positions below the channel
        { RCSInputs_t (4, -2),    RCSController::Response_t::FIRE_POSITIVE },
        { RCSInputs_t (-8, 0.5),  RCSController::Response_t::FIRE_POSITIVE },
        { RCSInputs_t (0, -2),    RCSController::Response_t::FIRE_POSITIVE }
    };

    // Run all I/O pairs
    for (const auto& test : hysteresisTests)
    {
        const RCSInputs_t& input = test.first;
        const RCSController::Response_t& correctResponse = test.second;
        RCSController::Response_t response;

        // Cause the controller to produce a nonzero response so hysteresis
        // is enforced
        CHECK_SUCCESS (makeNonzeroResponse (rcs));
        CHECK_SUCCESS (rcs->setAngle (input.first));
        CHECK_SUCCESS (rcs->setRate (input.second));
        CHECK_SUCCESS (rcs->run ());

        // Compare outputs
        CHECK_SUCCESS (rcs->getResponse (response));
        CHECK_EQUAL (correctResponse, response);
    }
}

/**
 * Phase channel takes on the correct shape. This test builds two large strings
 * to visualize the shape of the phase channel with and without hysteresis, and
 * then compares them to the known correct shapes.
 */
TEST (RCSController, PhaseChannelShape)
{
    // Generated planes are 30 responses x 30 responses
    const int PLANE_DIMENSIONS = 30;
    // Angle and rate bounds of the plane being visualized. These bounds were
    // chosen so that the channel is centered well within the visualized plane.
    const float PLANE_RATE_LOW =   -PHASE_CHANNEL_TEST_CONFIG.rateLimit * 2;
    const float PLANE_RATE_HIGH =  -PLANE_RATE_LOW;
    const float PLANE_ANGLE_LOW =  -PLANE_DIMENSIONS / 2;
    const float PLANE_ANGLE_HIGH = -PLANE_ANGLE_LOW;
    // Axis intervals. Chosen so that the plane is high enough resolution to
    // provide a good visual of the channel shape.
    const float PLANE_RATE_STEP = (PLANE_RATE_HIGH - PLANE_RATE_LOW)
                                  / PLANE_DIMENSIONS;
    const float PLANE_ANGLE_STEP =  1;

    // Configure and enable a new controller
    std::unique_ptr<RCSController> rcs;
    Error_t err = Controller::createNew<RCSController, RCSController::Config> (
        PHASE_CHANNEL_TEST_CONFIG, rcs);
    CHECK_EQUAL(E_SUCCESS, err);
    CHECK_SUCCESS (rcs->setMode (Controller::Mode_t::ENABLED));

    // Visualization strings to be built
    std::string planeNoHysteresis;
    std::string planeHysteresis;

    // Generate responses for every interval on both axes
    for (float rate = PLANE_RATE_HIGH; rate >= PLANE_RATE_LOW;
         rate -= PLANE_RATE_STEP)
    {
        for (float angle = PLANE_ANGLE_LOW; angle < PLANE_ANGLE_HIGH;
             angle += PLANE_ANGLE_STEP)
        {
            RCSController::Response_t response;

            // Cause an initial NO_FIRE response so hysteresis is not enforced
            CHECK_SUCCESS (makeZeroResponse (rcs));
            // Provide the current point in the plane and fetch response
            CHECK_SUCCESS (rcs->setAngle (angle));
            CHECK_SUCCESS (rcs->setRate (rate));
            CHECK_SUCCESS (rcs->run ());
            CHECK_SUCCESS (rcs->getResponse (response));

            planeNoHysteresis.append (gRESPONSE_SYMBOLS.at(response));

            // Cause a nonzero response so hysteresis is enforced
            CHECK_SUCCESS (makeNonzeroResponse (rcs));
            // Provide the current point in the plane and fetch response
            CHECK_SUCCESS (rcs->setAngle (angle));
            CHECK_SUCCESS (rcs->setRate (rate));
            CHECK_SUCCESS (rcs->run ());
            CHECK_SUCCESS (rcs->getResponse (response));

            planeHysteresis.append (gRESPONSE_SYMBOLS.at(response));
        }

        // Each inner loop populates a row of the phase plane, so we append
        // newlines to aid visualization
        planeNoHysteresis.append ("\n");
        planeHysteresis.append ("\n");
    }

    // Generated planes should be identical to the known correct shapes
    STRCMP_EQUAL (PHASE_CHANNEL_TEST_NO_HYSTERESIS.c_str (),
                  planeNoHysteresis.c_str ());
    STRCMP_EQUAL (PHASE_CHANNEL_TEST_HYSTERESIS.c_str (),
                  planeHysteresis.c_str ());
}
