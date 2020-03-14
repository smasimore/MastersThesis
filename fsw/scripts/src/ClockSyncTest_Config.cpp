/**
 * See ClockSyncTest_Config.hpp for instructions on running test.
 */

#include "NetworkManager.hpp"

#include "ClockSyncTest_Config.hpp"

std::unordered_map<NetworkManager::Node_t, 
                   NetworkManager::IP_t, 
                   EnumClassHash> ClockSyncTest_Config::mNodes = 
{
    {NetworkManager::Node_t::DEVICE_NODE_0, DEVICE_NODE_IP},
    {NetworkManager::Node_t::CONTROL_NODE,  CONTROL_NODE_IP}
};


std::vector<NetworkManager::ChannelConfig_t> ClockSyncTest_Config::mChannels =
{
    {NetworkManager::Node_t::CONTROL_NODE, 
     NetworkManager::Node_t::DEVICE_NODE_0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)}
};
