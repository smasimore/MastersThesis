/**
 * See ProfileEthernetRtt_Config.hpp for instructions on running test.
 */

#include <iostream>

#include "ProfileEthernetRtt_DeviceNode.hpp"

void ProfileEthernetRtt_DeviceNode::main (int, char**)
{
    // 1) Init thread priority.
    ProfileHelpers::setThreadPriAndAffinity ();

    // 2) Init Data Vector.
    std::shared_ptr<DataVector> pDv = nullptr;
    Errors::exitOnError (DataVector::createNew (
                             ProfileEthernetRtt_Config::mDvConfig,
                             pDv), "DV init");

    // 3) Init Network Manager.
    std::shared_ptr<NetworkManager> pNm = nullptr;
    NetworkManager::Config_t nmConfig = 
    {
        ProfileEthernetRtt_Config::mNodes,
        ProfileEthernetRtt_Config::mChannels,
        DEVICE_NODE_TO_COMPILE,
        DV_ELEM_TEST0,
        DV_ELEM_TEST1,
    };
    Errors::exitOnError (NetworkManager::createNew (nmConfig, pDv, pNm), 
                         "NM init");

    // 4) Synchronize clocks to be able to compare flight computer timestamps.
    Errors::exitOnError (ClockSync::syncClient (pNm, NODE_CONTROL, 
                                                CONTROL_NODE_IP), 
                         "ClockSync");

    // 5) Initialize buffer.
    std::vector<uint8_t> buf (REGION_SIZE_BYTES);

    // 6) Debug runs.
    for (uint32_t j = 0; j < NUM_DEBUG_RUNS; j++)
    {
        static Time::TimeNs_t prevSentNs = 0;

        // Receive "Region" from DN.
        Errors::exitOnError (pNm->recvBlock (NODE_CONTROL, buf), "Recv err");

        // Get time message received and store that time and previous send
        // time in response buffer.
        Time::TimeNs_t recvdNs = ProfileHelpers::getTimeNs();
        std::memcpy (&buf[0], &prevSentNs, sizeof (prevSentNs));
        std::memcpy (&buf[sizeof (Time::TimeNs_t)], 
                     &recvdNs, sizeof (recvdNs));

        // Send "Region" response to DN.
        Errors::exitOnError (pNm->send (NODE_CONTROL, buf), "Send err");

        // Store send time.
        prevSentNs = ProfileHelpers::getTimeNs();
    }

    // 7) For the rest of the script, loop forever.
    while (1)
    {
        // Receive "Region" from DN.
        Errors::exitOnError (pNm->recvBlock (NODE_CONTROL, buf), "Recv err");

        // Send "Region" response to DN.
        Errors::exitOnError (pNm->send (NODE_CONTROL, buf), "Send err");
    }
}
