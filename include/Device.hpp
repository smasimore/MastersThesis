/**
 * Base device class for implementing sensors and actuators. Device objects are 
 * the interface between the State Vector and the FPGA API.
 *
 * PRE-CONDITIONS
 * #1 The FPGA must be initialized and a session opened before initializing any 
 *    devices. This only needs to be done once for the entire program and not per 
 *    device. The FPGA takes ~1 second to fully initialize even though the 
 *    initialize/open methods return. The caller must sleep to ensure 
 *    initialization completes before initializing any devices.
 *
 * POST-CONDITIONS
 * #1 The FPGA session must be closed and finalized after all devices are no
 *    longer in use.
 *
 * IMPLEMENTING A DEVICE
 *
 * 1. Extend the Device base class to create YourDevice.
 * 2. Define struct Config_t, which will contain any device-specific
 *    configuration (e.g. which DIO pin to use) and will be passed to
 *    YourDevice on initialization.
 * 3. Implement the constructor
 *        YourDevice (kSession, kPStateVector, kConfig, kRet));
 * 4. Implement the run method, which will be called periodically.
 *
 * See LEDDevice for an example.
 *
 * USING A DEVICE
 *
 * 1. Call Device::createNew<YourDevice,
 *                           struct YourDevice::YourConfig> with an initialized
 *    FPGA session, State Vector, the relevant device config data, and a 
 *    unique_ptr to store the initialized device pointer in.
 * 2. Call YourDevice->run () for each loop of your main periodic thread.
 *
 * WARNINGS:
 *
 *     #1 In the current FPGA configuration, on initialization the DIO pins are
 *        set as inputs and have a floating value. Qualitatively, this floating
 *        value is ~.7 volts. Once the pin is set to be an output pin, this goes
 *        to 0 or 3.3V, depending on how it is set.
 *
 *     #2 After initializing an FPGA session, the process must wait for at least
 *        1 second for it to complete initializaion before initializing any
 *        devices. Otherwise FPGA commands (e.g. setting a pin as an output in 
 *        with a low value) will be delayed until the FPGA initialization is 
 *        complete. This can result in the pin reading high even though the 
 *        Device has been initialized with a low value.
 */

#ifndef aDEVICE_HPP
#define aDEVICE_HPP

#include <stdint.h>
#include <memory>

#include "Errors.h"
#include "NiFpga.h"
#include "StateVector.hpp"

class Device
{

    public:

        /**
         * Entry point for creating a new device object. Any config validation
         * must be done by the specific device constructor. Defined in the 
         * header so that the templatized functions do not need to each be 
         * instantiated explicitly.
         *
         * @param   kSession            Initialized and open FPGA session.
         * @param   kStateVector        Initialized State Vector for this node.
         * @param   kConfig             Device's config data.
         * @param   kPDeviceRet         Pointer to return device.
         *
         * @ret    E_SUCCESS            Device successfully created.
         *         E_STATE_VECTOR_NULL  State Vector pointer null.
         *         [other]              Constructor error returned by device.
        */
        template <class T_Device, class T_Config>
        static Error_t createNew (NiFpga_Session& kSession, 
                                  std::shared_ptr<StateVector> kPStateVector, 
                                  T_Config kConfig,
                                  std::unique_ptr<T_Device>& kPDeviceRet)
        {
            Error_t ret = E_SUCCESS;

            // Verify State Vector param not null.
            if (kPStateVector == nullptr)
            {
                return E_STATE_VECTOR_NULL;
            }

            // Create device.
            kPDeviceRet.reset (new T_Device (kSession, kPStateVector, kConfig, 
                               ret));
            if (ret != E_SUCCESS)
            {
                kPDeviceRet.reset (nullptr);
                return ret;
            }

            return E_SUCCESS;
        }


        /**
         * Run device logic once.
         *
         * @ret     E_SUCCESS   Device ran successfully.
         *          [other]     Error returned by device.
         */
        virtual Error_t run () = 0;;

    protected:

        /**
         * FPGA session.
         */
        NiFpga_Session& mSession;

        /**
         * State Vector.
         */
        std::shared_ptr<StateVector> mPStateVector;

        /**
         * Constructor. Must be called by derived device constructors.
         */
        Device (NiFpga_Session& kSession, std::shared_ptr<StateVector> kPStateVector);
};

#endif
