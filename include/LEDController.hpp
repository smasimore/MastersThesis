/**
 * Controller for an LED connected to a DIO.
 */
# ifndef LED_CONTROLLER_HPP
# define LED_CONTROLLER_HPP

#include <memory>

#include "Controller.hpp"
#include "DigitalOutDevice.hpp"
#include "NiFpga.h"

class LEDController final : public Controller
{
public:
    /**
     * Controller configuration.
     */
    typedef struct Config
    {
        StateVectorElement_t svElemControlVal;
    } Config_t;

    /**
     * Creates a new LEDController.
     *
     * @param   kConf         Controller config.
     * @param   kPStateVector Ptr to initialized State Vector.
     * @param   kSvModeElem   State Vector element to read to determine 
     *                        controller's mode.
     */
    LEDController (const LEDController::Config_t& kConf, 
                   std::shared_ptr<StateVector> kPStateVector,
                   StateVectorElement_t kSvModeElem);

    /**
     * Validates controller config.
     *
     * @ret     E_SUCCESS         Config was valid.
     *          E_INVALID_ELEM    Elem in config dne in SV.
     */
    Error_t verifyConfig ();

    /**
     * LED is on when controller is enabled.
     *
     * @ret     E_SUCCESS            Success.
     *          E_STATE_VECTOR_WRITE Failed to write to SV.
     */
    Error_t runEnabled ();

    /**
     * LED is off when controller is safed.
     *
     * @ret     E_SUCCESS            Success.
     *          E_STATE_VECTOR_WRITE Failed to write to SV.
     */
    Error_t runSafed ();

private:

    /**
     * SV element to write to set LED's digital output value.
     */
    StateVectorElement_t mSvElemControlVal;

    /**
     * Writes an LED control value to the SV.
     *
     * @param   kControlVal          Control value to write.
     *
     * @ret     E_SUCCESS            Successfully set LED.
     *          E_STATE_VECTOR_WRITE Failed to write to SV.
     */
    Error_t setLED (bool kControlVal);
};

# endif
