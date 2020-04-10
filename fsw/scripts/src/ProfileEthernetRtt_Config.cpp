/**
 * See ProfileEthernetRtt_Config.hpp for instructions on running test.
 */

#include "ProfileEthernetRtt_Config.hpp"

DataVector::Config_t ProfileEthernetRtt_Config::mDvConfig =
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT32  ( DV_ELEM_TEST0,  0 ),
        DV_ADD_UINT32  ( DV_ELEM_TEST1,  0 ),
    }},
};

std::unordered_map<Node_t, 
                   NetworkManager::IP_t, 
                   EnumClassHash> ProfileEthernetRtt_Config::mNodes = 
{
    {NODE_DEVICE0, DEVICE_NODE0_IP},
    {NODE_DEVICE1, DEVICE_NODE1_IP},
    {NODE_DEVICE2, DEVICE_NODE2_IP},
    {NODE_CONTROL, CONTROL_NODE_IP},
    {NODE_GROUND,  GROUND_NODE_IP}
};


std::vector<NetworkManager::ChannelConfig_t> 
    ProfileEthernetRtt_Config::mChannels =
{
    {NODE_CONTROL, 
     NODE_DEVICE0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    {NODE_CONTROL,
     NODE_DEVICE1,
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
    {NODE_CONTROL,
     NODE_DEVICE2,
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 2)},
    {NODE_CONTROL,
     NODE_GROUND, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 3)},
};
