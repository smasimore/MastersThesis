/**
 * See ClockSyncTest_Config.hpp for instructions on running test.
 */

#include "NetworkManager.hpp"

#include "ClockSyncTest_Config.hpp"

std::unordered_map<Node_t, 
                   NetworkManager::IP_t, 
                   EnumClassHash> ClockSyncTest_Config::mNodes = 
{
    {NODE_DEVICE0, DEVICE_NODE_IP},
    {NODE_CONTROL,  CONTROL_NODE_IP}
};


std::vector<NetworkManager::ChannelConfig_t> ClockSyncTest_Config::mChannels =
{
    {NODE_CONTROL, 
     NODE_DEVICE0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)}
};
