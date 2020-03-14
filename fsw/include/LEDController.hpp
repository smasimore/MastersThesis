/**
 * Controller for an LED connected to a DIO.
 */
# ifndef LED_CONTROLLER_HPP
# define LED_CONTROLLER_HPP

#include <memory>

#include "Controller.hpp"

class LEDController final : public Controller
{
public:
    /**
     * Controller configuration.
     */
    typedef struct Config
    {
        DataVectorElement_t dvElemControlVal;
    } Config_t;

    /**
     * Creates a new LEDController.
     *
     * @param   kConf         Controller config.
     * @param   kPDataVector  Ptr to initialized Data Vector.
     * @param   kDvModeElem   Data Vector element to read to determine 
     *                        controller's mode.
     */
    LEDController (const LEDController::Config_t& kConf, 
                   std::shared_ptr<DataVector> kPDataVector,
                   DataVectorElement_t kDvModeElem);

    /**
     * Validates controller config.
     *
     * @ret     E_SUCCESS         Config was valid.
     *          E_INVALID_ELEM    Elem in config dne in DV.
     */
    Error_t verifyConfig ();

    /**
     * LED is on when controller is enabled.
     *
     * @ret     E_SUCCESS            Success.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     */
    Error_t runEnabled ();

    /**
     * LED is off when controller is safed.
     *
     * @ret     E_SUCCESS            Success.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     */
    Error_t runSafed ();

private:

    /**
     * DV element to write to set LED's digital output value.
     */
    DataVectorElement_t mDvElemControlVal;

    /**
     * Writes an LED control value to the DV.
     *
     * @param   kControlVal          Control value to write.
     *
     * @ret     E_SUCCESS            Successfully set LED.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     */
    Error_t setLED (bool kControlVal);
};

# endif
