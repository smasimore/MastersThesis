/**
 * See ClockSyncTest_Config.hpp for instructions on running test.
 */

#include <iostream>

#include "Errors.hpp"
#include "ClockSync.hpp"
#include "NetworkManager.hpp"

#include "ClockSyncTest_Config.hpp"
#include "ClockSyncTest_Client.hpp"

void ClockSyncTest_Client::main (int ac, char** av)
{
    std::cout << "TEST START: Attempting to sync client to server" << std::endl;

    // 1) Init Network Manager. 
    NetworkManager::Config_t nmConfig = 
    {
        ClockSyncTest_Config::mNodes,
        ClockSyncTest_Config::mChannels,
        NetworkManager::Node_t::DEVICE_NODE_0,
    };
    std::shared_ptr<NetworkManager> pNm;
    Errors::exitOnError (NetworkManager::createNew (nmConfig, pNm),
                         "Failed to init Network Manager");

    // 2) Attempt to sync client with server.
    Errors::exitOnError(ClockSync::syncClient (
                                         pNm, 
                                         NetworkManager::Node_t::CONTROL_NODE, 
                                         CONTROL_NODE_IP),
                        "Failed to sync.");

    // 3) Print pass message.
    std::cout << "TEST PASSED: Client sync successful." << std::endl;
}
