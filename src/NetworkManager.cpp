#include <set>
#include <cstring>
#include <cinttypes>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 

#include "NetworkManager.hpp"

const uint16_t NetworkManager::MIN_PORT = 2200;
const uint16_t NetworkManager::MAX_PORT = 2299;

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t NetworkManager::createNew (NetworkManager::NetworkManagerConfig_t& kConfig,
                                std::shared_ptr<NetworkManager>& kPNetworkManagerRet)
{
    Error_t ret = E_SUCCESS;

    // Verify config.
    ret = NetworkManager::verifyConfig (kConfig);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Create Network Manager.
    kPNetworkManagerRet.reset (new NetworkManager (kConfig, ret));

    // Check for error on construct and free memory if it failed.
    if (ret != E_SUCCESS)
    {
        kPNetworkManagerRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t NetworkManager::send (NetworkManager::Node_t kNode, 
                              std::vector<uint8_t>& kBuf)
{
    // 1) Verify buffer is not empty.
    if (kBuf.size () == 0)
    {
        return E_EMPTY_BUFFER;
    }

    // 2) Verify valid node and get channel information.
    if (mNodeToChannel.find (kNode) == mNodeToChannel.end ())
    {
        return E_INVALID_NODE;
    }
    NetworkManager::Channel_t channel = mNodeToChannel[kNode];

    // 3) Fill destination address info.
    struct sockaddr_in destAddr;
    memset ((void*) (&destAddr), 0, sizeof (destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons (channel.toPort);
    destAddr.sin_addr.s_addr = htonl (channel.toIP);
        
    // 4) Send message with no flags (0).
    int32_t numBytesSent = sendto (channel.socketFd, kBuf.data (), kBuf.size (), 
                                   0, (const struct sockaddr*) &destAddr, 
                                   sizeof (destAddr) );
    
    // 5) Verify full message sent.
    if (numBytesSent == -1)
    {
        return E_FAILED_TO_SEND_MSG;
    }
    else if (numBytesSent != (int32_t) kBuf.size ())
    {
        return E_UNEXPECTED_SEND_SIZE;
    }
    
    return E_SUCCESS;
}

Error_t NetworkManager::recv (NetworkManager::Node_t kNode, 
                             std::vector<uint8_t>& kBuf)
{
    // 1) Verify buffer is not empty.
    if (kBuf.size () == 0)
    {
        return E_EMPTY_BUFFER;
    }

    // 2) Verify valid node and get channel information.
    if (mNodeToChannel.find (kNode) == mNodeToChannel.end ())
    {
        return E_INVALID_NODE;
    }
    NetworkManager::Channel_t channel = mNodeToChannel[kNode];

    // 3) Receive message. MSG_TRUNC causes recv to return the total size of 
    // the received packet even if it is larger than the buffer supplied.
    int32_t numBytesRecvd = ::recv (channel.socketFd, kBuf.data (), 
                                    kBuf.size (), MSG_TRUNC);
    if (numBytesRecvd == -1)
    {
        return E_FAILED_TO_RECV_MSG;
    }
    else if (numBytesRecvd != (int32_t) kBuf.size ())
    {
        return E_UNEXPECTED_RECV_SIZE;
    }

    return E_SUCCESS;
}

NetworkManager::~NetworkManager ()
{
    for (std::pair<Node_t, Channel_t> element : mNodeToChannel)
    {
        close (element.second.socketFd);
    }
}

/**************************** PRIVATE FUNCTIONS *******************************/

NetworkManager::NetworkManager (NetworkManager::NetworkManagerConfig_t& kConfig, 
                                Error_t& kRet)
{
    // 1) Parse info from kConfig.
    std::vector<NetworkManager::ChannelConfig_t> channelConfigs = 
        kConfig.channels;
    NetworkManager::Node_t me = kConfig.me;

    uint32_t meIp; 
    kRet = NetworkManager::convertIPStringToUInt32 (kConfig.nodeToIp[me], meIp);
    if (kRet != E_SUCCESS)
    {
        return;
    }

    // 2) Loop through config and create socket per channel where "me" is one
    //    of the channel's nodes.
    for (NetworkManager::ChannelConfig_t channelConfig : channelConfigs)
    {
        // 2a) If "me" is not either of the channelConfig's nodes, continue.
        if (me != channelConfig.node1 && me != channelConfig.node2)
        {
            continue;
        }

        // 2b) Create socket for channelConfig.
        int32_t socketFd = -1;
        kRet = this->createSocket (meIp, channelConfig.port, socketFd);
        if (kRet != E_SUCCESS)
        {
            return;
        }

        // 2c) Get node on other side of channelConfig.
        NetworkManager::Node_t toNode = me == channelConfig.node1 
            ? channelConfig.node2 
            : channelConfig.node1;

        // 2d) Store node to channel info.
        NetworkManager::Channel_t channel;
        channel.socketFd = socketFd;
        channel.toPort = channelConfig.port; 
        
        kRet = this->convertIPStringToUInt32 (kConfig.nodeToIp[toNode], channel.toIP);
        if (kRet != E_SUCCESS)
        {
            return;
        }

        mNodeToChannel.insert ({toNode, channel});
    }
}

Error_t NetworkManager::verifyConfig (NetworkManager::NetworkManagerConfig_t& kConfig)
{
    std::unordered_map<Node_t, IP_t, EnumClassHash> nodeToIp = kConfig.nodeToIp;
    std::vector<NetworkManager::ChannelConfig_t> channelConfigs = 
        kConfig.channels;

    // 1) Verify kConfig not empty.
    if (nodeToIp.size () == 0)
    {
        return E_EMPTY_NODE_CONFIG;
    }
    else if (channelConfigs.size () == 0)
    {
        return E_EMPTY_CHANNEL_CONFIG;
    }

    // 2) Verify nodes valid & IP's are valid and unique. Can't have duplicate
    //    nodes at this point, since stored in a map.
    std::set<std::string> ipSet;
    for (std::pair<NetworkManager::Node_t, std::string> element : nodeToIp)
    {
        NetworkManager::Node_t node = element.first;
        std::string ip              = element.second;

        // 2a) Verify valid node enum.
        if (node >= NetworkManager::Node_t::LAST)
        {
            return E_INVALID_ENUM;
        }

        // 2b) Verify unique ip.
        if (ipSet.insert (ip).second == false)
        {
            return E_DUPLICATE_IP;
        }
    
        // 2c) Verify valid ip.
        uint32_t _unused;
        Error_t ret = NetworkManager::convertIPStringToUInt32 (ip, _unused);
        if (ret != E_SUCCESS)
        {
            return ret;
        }
    }

    // 3) Verify channels reference defined nodes, use valid port numbers, and
    //    only 1 channel per node pair.
    std::set<std::set<NetworkManager::Node_t>> nodePairSet;
    for (uint32_t i = 0; i < channelConfigs.size (); i++)
    {
        NetworkManager::ChannelConfig_t channelConfig = channelConfigs[i];

        // 3a) Verify unique node pair.
        std::set<NetworkManager::Node_t> nodePair = {channelConfig.node1,
                                                     channelConfig.node2};
        if (nodePairSet.insert (nodePair).second == false)
        {
            return E_DUPLICATE_CHANNEL;
        }

        // 3b) Verify nodes defined.
        if (nodeToIp.find (channelConfig.node1) == nodeToIp.end () ||
            nodeToIp.find (channelConfig.node2) == nodeToIp.end ())
        {
            return E_UNDEFINED_NODE_IN_CHANNEL;
        }

        // 3c) Verify valid port number.
        if (channelConfig.port < NetworkManager::MIN_PORT || 
            channelConfig.port > NetworkManager::MAX_PORT)
        {
            return E_INVALID_PORT;
        }
    }

    // 4) Verify "me" is a defined node.
    if (nodeToIp.find (kConfig.me) == nodeToIp.end ())
    {
        return E_UNDEFINED_ME_NODE;
    }

    return E_SUCCESS;
}

Error_t NetworkManager::convertIPStringToUInt32 (std::string kIpStr,
                                                 uint32_t& kIpUInt32Ret)
{
    const char DELIMITER             = '.';
    const uint8_t EXPECTED_NUM_BYTES = 4;

    // Break IP string into '.' separated regions.
    uint8_t ipRegionUInt8Arr[EXPECTED_NUM_BYTES];
    std::string ipRegionStr;
    uint8_t ipRegionStart = 0;
    uint8_t ipRegionEnd   = 0;
    uint8_t numIpRegions  = 0;
    while (ipRegionEnd < kIpStr.size ())
    {
        // Get next region.
        ipRegionEnd = kIpStr.find (DELIMITER, ipRegionStart);
        ipRegionStr = kIpStr.substr (ipRegionStart, ipRegionEnd - ipRegionStart);
        ipRegionStart = ipRegionEnd + 1;
        numIpRegions++;

        // Verify all characters in IP region are digits.
        if (std::all_of (ipRegionStr.begin(), ipRegionStr.end(), ::isdigit) == false)
        {
            return E_NON_NUMERIC_IP;
        }

        // Convert region to a uint32, verify no bigger than max uint8, and store
        // in byte array.
        uint32_t byteUInt32 = std::stoul (ipRegionStr);
        if (byteUInt32 > UINT8_MAX)
        {
            return E_INVALID_IP_REGION;
        }
        ipRegionUInt8Arr[numIpRegions - 1] = static_cast<uint8_t> (byteUInt32);
    }

    // Verify number of IP regions matches expected.
    if (numIpRegions != EXPECTED_NUM_BYTES)
    {
        return E_INVALID_IP_SIZE;
    }

    // Convert byte array to a uint32 and store in return parameter.
    kIpUInt32Ret = 
        static_cast<uint32_t> (ipRegionUInt8Arr[0]) << 24 |
        static_cast<uint32_t> (ipRegionUInt8Arr[1]) << 16 |
        static_cast<uint32_t> (ipRegionUInt8Arr[2]) <<  8 |
        static_cast<uint32_t> (ipRegionUInt8Arr[3]);

    return E_SUCCESS;
}

Error_t NetworkManager::createSocket (uint32_t kMeIp, uint16_t kPort, 
                                      int32_t& kSocketRet)
{
    // 1) Create socket using IPv4 protocol (AF_INET), UDP (SOCK_DGRAM), and
    //    no additionally specified protocol (0, UDP is set through SOCK_DGRAM)
    int32_t sockFd = socket (AF_INET, SOCK_DGRAM, 0);
    if (sockFd == -1)
    {
        return E_FAILED_TO_CREATE_SOCKET;
    }
    
    // 2) Bind socket to me address & port.
    struct sockaddr_in meAddr;
    memset ((void*) (&meAddr), 0, sizeof (meAddr));
    meAddr.sin_family = AF_INET;
    meAddr.sin_addr.s_addr = htonl (kMeIp);
    meAddr.sin_port = htons (kPort);
    if (bind (sockFd, (const struct sockaddr *) &meAddr, sizeof (meAddr)) != 0)
    {
        return E_FAILED_TO_BIND_TO_SOCKET;
    }

    // 3) Store socket FD in return parameter.
    kSocketRet = sockFd;

    return E_SUCCESS;
}
