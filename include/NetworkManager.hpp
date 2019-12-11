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
 * and each edge is a communication channel (e.g. FC <--> RIO0 channel). 
 * Additionally, "broadcast" is represented as a node, and an edge should be 
 * added between each computer that will broadcast or listen to broadcasts to 
 * the broadcast node. On initialization, a socket will be created per channel.
 * Only 1 channel is permitted per node pair based on the current design, but if
 * needed the Network Manager can be refactored to support multiple channels per
 * node pair.
 *
 * Choose ports between 2200-2299. These are unused on the sbRIO's and on Ubuntu
 * 16.4. To see what ports are in use, run "cat /etc/services".
 */

#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <stdint.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "EnumClassHash.hpp"
#include "Errors.h"

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
     * Allowed network nodes.
     */
    enum Node_t : uint8_t
    {
        FLIGHT_COMPUTER,
        REMOTE_IO_0,
        REMOTE_IO_1,
        REMOTE_IO_2,
        BROADCAST,
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
    typedef struct NetworkManagerConfig
    {
        std::unordered_map<Node_t, IP_t, EnumClassHash> nodeToIp;
        std::vector<ChannelConfig_t>                          channels;
        Node_t                                          me;
    } NetworkManagerConfig_t;

    /**
     * Entry point for creating a new Network Manager. Validates the passed in 
     * config. This should only be called once per compute node, although
     * this is not enforced to facilitate testing.
     *
     * @param   kConfig                     Network Manager's config data.
     * @param   kPNetworkManagerRet         Pointer to return Network Manager.
     *
     * @ret     E_SUCCESS                   Network Manager successfully 
     *                                      created.
     *          E_EMPTY_CONFIG              Config empty.
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
    */
    static Error_t createNew (NetworkManagerConfig_t& kConfig, 
                              std::shared_ptr<NetworkManager>& kPNetworkManagerRet);

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
     * Receive a message from a node. kBuf must already have size equal to
     * expected message size.
     *
     * WARNING: This method will block if no message currently in receive
     *          buffer.
     *
     * @param   kNode                        Node to send message to.
     * @param   kBuf                        Buffer to fill with message.
     *
     * @ret     E_SUCCESS                   Message successfully sent.
     *          E_EMPTY_BUFFER              kBuf empty.
     *          E_INVALID_NODE              No channel for node.
     *          E_FAILED_TO_RECV_MSG        Failed to send message.
     *          E_UNEXPECTED_RECV_SIZE      Message recv length != kBuf size.
    */
    Error_t recv (NetworkManager::Node_t kNode, std::vector<uint8_t>& kBuf);

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
    static Error_t verifyConfig (NetworkManagerConfig_t& kConfig);

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
    NetworkManager (NetworkManagerConfig_t& kConfig, Error_t& kRet);

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
