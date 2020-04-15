#include <set>
#include <cstring>
#include <cinttypes>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/select.h>
#include <fcntl.h>

#include "NetworkManager.hpp"

const uint16_t NetworkManager::NOOP_PORT            = 2200;
const uint16_t NetworkManager::MIN_PORT             = 2201;
const uint16_t NetworkManager::MAX_PORT             = 2299;
const Time::TimeNs_t NetworkManager::MAX_TIMEOUT_NS = 100 * Time::NS_IN_S;
const uint16_t NetworkManager::MAX_RECV_BYTES       = 1024;

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t NetworkManager::createNew (NetworkManager::Config_t& kConfig,
                                   std::shared_ptr<DataVector> kPDv,
                                   std::shared_ptr<NetworkManager>& kPNmRet)
{
    Error_t ret = E_SUCCESS;

    // Verify config.
    ret = NetworkManager::verifyConfig (kConfig, kPDv);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Create Network Manager.
    kPNmRet.reset (new NetworkManager (kConfig, kPDv, ret));

    // Check for error on construct and free memory if it failed.
    if (ret != E_SUCCESS)
    {
        kPNmRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t NetworkManager::send (Node_t kNode, std::vector<uint8_t>& kBuf)
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
   
    // 6) Send no-op msg to destination node to ensure message does not get 
    //    stuck in rx queue. This is a known issue with the Zynq-7000 series
    //    Gigabit Ethernet Controller.
    destAddr.sin_port = NOOP_PORT;
    uint8_t noopMsg = 0xff;
    numBytesSent = sendto (channel.socketFd, &noopMsg, sizeof (noopMsg), 0, 
                           (const struct sockaddr*) &destAddr, 
                           sizeof (destAddr));
    
    // 7) Verify noop message sent successfully.
    if (numBytesSent == -1)
    {
        return E_FAILED_TO_SEND_MSG;
    }
    else if (numBytesSent != (int32_t) sizeof (noopMsg))
    {
        return E_UNEXPECTED_SEND_SIZE;
    }

    // 8) Increment message sent counter.
    if (mPDataVector->increment (mDvElemMsgTxCount) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

Error_t NetworkManager::recvBlock (Node_t kNode, std::vector<uint8_t>& kBufRet)
{
    // 1) Verify params.
    Error_t ret = verifyRecvParams (kNode, kBufRet);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 2) Get node's channel information.
    NetworkManager::Channel_t channel = mNodeToChannel[kNode];

    // 3) Set socket to be blocking.
    int32_t flags = fcntl (channel.socketFd, F_GETFL);
    if (flags == -1)
    {
        return E_FAILED_TO_GET_SOCKET_FLAGS;
    }
    // Check if socket is currently non-blocking. If yes, set as blocking.
    if ((flags & O_NONBLOCK) != 0)
    {
        if (fcntl (channel.socketFd, F_SETFL, flags & ~O_NONBLOCK) == -1)
        {
            return E_FAILED_TO_SET_SOCKET_FLAGS;
        }
    }

    // 4) Receive message. MSG_TRUNC causes recv to return the total size of 
    //    the received packet even if it is larger than the buffer supplied.
    int32_t numBytesRecvd = ::recv (channel.socketFd, kBufRet.data (), 
                                    kBufRet.size (), MSG_TRUNC);
    if (numBytesRecvd == -1)
    {
        return E_FAILED_TO_RECV_MSG;
    }
    else if (numBytesRecvd != (int32_t) kBufRet.size ())
    {
        return E_UNEXPECTED_RECV_SIZE;
    }

    // 5) Increment message received counter.
    if (mPDataVector->increment (mDvElemMsgRxCount) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

Error_t NetworkManager::recvNoBlock (Node_t kNode, 
                                     std::vector<uint8_t>& kBufRet,
                                     bool& kMsgReceivedRet)
{
    // 1) Initialize kMsgReceivedRet to false.
    kMsgReceivedRet = false;

    // 2) Verify params.
    Error_t ret = verifyRecvParams (kNode, kBufRet);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 3) Get node's channel information.
    NetworkManager::Channel_t channel = mNodeToChannel[kNode];

    // 4) Set socket to be non-blocking.
    int32_t flags = fcntl (channel.socketFd, F_GETFL);
    if (flags == -1)
    {
        return E_FAILED_TO_GET_SOCKET_FLAGS;
    }
    // Check if socket is currently blocking. If yes, set as non-blocking.
    if ((flags & O_NONBLOCK) == 0)
    {
        if (fcntl (channel.socketFd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            return E_FAILED_TO_SET_SOCKET_FLAGS;
        }
    }

    // 5) Attempt to receive a message. MSG_TRUNC causes recv to return the 
    //    total size of the received packet even if it is larger than the buffer 
    //    supplied.
    int32_t numBytesRecvd = ::recv (channel.socketFd, kBufRet.data (), 
                                    kBufRet.size (), MSG_TRUNC);
    if (numBytesRecvd == -1)
    {
        // Recv failed due to no message rather than an error.
        if (errno == EAGAIN)
        {
            return E_SUCCESS;
        }
        return E_FAILED_TO_RECV_MSG;
    }
    else if (numBytesRecvd != (int32_t) kBufRet.size ())
    {
        return E_UNEXPECTED_RECV_SIZE;
    }

    // 6) Increment message received counter.
    if (mPDataVector->increment (mDvElemMsgRxCount) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    kMsgReceivedRet = true;
    return E_SUCCESS;
}

Error_t NetworkManager::recvMult (Time::TimeNs_t kTimeoutNs,
                                  std::vector<Node_t> kNodes,
                                  std::vector<std::vector<uint8_t>>& kBufsRet, 
                                  std::vector<uint32_t>& kNumMsgsReceivedRet)
{
    // 1) Verify vector inputs are the same size.
    uint8_t numNodes = kNodes.size ();
    if (numNodes != kBufsRet.size () ||
        numNodes != kNumMsgsReceivedRet.size ())
    {
        return E_VECTORS_DIFF_SIZES;
    }

    // 2) Verify timeout less than max.
    if (kTimeoutNs > NetworkManager::MAX_TIMEOUT_NS)
    {
        return E_TIMEOUT_TOO_LARGE;
    }

    // 3) Loop through nodes to receive from to verify buffers, initialize
    //    kNumMsgsReceivedRet, and get relevant channels.
    std::vector<NetworkManager::Channel_t> channels (numNodes);
    for (uint8_t i = 0; i < numNodes; i++)
    {
        // 3a) Verify buffer is not empty.
        if (kBufsRet[i].size () == 0)
        {
            return E_EMPTY_BUFFER;
        }

        // 3b) Verify node is valid.
        if (mNodeToChannel.find (kNodes[i]) == mNodeToChannel.end ())
        {
            return E_INVALID_NODE;
        }

        // 3c) Initialize kNumMsgsReceivedRet.
        kNumMsgsReceivedRet[i] = 0;

        // 3d) Build vector of channels. Each channel contains socket fd used 
        //     that will be used in the select call.
        channels[i] = mNodeToChannel[kNodes[i]];
    }

    // 4) Build file descriptor set for select call.
    fd_set readFds;
    FD_ZERO (&readFds);
    for (NetworkManager::Channel_t channel : channels)
    {
        FD_SET (channel.socketFd, &readFds); 
    }

    // 5) Create struct timeval for select call.
    struct timeval timeout;
    timeout.tv_sec = kTimeoutNs / Time::NS_IN_S;
    timeout.tv_usec = (kTimeoutNs % Time::NS_IN_S) / Time::NS_IN_US;

    // 6) Attempt to receive message from nodes until timeout expires.
    while (timeout.tv_sec > 0 || timeout.tv_usec > 0)
    {
        // 6a) Call select on fd set. Select returns 0 if timeout expired, -1 if 
        //     there was an error, and > 0 to specify how many fd's have content 
        //     to read. Select modifies timeout to store time remaining. fd_set 
        //     is modified to contain fd's that are ready to read.
        //
        //     NOTE: Calling select has up to 250us of overhead.
        fd_set readFdsResult = readFds;
        int32_t selectRet = select (FD_SETSIZE, &readFdsResult, nullptr, 
                                    nullptr, &timeout);
        if (selectRet < 0)
        {
            return E_SELECT_FAILED;
        }
        else if (selectRet == 0)
        {
            break;
        }
       
        // 6b) Loop through channels to determine which sockets can be read and
        //     read messages.
        for (uint8_t i = 0; i < numNodes; i++)
        {
            NetworkManager::Channel_t channel = channels[i];
            if (FD_ISSET (channel.socketFd, &readFdsResult) != 0)
            {
                // 6b i) Read socket. MSG_TRUNC causes recv to return the total
                //      size of the received packet even if it is larger than 
                //      the buffer supplied.
                int32_t numBytesRecvd = ::recv (channel.socketFd, 
                                                kBufsRet[i].data (), 
                                                kBufsRet[i].size (), 
                                                MSG_TRUNC);

                // 6b ii) Handle failed recv.
                if (numBytesRecvd == -1)
                {
                    return E_FAILED_TO_RECV_MSG;
                }
                else if (numBytesRecvd != (int32_t) kBufsRet[i].size ())
                {
                    return E_UNEXPECTED_RECV_SIZE;
                }

                // 6b iii) Increment message received counter.
                if (mPDataVector->increment (mDvElemMsgRxCount) != E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }

                // 6b iv) Increment msgs received count.
                kNumMsgsReceivedRet[i]++;
            }
        }
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

NetworkManager::NetworkManager (NetworkManager::Config_t& kConfig, 
                                std::shared_ptr<DataVector> kPDv,
                                Error_t& kRet) :
    mPDataVector (kPDv),
    mDvElemMsgTxCount (kConfig.dvElemMsgTxCount),
    mDvElemMsgRxCount (kConfig.dvElemMsgRxCount)
{
    // 1) Parse info from kConfig.
    std::vector<NetworkManager::ChannelConfig_t> channelConfigs = 
        kConfig.channels;
    Node_t me = kConfig.me;

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
        Node_t toNode = me == channelConfig.node1 
            ? channelConfig.node2 
            : channelConfig.node1;

        // 2d) Store node to channel info.
        NetworkManager::Channel_t channel;
        channel.socketFd = socketFd;
        channel.toPort = channelConfig.port; 
        
        kRet = this->convertIPStringToUInt32 (kConfig.nodeToIp[toNode], 
                                              channel.toIP);
        if (kRet != E_SUCCESS)
        {
            return;
        }

        mNodeToChannel.insert ({toNode, channel});
    }
}

Error_t NetworkManager::verifyConfig (NetworkManager::Config_t& kConfig,
                                      std::shared_ptr<DataVector> kPDv)
{
    std::unordered_map<Node_t, IP_t, EnumClassHash> nodeToIp = kConfig.nodeToIp;
    std::vector<NetworkManager::ChannelConfig_t> channelConfigs = 
        kConfig.channels;

    // 1) Verify kPDv not null.
    if (kPDv == nullptr)
    {
        return E_DATA_VECTOR_NULL;
    }

    // 2) Verify kConfig not empty.
    if (nodeToIp.size () == 0)
    {
        return E_EMPTY_NODE_CONFIG;
    }
    else if (channelConfigs.size () == 0)
    {
        return E_EMPTY_CHANNEL_CONFIG;
    }

    // 3) Verify msg rx/tx counter elems exist in Data Vector.
    if (kPDv->elementExists (kConfig.dvElemMsgTxCount) != E_SUCCESS ||
        kPDv->elementExists (kConfig.dvElemMsgRxCount) != E_SUCCESS)
    {
        return E_INVALID_ELEM;
    }

    // 4) Verify nodes valid & IP's are valid and unique. Can't have duplicate
    //    nodes at this point, since stored in a map.
    std::set<std::string> ipSet;
    for (std::pair<Node_t, std::string> element : nodeToIp)
    {
        Node_t node    = element.first;
        std::string ip = element.second;

        // 4a) Verify valid node enum.
        if (node >= NODE_LAST)
        {
            return E_INVALID_ENUM;
        }

        // 4b) Verify unique ip.
        if (ipSet.insert (ip).second == false)
        {
            return E_DUPLICATE_IP;
        }
    
        // 4c) Verify valid ip.
        uint32_t _unused;
        Error_t ret = NetworkManager::convertIPStringToUInt32 (ip, _unused);
        if (ret != E_SUCCESS)
        {
            return ret;
        }
    }

    // 5) Verify channels reference defined nodes, use valid port numbers, and
    //    only 1 channel per node pair.
    std::set<std::set<Node_t>> nodePairSet;
    for (uint32_t i = 0; i < channelConfigs.size (); i++)
    {
        NetworkManager::ChannelConfig_t channelConfig = channelConfigs[i];

        // 5a) Verify unique node pair.
        std::set<Node_t> nodePair = {channelConfig.node1, channelConfig.node2};
        if (nodePairSet.insert (nodePair).second == false)
        {
            return E_DUPLICATE_CHANNEL;
        }

        // 5b) Verify nodes defined.
        if (nodeToIp.find (channelConfig.node1) == nodeToIp.end () ||
            nodeToIp.find (channelConfig.node2) == nodeToIp.end ())
        {
            return E_UNDEFINED_NODE_IN_CHANNEL;
        }

        // 5c) Verify valid port number.
        if (channelConfig.port < NetworkManager::MIN_PORT || 
            channelConfig.port > NetworkManager::MAX_PORT)
        {
            return E_INVALID_PORT;
        }
    }

    // 6) Verify "me" is a defined node.
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
        ipRegionStr = kIpStr.substr (ipRegionStart, 
                                     ipRegionEnd - ipRegionStart);
        ipRegionStart = ipRegionEnd + 1;
        numIpRegions++;

        // Verify all characters in IP region are digits.
        if (std::all_of (ipRegionStr.begin(), ipRegionStr.end(), ::isdigit) 
                == false)
        {
            return E_NON_NUMERIC_IP;
        }

        // Convert region to a uint32, verify no bigger than max uint8, and 
        // store in byte array.
        uint32_t byteUInt32 = std::stoul (ipRegionStr);
        if (byteUInt32 > UINT8_MAX)
        {
            return E_INVALID_IP_REGION;
        }

        // Verify number of IP regions does not exceed expected
        if (numIpRegions > EXPECTED_NUM_BYTES)
        {
            return E_INVALID_IP_SIZE;
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
    int32_t sockFd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

Error_t NetworkManager::verifyRecvParams (Node_t kNode, 
                                          std::vector<uint8_t>& kBuf)
{
    // Verify node is valid.
    if (mNodeToChannel.find (kNode) == mNodeToChannel.end ())
    {
        return E_INVALID_NODE;
    }

    // Verify buffer is not empty or greater than max allowed recv size. 
    if (kBuf.size () == 0)
    {
        return E_EMPTY_BUFFER;
    }
    else if (kBuf.size () > MAX_RECV_BYTES)
    {
        return E_GREATER_THAN_MAX_RECV_BYTES;
    }

    return E_SUCCESS;
}
