/**
 * Device to control an analog input pin. Constructor configures the input mode
 * (RSE vs. differential) and input range (+/- 10V, 5V, 2V, or 1V) of the pin.
 * Run method reads the voltage on the pin, converts this voltage to an
 * engineering unit with the device's transfer function, and then writes this
 * voltage and engineering unit to the Data Vector.
 *
 * NOTES:
 *
 *   (1) For a pair of differential pins (X, Y), the FPGA will treat the voltage
 *       on min(X, Y) as the "real" signal. The consequence of this is that the
 *       FPGA will read negated values on differential pins >= 8 since the
 *       "real" signal is on the unused partner pin at 0V. For this reason,
 *       differential AnalogInDevices may only be configured on pins < 8.
 *
 *   (2) In a static test setting with minimal EMI, AnalogInDevice voltage
 *       measurements were accurate to around +/- 0.005V.
 */

#ifndef ANALOG_IN_DEVICE_HPP
#define ANALOG_IN_DEVICE_HPP

#include "Device.hpp"
#include "NiFpga_IO.h"

class AnalogInDevice final : public Device
{
public:
    /**
     * Function for converting voltage to engineering unit.
     *
     * @param   kV        Voltage.
     * @param   kEngr     Engineering unit return.
     *
     * @ret     E_SUCCESS Voltage to engineering unit conversion succeeded.
     *          [other]   Error during conversion. 
     */
    typedef Error_t TransferFunc_t (float kV, float& kEngrRet);

    /**
     * Analog in mode. The values of this enum correspond to constants in the
     * FPGA API.
     */
    typedef enum : uint8_t
    {
        MODE_DIFF = 0, // Differential (2 pins)
        MODE_RSE  = 1, // Referenced single-ended (1 pin)

        MODE_LAST
    } Mode_t;
    
    /**
     * Analog in voltage range. The values of this enum correspond to constants
     * in the FPGA API.
     */
    typedef enum : uint8_t
    {
        RANGE_10V = 0,
        RANGE_5V  = 1,
        RANGE_2V  = 2,
        RANGE_1V  = 3,

        RANGE_LAST
    } Range_t;

    /**
     * Min and max analog in pin numbers supported by sbRIO.
     */
    static const uint8_t MIN_PIN_NUMBER;
    static const uint8_t MAX_PIN_NUMBER;

    /**
     * Device configuration.
     */
    typedef struct
    {
        DataVectorElement_t dvElemOutputVolts; // Elem to write voltage to.
        DataVectorElement_t dvElemOutputEngr;  // Elem to write engr unit to.
        uint8_t pinNumber;                     // Device pin number.
        TransferFunc_t* pTransferFunc;         // Voltage -> engr unit func.
        Range_t range;                         // Input voltage range.
        Mode_t mode;                           // Differential or RSE.
    } Config_t;

    /**
     * Derived device constructor. This must be public so that it is visible to
     * Device::createNew.
     *
     * @param   kSession    Initialized and open FPGA session.
     * @param   kDataVector Node's Data Vector.
     * @param   kConfig     Device config.
     * @param   kRet        E_SUCCESS            Config valid.
     *                      E_OUT_OF_BOUNDS      Pin number out of bounds.
     *                      E_INVALID_ENUM       Invalid range or mode.
     *                      E_INVALID_ELEM       Output elems not in DV or wrong
     *                                           type.
     *                      E_INVALID_POINTER    Null transfer func pointer.
     *                      E_FPGA_WRITE         Failed to configure pin.
     *                      E_PIN_NOT_CONFIGURED Differential mode is only for
     *                                           devices on pins < 8. See note
     *                                           (1) at the top of this file.
     */
    AnalogInDevice (NiFpga_Session& kSession,
                    std::shared_ptr<DataVector> kPDataVector,
                    Config_t& kConfig,
                    Error_t& kRet);

    /**
     * Validates the device config.
     *
     * @param   kConfig              Config to validate.
     *
     * @ret     E_SUCCESS            Config valid.
     *          E_OUT_OF_BOUNDS      Pin number out of bounds.
     *          E_INVALID_ENUM       Invalid range or mode.
     *          E_INVALID_ELEM       Output elems not in DV or wrong type.
     *          E_INVALID_POINTER    Null transfer func pointer.
     *          E_PIN_NOT_CONFIGURED Differential mode is only for pins < 8.
     *                               See note (1) at the top of this file.
     */
    Error_t verifyConfig (Config_t& kConfig);
    
    /**
     * Reads input voltage, converts to engineering unit, and writes
     * engineering unit to DV.
     *
     * @ret     E_SUCCESS           Run succeeded.
     *          E_FPGA_READ         Failed to read from FPGA.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     *          [other]             Error generated by transfer function.
     */
    Error_t run ();

private:
    /**
     * DV element to which device output in volts is written.
     */
    DataVectorElement_t mDvElemOutputVolts;

    /**
     * DV element to which device output in engineering units is written.
     */
    DataVectorElement_t mDvElemOutputEngr;

    /**
     * Transfer function for converting voltage to engineering unit.
     */
    TransferFunc_t* mPTransferFunc;

    /**
     * Pin fxp resource identifier.
     */
    uint32_t mFxpResourceId;

    /**
     * Pin fxp type info identifier.
     */
    NiFpga_FxpTypeInfo mFxpTypeInfoId;
};

#endif
