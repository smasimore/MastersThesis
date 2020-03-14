/**
 * The Network Manager is responsible for initializing sockets to enable 
 * transmitting and receiving messages across the flight network. The flight
 * network uses Ethernet and UDP (to reduce communication latency). 
 *
 * A node is defined as a computer on the network. A communication channel
 * is defined as a node:node pair that will be communicating on the network. A
 * socket is created per communication channel. A peer-to-peer architecture is 
 * used since each node will send AND receive data and the communication across
 * the network is deterministic. Unlike a typical client-server model, we know 
 * exactly which nodes will be communicating and what messages they will be 
 * sending in each communication channel. This design enables us to call recv 
 * on a socket and know exactly what message we are receiving and from which 
 * node. This means we do not need to "peek" or provide a max-size buffer to 
 * read the message and then figure out what type of message it is later.
 *
 *                         ------- CONFIG -------
 *
 * The config represents the network as a graph, where each computer is a node
 * and each edge is a communication channel (e.g. FC <--> RIO0 channel).  On 
 * initialization, a socket will be created per channel. Only 1 channel is 
 * permitted per node pair based on the current design, but if needed the 
 * Network Manager can be refactored to support multiple channels per node pair.
 * Broadcast is NOT currently supported.
 *
 * Choose ports between 2200-2299. These are unused on the sbRIO's and on Ubuntu
 * 16.4. To see what ports are in use, run "cat /etc/services".
 *
 *
 *                         ------- WARNINGS -------
 *
 * #1 Receiving data on the same channel is NOT threadsafe. For example:
 *      a) Thread 1 running on Node A calls recvMult on a channel from Node B
 *      b) Thread 1 calls select, which returns that the channel now has 
 *         data in it from Node B.
 *      c) Thread 2 takes over the CPU (blocking Thread 1) and calls recv on 
 *         the same channel from Node B and reads the new data.
 *      d) Thread 1 runs again and attempts to read the same new data it is
 *         expecting from Node B and blocks. 
 *
 *      Thread 1 blocking on the recvMult call is undesired and unexpected, as
 *      recvMult is intended to allow the Control Node to listen on multiple
 *      channels and timeout. recvMult should never block.
 *
 */

#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <stdint.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "EnumClassHash.hpp"
#include "Errors.hpp"

class NetworkManager final 
{

public:

    /**
     * Minimum port value permitted.
     */
    static const uint16_t MIN_PORT;

    /**
     * Maximum port value permitted.
     */
    static const uint16_t MAX_PORT;

    /**
     * Maximum timeout size in microseconds.
     */
    static const uint32_t MAX_TIMEOUT_US;

    /**
     * Allowed network nodes.
     */
    enum Node_t : uint8_t
    {
        CONTROL_NODE,
        DEVICE_NODE_0,
        DEVICE_NODE_1,
        DEVICE_NODE_2,
        GROUND,

        LAST
    };

    /**
     * IPv4 address type. This is expected to be in "x.x.x.x" format, which each
     * x being a uint8 represented as a string.
     */
    typedef std::string IP_t;

    /**
     * Struct to represent a communication channel config. Each channel gets 
     * converted to a socket on initialization.
     */
    typedef struct ChannelConfig
    {
        Node_t   node1;
        Node_t   node2;
        uint16_t port;
    } ChannelConfig_t;

    /**
     * Network Manager config.
     */
    typedef struct Config
    {
        std::unordered_map<Node_t, IP_t, EnumClassHash> nodeToIp;
        std::vector<ChannelConfig_t>                    channels;
        Node_t                                          me;
    } Config_t;

    /**
     * Entry point for creating a new Network Manager. Validates the passed in 
     * config. This should only be called once per compute node, although
     * this is not enforced to facilitate testing.
     *
     * @param   kConfig                     Network Manager's config data.
     * @param   kPNmRet                     Pointer to return Network Manager.
     *
     * @ret     E_SUCCESS                      Network Manager successfully 
     *                                         created.
     *          E_EMPTY_CONFIG                 Config empty.
     *          E_EMPTY_NODE_CONFIG            Empty node map.
     *          E_EMPTY_CHANNEL_CONFIG         Empty channels list.
     *          E_INVALID_ENUM                 Invalid node enum.
     *          E_DUPLICATE_IP                 Duplicate IP in node map.
     *          E_NON_NUMERIC_IP               Character in numeric region of 
     *                                         IP.
     *          E_INVALID_IP_REGION            Size of IP region greater than 1
     *                                         bytes.
     *          E_INVALID_IP_SIZE              Invalid number of IP regions.
     *          E_UNDEFINED_NODE_IN_CHANNEL    Node in channel not in nodeToIp.
     *          E_INVALID_PORT                 Port not within permitted bounds.
     *          E_UNDEFINED_ME_NODE            "Me" is not defined in nodeToIp.
     *          E_FAILED_TO_CREATE_SOCKET      Failed to create socket.
     *          E_FAILED_TO_SET_SOCKET_OPTIONS Failed to set socket options.
     *          E_FAILED_TO_BIND_TO_SOCKET     Failed to bind "me" info to 
     *                                         socket. This can happen if IP 
     *                                         assigned to "me" is not correct.
    */
    static Error_t createNew (Config_t& kConfig, 
                              std::shared_ptr<NetworkManager>& kPNmRet);

    /**
     * Send a message to a node. 
     *
     * @param   kNode                       Node to send message to.
     * @param   kBuf                        Data to send.
     *
     * @ret     E_SUCCESS                   Message successfully sent.
     *          E_EMPTY_BUFFER              kBuf empty.
     *          E_INVALID_NODE              No channel for node.
     *          E_FAILED_TO_SEND_MSG        Failed to send message.
     *          E_UNEXPECTED_SEND_SIZE      Message send length != kBuf size.
    */
    Error_t send (NetworkManager::Node_t kNode, std::vector<uint8_t>& kBuf);

    /**
     * Receive a message from a node. kBufRet must already have size equal to
     * expected message size.
     *
     * WARNING: This method will block if no message currently in receive
     *          buffer.
     *
     * @param   kNode                       Node to receive message from.
     * @param   kBufRet                     Buffer to fill with message.
     *
     * @ret     E_SUCCESS                   Message successfully received.
     *          E_EMPTY_BUFFER              kBuf empty.
     *          E_INVALID_NODE              No channel for node.
     *          E_FAILED_TO_RECV_MSG        Failed to receive message.
     *          E_UNEXPECTED_RECV_SIZE      Message recv length != kBuf size.
    */
    Error_t recv (NetworkManager::Node_t kNode, std::vector<uint8_t>& kBufRet);

    /**
     * Receive a message from each of the provided nodes. The method returns
     * after either a message is received from each of the nodes or the timeout
     * has been reached. Each buffer in kBufsRet must already have size equal to
     * expected message size. kNodes, kBufsRet, and kMsgReceived must be the
     * same size.
     *
     * Note: The select call has up to 250us of overhead.
     *
     * @param   kTimeoutUs                  Timeout in microseconds. Max timeout 
     *                                      is 999999us.
     * @param   kNodes                      Nodes to receive messages from.
     * @param   kBufsRet                    Buffers to fill with messages.
     * @param   kMsgReceivedRet             True if message received from node.
     *
     * @ret     E_SUCCESS                   Messages successfully received or
     *                                      timeout expired.
     *          E_TIMEOUT_TOO_LARGE         Timeout greater than max.
     *          E_VECTORS_DIFF_SIZES        Vector params have different sizes.
     *          E_EMPTY_BUFFER              One or more of the buffers empty.
     *          E_INVALID_NODE              One or more node has no channel.
     *          E_SELECT_FAILED             Select call failed. Returns as soon 
     *                                      as failure occurs.
     *          E_FAILED_TO_RECV_MSG        Failed to receive message from one
     *                                      or more nodes. Returns as soon as
     *                                      failure occurs.
     *          E_UNEXPECTED_RECV_SIZE      Message recv length != kBuf size for
     *                                      one or more nodes. Returns as soon 
     *                                      as failure occurs.
    */
    Error_t recvMult (uint32_t kTimeoutUs,
                      std::vector<NetworkManager::Node_t> kNodes, 
                      std::vector<std::vector<uint8_t>>& kBufsRet, 
                      std::vector<bool>& kMsgReceivedRet);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF NETWORK MANAGER
     *
     * Verify provided config.
     *
     * @param   kConfig                     Config to check.
     *
     * @ret     E_SUCCESS                   Config valid.
     *          E_EMPTY_NODE_CONFIG         Empty node map.
     *          E_EMPTY_CHANNEL_CONFIG      Empty channels list.
     *          E_INVALID_ENUM              Invalid node enum.
     *          E_DUPLICATE_IP              Duplicate IP in node map.
     *          E_NON_NUMERIC_IP            Character in numeric region of IP.
     *          E_INVALID_IP_REGION         Size of IP region greater than 1 
     *                                      bytes.
     *          E_INVALID_IP_SIZE           Invalid number of IP regions.
     *          E_UNDEFINED_NODE_IN_CHANNEL Node in channel not in nodeToIp.
     *          E_INVALID_PORT              Port not within permitted bounds.
     *          E_UNDEFINED_ME_NODE         "Me" is not defined in nodeToIp.
     *          E_DUPLICATE_CHANNEL         More than 1 channel per node pair.
     */
    static Error_t verifyConfig (Config_t& kConfig);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF NETWORK MANAGER
     *
     * Convert string IP to uint32.
     *
     * @param   kIpStr                 String version of IP.
     * @param   kIpUInt32Ret           Variable to return uint32_t version of 
     *                                 IP.
     *
     * @ret     E_SUCCESS              Successfully converted string to uint32.
     *          E_NON_NUMERIC_IP       Character in numeric region of IP.
     *          E_INVALID_IP_REGION    Size of IP region greater than 1 bytes.
     *          E_INVALID_IP_SIZE      Invalid number of IP regions.
     */
    static Error_t convertIPStringToUInt32 (std::string kIpStr, 
                                            uint32_t& kIpUInt32Ret);

    /**
     * Destructor. Clean up sockets.
     */        
    ~NetworkManager ();

private:

    /**
     * Internal struct to represent a channel.
     */
    typedef struct Channel_t
    {
        int32_t socketFd;
        uint32_t toIP;
        uint16_t toPort;
    } Channel_t;

    /**
     * Map from destination node to channel.
     */
    std::unordered_map<Node_t, Channel_t, EnumClassHash> mNodeToChannel;

    /**
     * Constructor. 
     *
     * @param    kConfig  Network Manager config.
     * @param    kRet     E_SUCCESS                      Successfully created
     *                                                   Network Manager.
     *                    E_NON_NUMERIC_IP               Character in numeric 
     *                                                   region of IP.
     *                    E_INVALID_IP_REGION            Size of IP region 
     *                                                   greater than 1 bytes.
     *                    E_INVALID_IP_SIZE              Invalid number of IP 
     *                                                   regions.
     *                    E_FAILED_TO_CREATE_SOCKET      Failed to create 
     *                                                   socket.
     *                    E_FAILED_TO_SET_SOCKET_OPTIONS Failed to set socket 
     *                                                   options.
     *                    E_FAILED_TO_BIND_TO_SOCKET     Failed to bind "me" 
     *                                                   info to socket.
     *                        
     */        
    NetworkManager (Config_t& kConfig, Error_t& kRet);

    /**
     * Create a socket to send and receive messages on. Socket is blocking.
     *
     * @param   kMeIp                          IP of current node.
     * @param   kPort                          Port to receive messages on.
     * @param   kSocketRet                     Param to return socket FD in.
     *
     * @ret     E_SUCCESS                      Successfully created socket.
     *          E_FAILED_TO_CREATE_SOCKET      Failed to create socket.
     *          E_FAILED_TO_SET_SOCKET_OPTIONS Failed to set socket options.
     *          E_FAILED_TO_BIND_TO_SOCKET     Failed to bind "me" info to 
     *                                         socket.
     */
    Error_t createSocket (uint32_t kMeIp, uint16_t kPort, 
                          int32_t& kSocketRet);

};
#endif
