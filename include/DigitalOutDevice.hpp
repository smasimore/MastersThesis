/**
 * Device to control a digital output pin. Constructor configures FPGA pin to 
 * be output. Run method first reads control value State Vector element 
 * (configured in  Config_t) to determine value to set on pin and second reads 
 * FPGA pin value and stores in feedback value State Vector element (configured 
 * in Config_t).
 *
 * WARNINGS:
 *
 *     #1 This Device will only work for code running on an sbRIO-9637. The 9627
 *        without the RMC connector only supports DIO 0 - 3.
 */

#ifndef DIGITAL_OUT_DEVICE_HPP
#define DIGITAL_OUT_DEVICE_HPP

#include "Device.hpp"

class DigitalOutDevice final : public Device
{

    public:

        /**
         * Min digital pin number supported by sbRIO.
         *
         * Note: Once all DIO configured in FPGA this can be removed.
         */
        static const uint8_t MIN_PIN_NUMBER;

        /**
         * Max digital pin number supported by sbRIO.
         */
        static const uint8_t MAX_PIN_NUMBER;

        /**
         * Device configuration. 
         */
        typedef struct Config
        {
            StateVectorElement_t svElemControlVal;
            StateVectorElement_t svElemFeedbackVal;
            uint8_t pinNumber;
        } Config_t;

        /**
         * Sets digital output value and reads current pin value.
         *
         * @ret   E_SUCCESS             Device ran successfully.
         *        E_STATE_VECTOR_READ   Failed to read from SV.
         *        E_STATE_VECTOR_WRITE  Failed to write to SV.
         *        E_FPGA_READ           Failed to read from FPGA.
         *        E_FPGA_WRITE          Failed to write to FPGA.
         */
        virtual Error_t run ();

        /**
         * Verify config is valid.
         *
         * @param kConfig               Config to verify.
         *
         * @ret   E_SUCCESS             Config valid.
         *        E_OUT_OF_BOUNDS       pinNumber out of bounds.
         *        E_INVALID_ELEM        svElemControlVal or svElemFeedbackVal 
         *                              not in SV.
         */
        Error_t verifyConfig (Config_t& kConfig);

        /**
         * Derived device constructor. This must be public so that it is visible 
         * to Device::createNew.
         *
         * @param  kSession      Initialized and open FPGA session.
         * @param  kStateVector  Node's State Vector.
         * @param  kConfig       Device config.
         * @param  kRet          E_SUCCESS             Config valid.
         *                       E_OUT_OF_BOUNDS       pin out of bounds.
         *                       E_INVALID_ELEM        svElemControlVal or 
         *                                             svElemFeedbackVal not in 
         *                                             SV.
         *                       E_STATE_VECTOR_READ   Failed to read from SV.
         *                       E_FPGA_WRITE          Failed to write to FPGA.
         *                       E_PIN_NOT_CONFIGURED  Pin not configured.
         */
        DigitalOutDevice (NiFpga_Session& kSession, 
                          std::shared_ptr<StateVector> kPStateVector,
                          Config_t& kConfig,
                          Error_t& kRet);

    private:

        /**
         * SV element to read to determine digital output value to set in run
         * method.
         */
        StateVectorElement_t mSvElemControlVal;

        /**
         * SV element to write digital pin feedback value to.
         */
        StateVectorElement_t mSvElemFeedbackVal;

        /**
         * FPGA control identifier to set.
         */
        uint32_t mFpgaControl;

        /**
         * FPGA indicator identifier to read.
         */
        uint32_t mFpgaIndicator;

        /**
         * Read control value from State Vector and write to FPGA.
         *
         * @ret   E_SUCCESS             Device ran successfully.
         *        E_STATE_VECTOR_READ   Failed to read from SV.
         *        E_FPGA_WRITE          Failed to write to FPGA.
         */
        Error_t updateFpgaControlValue ();
};

#endif
