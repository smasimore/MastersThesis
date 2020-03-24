/**
 * Entry point to execute the Control Node flight software. The entry () 
 * function initializes all fsw objects and creates the periodic thread, which
 * executes the loop () function.
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
 * Every loop, the Control Node sends out the following data:
 *
 *     1) A copy of DV_REG_CN_TO_DN0 to Device Node 0
 *     2) A copy of DV_REG_CN_TO_DN1 to Device Node 1
 *     3) A copy of DV_REG_CN_TO_DN2 to Device Node 2
 *     4) A copy of the entire Data Vector to Ground
 *
 * And attempts to receive the following data:
 *
 *     1) A copy of DV_REG_DN0_TO_CN from Device Node 0
 *     2) A copy of DV_REG_DN1_TO_CN from Device Node 1
 *     3) A copy of DV_REG_DN2_TO_CN from Device Node 2
 *     4) A copy of DV_REG_GND_TO_CN fom Ground
 *
 * The Network Manager configuration MUST include these 5 nodes and 4 channels.
 *
 *
 *            ---- REQUIRED DATA VECTOR REGIONS & ELEMENTS ----
 *
 *
 * Required Regions and Elements:
 *       
 *       DV_REG_CN
 *           DV_ELEM_STATE
 *           DV_ELEM_CN_TIME_NS
 *           DV_ELEM_CN_LOOP_COUNT
 *           DV_ELEM_CN_ERROR_COUNT
 *           DV_ELEM_DN0_RX_MISS_COUNT
 *           DV_ELEM_DN1_RX_MISS_COUNT
 *           DV_ELEM_DN2_RX_MISS_COUNT
 *       DV_REG_CN_TO_DN0
 *       DV_REG_CN_TO_DN1
 *       DV_REG_CN_TO_DN2
 *       DV_REG_DN0_TO_CN
 *       DV_REG_DN1_TO_CN
 *       DV_REG_DN2_TO_CN
 *       DV_REG_GND_TO_CN
 *
 *
 * NOTES:
 *
 *     #1 Globals are initialized in entry and used in the loop. Params to the 
 *        periodic loop are passed via globals because smart pointers passed via
 *        a void * intermediate will result in a copy of the smart ptr rather 
 *        than a reference increment, resulting in a double free of the memory
 *        if the loop exits.
 *
 *     #2 Because Controllers each have a differently-typed config, in order to 
 *        avoid hardcoding which Controllers are initialized in the entry 
 *        function, a controller initialization function must be provided by the
 *        caller.
 */

#ifndef CONTROL_NODE_HPP
#define CONTROL_NODE_HPP

#include <memory>
#include <vector>

#include "ThreadManager.hpp"
#include "NetworkManager.hpp"
#include "ClockSync.hpp"
#include "DataVector.hpp"
#include "CommandHandler.hpp"
#include "StateMachine.hpp"
#include "Controller.hpp"

namespace ControlNode
{
    /**
     * Function pointer type to pass to entry function for initializing 
     * Controllers. 
     *
     * @param  kPDv        Pointer to Data Vector.
     * @param  kPCtrlsRet  Vector to store pointers to Controllers.
     *
     * @ret    E_SUCCESS   Controllers initialized successfully.
     *         [other]     Controller initialization failed.
     */
    typedef Error_t (*fInitializeControllers_t) (
                         std::shared_ptr<DataVector> kPDv, 
                         std::vector<std::unique_ptr<Controller>>& kPCtrlsRet);

    /**
     * Entry point for the Control Node. Initializes all software components and 
     * begins periodic loop. Exits program on failure and does not return on
     * success.
     *
     * @param  kNmConfig          Network Manager config.
     * @param  kDvConfig          Data Vector config.
     * @param  kChConfig          Command Handler config.
     * @param  kSmConfig          State Machine config.
     * @param  kFInitControllers  Function pointer to controller init function.
     */
    void entry (NetworkManager::Config_t kNmConfig, 
                DataVector::Config_t     kDvConfig,
                CommandHandler::Config_t kChConfig,
                StateMachine::Config_t   kSmConfig,
                fInitializeControllers_t kFInitControllers);

};

# endif
