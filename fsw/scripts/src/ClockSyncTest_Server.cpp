/**
 * See ClockSyncTest_Config.hpp for instructions on running test.
 */

#include <iostream>

#include "Errors.hpp"
#include "ClockSync.hpp"
#include "NetworkManager.hpp"

#include "ClockSyncTest_Config.hpp"
#include "ClockSyncTest_Server.hpp"

void ClockSyncTest_Server::main (int ac, char** av)
{
    std::cout << "\nTEST START: " 
        << "Attempting to start server and sync with clients" << std::endl;

    // 1) Init Network Manager.
    NetworkManager::Config_t nmConfig = 
    {
        ClockSyncTest_Config::mNodes,
        ClockSyncTest_Config::mChannels,
        NetworkManager::Node_t::CONTROL_NODE,
    };
    std::shared_ptr<NetworkManager> pNm;
    Errors::exitOnError (NetworkManager::createNew (nmConfig, pNm),
                         "Failed to init Network Manager");

    // 2) Attempt to sync client with server.
    Errors::exitOnError(ClockSync::syncServer (
                                     pNm, 
                                     {NetworkManager::Node_t::DEVICE_NODE_0}),
                        "Failed to sync.");

    // 3) Print pass message.
    std::cout << "TEST PASSED: Clients sync'd to server successfully." 
        << std::endl;
}
