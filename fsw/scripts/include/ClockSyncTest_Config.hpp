/**
 * Integration test to verify ClockSync. 
 *
 * TO RUN THIS TEST:
 *
 * 0) Connect two sbRIO's to a switch.
 * 1) Update CONTROL_NODE_IP and DEVICE_NODE_IP below to reflect the static 
 *    IP's of the sbRIO's.
 * 2) Compile FlightSoftwareScript with ClockSyncTest_Server::main and load onto
 *    sbRIO designated as Control Node (CN).
 * 3) Compile FlightSoftwareScript with ClockSyncTest_Client::main and load onto
 *    sbRIO designated as Device Node 0 (DN0).
 * 4) ssh into DN0 and execute FlightSoftwareScript.
 * 5) ssh into CN and execute FlightSoftwareScript.
 * 6) On success, both scripts will print out TEST PASSED and exit.
 */

# ifndef CLOCK_SYNC_TEST_CONFIG_HPP
# define CLOCK_SYNC_TEST_CONFIG_HPP

#include <vector>

#include "NetworkManager.hpp"

/**
 * Control and Device Node IP's.
 */
#define CONTROL_NODE_IP "10.0.0.4"
#define DEVICE_NODE_IP  "10.0.0.1"

namespace ClockSyncTest_Config
{
    /**
     * Nodes used to initialize Network Manager.
     */
    extern std::unordered_map<NetworkManager::Node_t, 
                              NetworkManager::IP_t, 
                              EnumClassHash> mNodes;
    /**
     * Channels used to initialize Network Manager.
     */
    extern std::vector<NetworkManager::ChannelConfig_t> mChannels;
}

# endif
