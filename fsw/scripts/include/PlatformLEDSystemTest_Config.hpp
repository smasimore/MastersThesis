/**
 * System-level test for Platform. Exercises all components of the Platform.
 *
 *                       ---- Experiment Setup ----
 *
 * 1)  Connect LED's to the following Device Nodes pins (the Device Node grounds 
 *     must be used to complete the circuits):
 *       
 *       DEVICE NODE 0: DIO 5, 7, 9, 11, 13
 *       DEVICE NODE 1: DIO 5, 7, 9, 11, 13
 *       DEVICE NODE 2: DIO 5
 *
 * 2)  Run following command on Linux-based Ground Node to allow UDP messages 
 *     through firewall:
 *     "sudo iptables -I INPUT -i <Flight Network Interface> -p udp -j ACCEPT"
 * 3)  Connect four sbRIO's and ground computer to the switch.
 * 4)  Set the IP and NUM_RUNS macros.
 * 5)  Compile Control Node binary using Script build config.
 * 6)  Compile Device Node binaries, with relevant device node set in 
 *     DEVICE_NODE_TO_COMPILE macro and using Script build config.
 * 7)  Compile Ground Node binary using Script_x86 build config.
 * 8)  Start Ground Node binary on x86 computer
 * 9)  Start Device Node binaries on 9637 sbRIO's.
 * 10) Start Control Node binary on 9637 or 9627 sbRIO.
 */

#ifndef PLATFORM_LED_SYSTEM_TEST_CONFIG_HPP
#define PLATFORM_LED_SYSTEM_TEST_CONFIG_HPP

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

namespace PlatformLEDSystemTest_Config
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
     * Ground Node Data Vector config. 
     */
    extern DataVector::Config_t mGndDvConfig;

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
     * Ground Node Network Manager config. 
     */
    extern NetworkManager::Config_t mGndNmConfig;

    /**
     * Command Handler config.
     */
    extern CommandHandler::Config_t mChConfig;
}

#endif
