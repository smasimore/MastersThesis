/**
 * Entry point to execute Device Node flight software. The entry () function 
 * initializes all fsw objects and creates a thread that executes the loop () 
 * function.
 *
 *
 *           ---- FLIGHT NETWORK TOPOLOGY AND DATA FLOW ----
 * 
 *
 * The current implementation follows the Platform v1 design, with the
 * following network configuration:
 *
 *                       Device Node 0
 *                         /
 *                        /
 * Ground --- Control Node --- Device Node 1
 *                        \
 *                         \
 *                       Device Node 2
 *
 * Every loop, each Device Node x blocks until it receives a copy of 
 * DV_REG_CN_TO_DNx. This synchronizes the Device Node x loop to the Control
 * Node Loop. After unblocking, each Device Node sends a copy of 
 * DV_REG_DNx_TO_CN to the Control Node.
 *
 * The Network Manager configuration MUST support this topology.
 *
 *
 *            ---- REQUIRED DATA VECTOR REGIONS & ELEMENTS ----
 *
 *
 * Required Regions and Elements:
 *       
 *       DV_REG_CN_TO_DNx
 *       DV_REG_DNx_TO_CN
 *           DV_ELEM_DNx_LOOP_COUNT
 *           DV_ELEM_DNx_ERROR_COUNT
 *
 *
 * NOTES:
 *
 *     #1 Globals are initialized in entry and used in the loop. Params to the 
 *        loop thread are passed via globals because smart pointers passed via
 *        a void * intermediate will result in a copy of the smart ptr rather 
 *        than a reference increment, resulting in a double free of the memory
 *        if the loop exits.
 *
 *     #2 Because Controllers and Devices each have a differently-typed config, 
 *        in order to avoid hardcoding which Controllers and Devices are 
 *        initialized in the entry function, an initialization function must be 
 *        provided by the caller.
 *
 *     #3 Low-level Controllers (e.g. a fin PID Controller) that require quick
 *        reaction time are intended to be run on Device Nodes. High-level
 *        Controllers that do not require as quick of a reaction time (e.g. GNC 
 *        Controller) are intended to run on the Control Node.
 */

#ifndef DEVICE_NODE_HPP
#define DEVICE_NODE_HPP

#include <memory>
#include <vector>

#include "ThreadManager.hpp"
#include "NetworkManager.hpp"
#include "ClockSync.hpp"
#include "Time.hpp"
#include "DataVector.hpp"
#include "FPGASession.hpp"
#include "Controller.hpp"
#include "Device.hpp"

namespace DeviceNode
{
    /**
     * Function pointer type to pass to entry function for initializing 
     * Controllers and Devices.  
     *
     * @param  kPDv            Pointer to Data Vector.
     * @param  kFpgaSession    FPGA session.
     * @param  kPCtrls         Vector of pointers to Controllers.
     * @param  kPSensorDevs    Vector of pointers to sensor Devices.
     * @param  kPActuatorDevs  Vector of pointers to actuator Devices.
     *
     * @ret    E_SUCCESS  Initialized successfully.
     *         [other]    Initialization failed.
     */
    typedef Error_t (*fInitializeCtrlsAndDevs_t) (
                         std::shared_ptr<DataVector> kPDv, 
                         NiFpga_Session& kFpgaSession,
                         std::vector<std::unique_ptr<Controller>>& kPCtrls,
                         std::vector<std::unique_ptr<Device>>& kPSensorDevs,
                         std::vector<std::unique_ptr<Device>>& kPActuatorDevs);

    /**
     * Entry point for the Device Nodes. Initializes all software components and 
     * begins loop. Exits program on failure and does not return on success.
     *
     * @param  kNmConfig           Network Manager config.
     * @param  kDvConfig           Data Vector config.
     * @param  kFInitCtrlsAndDevs  Function pointer to Controller and Device 
     *                             initialization function.
     *
     * FOR TESTING PURPOSES ONLY -- MUST BE FALSE FOR ALL FLIGHT SOFTWARE
     *
     * @param  kSkipClockSync      Skip clock synchronization step to enable
     *                             single sbRIO unit testing.
     */
    void entry (NetworkManager::Config_t  kNmConfig, 
                DataVector::Config_t      kDvConfig,
                fInitializeCtrlsAndDevs_t kFInitCtrlsAndDevs,
                bool                      kSkipClockSync);

};

# endif
