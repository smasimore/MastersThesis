/**
 * See ClockSyncTest_Config.hpp for instructions on running test.
 */

#include <iostream>

#include "Errors.hpp"
#include "ClockSync.hpp"
#include "NetworkManager.hpp"

#include "ClockSyncTest_Config.hpp"
#include "ClockSyncTest_Client.hpp"

const uint32_t EXPECTED_MSG_TX_COUNT = 1;
const uint32_t EXPECTED_MSG_RX_COUNT = 1;

void ClockSyncTest_Client::main (int ac, char** av)
{
    std::cout << "TEST START: Attempting to sync client to server" << std::endl;

    // 1) Init Data Vector with message counters.
    DataVector::Config_t dvConfig =
    {
        {DV_REG_TEST0,
        {
            DV_ADD_UINT32 (DV_ELEM_TEST0, 0),
            DV_ADD_UINT32 (DV_ELEM_TEST1, 0),
        }},
    };
    std::shared_ptr<DataVector> pDv = nullptr;
    Errors::exitOnError (DataVector::createNew (dvConfig, pDv),
                         "Failed to init Data Vector");

    // 2) Init Network Manager. 
    NetworkManager::Config_t nmConfig = 
    {
        ClockSyncTest_Config::mNodes,
        ClockSyncTest_Config::mChannels,
        NetworkManager::Node_t::DEVICE_NODE_0,
        DV_ELEM_TEST0, // msg tx count
        DV_ELEM_TEST1, // msg rx count
    };
    std::shared_ptr<NetworkManager> pNm;
    Errors::exitOnError (NetworkManager::createNew (nmConfig, pDv, pNm),
                         "Failed to init Network Manager");

    // 3) Attempt to sync client with server.
    Errors::exitOnError(ClockSync::syncClient (
                                         pNm, 
                                         NetworkManager::Node_t::CONTROL_NODE, 
                                         CONTROL_NODE_IP),
                        "Failed to sync.");

    // 4) Verify message tx and rx counts.
    uint32_t msgTxCount = 0;
    uint32_t msgRxCount = 0;
    Errors::exitOnError (pDv->read (DV_ELEM_TEST0, msgTxCount), "DV read fail");
    Errors::exitOnError (pDv->read (DV_ELEM_TEST1, msgRxCount), "DV read fail");
    if (msgTxCount != EXPECTED_MSG_TX_COUNT || 
        msgRxCount != EXPECTED_MSG_RX_COUNT)
    {
        // Fail.
        std::cout << "TEST FAILED: Incorrect number of messages tx'd or rx'd." 
            << std::endl;
    }
    else
    {
        // Pass.
        std::cout << "TEST PASSED: Client sync successful." << std::endl;
    }
}
