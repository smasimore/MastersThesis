/**
 * ProfilePlatformX scripts are a set of 4 experiments to measure Platform 
 * performance with regards to network communications, jitter, reaction time,
 * and overall Platform CPU overhead. Each experiment runs on a Control Node and
 * 3 Device Nodes. A Ground Node is not required. The config data in this file
 * are shared across the 4 scripts.
 *
 *     #1 ProfilePlatformComms: Measure Platform v1 message miss, reorder, and 
 *        drop rates with the maximum system data load. This means using the 
 *        largest allowed data receive message size (1024). The following is
 *        measured:
 *
 *             Message Miss Rate:    # of messages that missed their segment 
 *                                   deadline / # of received messages.
 *             Message Reorder Rate: # of messages received out-of-order from a 
 *                                   Device Node / # of received messages.
 *             Message Drop Rate:    # of messages we expected to receive but 
 *                                   never did / # of expected messages. This
 *                                   metric also includes the case that a
 *                                   CN --> DN message gets stuck in the DN RX 
 *                                   queue (which is not technically a msg 
 *                                   drop).
 *
 *        Maximum NUM_RUNS: No max since memory usage does not increase with
 *                          NUM_RUNS.
 *
 *     #2 ProfilePlatformJitter: Measure Platform v1 jitter. Jitter in this
 *        experiment is measured by comparing run over run when the first 
 *        Controller runs on the Control Node, and when the first Sensor runs on 
 *        the Device Nodes. In a perfect world, run over run would be exactly
 *        10ms apart. Jitter measures the actual variation.
 *
 *        Maximum NUM_RUNS: An int64_t is stored per run, so the max recommended
 *                          NUM_RUNS is 10k.
 *
 *     #3 ProfilePlatformRxnTime: Measure Platform v1 reaction time for:
 *         a) A Controller running on the Control Node with dependent sensors & 
 *            actuators on Device Nodes, and
 *         b) a Controller running on a Device Node with dependent sensors &
 *            actuators running on that same Device Node.
 *        Results printed by each Device Node.
 *
 *        Maximum NUM_RUNS: An uint64_t is stored per run, so the max 
 *                          recommended NUM_RUNS is 10k.
 *
 *     #4 ProfilePlatformOverhead: Measure wall time and CPU process time 
 *        available to the control logic. Platform overhead for Control and
 *        Device Nodes is derived from this. Does not use NUM_RUNS.
 *
 *                       ---- Experiment Setup ----
 * 
 * 1) Connect four sbRIO's to the switch.
 * 2) Set the IP and NUM_RUNS macros.
 * 3) Compile Control Node binary.
 * 4) Compile Device Node binaries, with relevant device node set in 
 *    DEVICE_NODE_TO_COMPILE macro.
 * 5) Start Device Node binaries.
 * 6) Start Control Node binary.
 *
 */

#ifndef PROFILE_PLATFORM_CONFIG_HPP
#define PROFILE_PLATFORM_CONFIG_HPP

#include "DataVector.hpp"
#include "NetworkManager.hpp"
#include "CommandHandler.hpp"
#include "Controller.hpp"
#include "StateMachine.hpp"

#include "ProfileHelpers.hpp"

/**
 * Macros to set.
 */
#define DEVICE_NODE_TO_COMPILE  DEVICE_NODE2
#define NUM_RUNS                8640000
#define DEVICE_NODE0_IP         "10.0.0.1"
#define DEVICE_NODE1_IP         "10.0.0.2"
#define DEVICE_NODE2_IP         "10.0.0.3"
#define CONTROL_NODE_IP         "10.0.0.4"
#define GROUND_NODE_IP          "10.0.0.99"

/**
 * Preprocessor directives cannot be compared to enums, so redefine Device Node 
 * enums as directives.
 */
#define DEVICE_NODE0            1
#define DEVICE_NODE1            2
#define DEVICE_NODE2            3

namespace ProfilePlatform_Config
{

    /**
     * Data Vector Region to send from Control Node to Device Node 0.
     */
    extern DataVector::RegionConfig_t mDvRegCnToDn0;

    /**
     * Data Vector Region to send from Control Node to Device Node 1.
     */
    extern DataVector::RegionConfig_t mDvRegCnToDn1;

    /**
     * Data Vector Region to send from Control Node to Device Node 2.
     */
    extern DataVector::RegionConfig_t mDvRegCnToDn2;

    /**
     * Data Vector Region to send from Device Node 0 to Control Node.
     */
    extern DataVector::RegionConfig_t mDvRegDn0ToCn;

    /**
     * Data Vector Region to send from Device Node 1 to Control Node.
     */
    extern DataVector::RegionConfig_t mDvRegDn1ToCn;

    /**
     * Data Vector Region to send from Device Node 2 to Control Node.
     */
    extern DataVector::RegionConfig_t mDvRegDn2ToCn;

    /**
     * Data Vector Region to send from Ground Node to Control Node.
     */
    extern DataVector::RegionConfig_t mDvRegGndToCn;

    /**
     * Control Node Data Vector config.
     */
    extern DataVector::Config_t mCnDvConfig;

    /**
     * Device Node Data Vector config. 
     */
    extern DataVector::Config_t mDnDvConfig;

    /**
     * Nodes used to initialize Network Manager.
     */
    extern std::unordered_map<Node_t, 
                              NetworkManager::IP_t, 
                              EnumClassHash> mNodes;

    /**
     * Channels used to initialize Network Manager.
     */
    extern std::vector<NetworkManager::ChannelConfig_t> mChannels;

    /**
     * Control Node Network Manager config. 
     */
    extern NetworkManager::Config_t mCnNmConfig;

    /**
     * Device Node Network Manager config. 
     */
    extern NetworkManager::Config_t mDnNmConfig;

    /**
     * Command Handler config.
     */
    extern CommandHandler::Config_t mChConfig;
}

#endif
