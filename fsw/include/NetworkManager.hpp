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
 *                         ------- NOTES -------
 *
 * #1 Due to a known issue with the Zynq-7000 series Gigabit Ethernet 
 *    Controller, messages can get stuck in the RX FIFO queue. They are unstuck
 *    when another Ethernet frame comes into the NIC. To reduce the likelihood 
 *    of messages getting stuck, after every message send a noop message is sent 
 *    to an unused port to unstick the actual data message. While this has 
 *    greatly reduced stuck messages, there are still some conditions under
 *    which stuck messages have been seen. E.g. they are still present when
 *    running the ProfileEthernetRTT scripts. They have not, however, been seen
 *    again when using the flight software Control Node and Device Node 
 *    networking logic. 
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
 *      channels and timeout. recvMult should never block indefinitely.
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
#include "DataVector.hpp"
#include "Errors.hpp"
#include "Time.hpp"

/**
 * Allowed network nodes. Defined outside of class to enable more succinct
 * usage.
 */
enum Node_t : uint8_t
{
    NODE_CONTROL,
    NODE_DEVICE0,
    NODE_DEVICE1,
    NODE_DEVICE2,
    NODE_GROUND,

    NODE_LAST
};

class NetworkManager final 
{

public:

    /**
     * Port used to send noop message to a node after an actual message is sent.
     * Used to clear out a potentially "stuck" message from the node's RX queue.
     */
    static const uint16_t NOOP_PORT;

    /**
     * Minimum port value permitted.
     */
    static const uint16_t MIN_PORT;

    /**
     * Maximum port value permitted.
     */
    static const uint16_t MAX_PORT;

    /**
     * Maximum timeout size in nanoseconds.
     */
    static const Time::TimeNs_t MAX_TIMEOUT_NS;

    /**
     * Maximum size of a message being received.
     */
    static const uint16_t MAX_RECV_BYTES;

    /**
     * IPv4 address type. This is expected to be in "x.x.x.x" format, which each
     * x being a uint8 represented as a string.
     */
    typedef std::string IP_t;

    /**
     * Struct to represent a communication channel config. Each channel is 
     * bidirectional and gets converted to a socket on initialization.
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
        /**
         * Map from all nodes in the network to their IP.
         */
        std::unordered_map<Node_t, IP_t, EnumClassHash> nodeToIp;
        /**
         * All channels.
         */
        std::vector<ChannelConfig_t>                    channels;
        /**
         * Node config is for.
         */
        Node_t                                          me;
        /**
         * Data Vector element to log number of messages successfully sent to.
         */
        DataVectorElement_t                             dvElemMsgTxCount;
        /**
         * Data Vector element to log number of messages successfully received
         * to.
         */
        DataVectorElement_t                             dvElemMsgRxCount;
    } Config_t;

    /**
     * Entry point for creating a new Network Manager. Validates the passed in 
     * config. This should only be called once per compute node, although
     * this is not enforced to facilitate testing.
     *
     * @param   kConfig                        Network Manager's config data.
     * @param   kPDv                           Pointer to Data Vector.
     * @param   kPNmRet                        Pointer to return Network 
     *                                         Manager.
     *
     * @ret     E_SUCCESS                      Network Manager successfully 
     *                                         created.
     *          E_DATA_VECTOR_NULL             kPDv null.
     *          E_EMPTY_CONFIG                 Config empty.
     *          E_INVALID_ELEM                 Count DV elems do not exist.
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
                              std::shared_ptr<DataVector> kPDv,
                              std::shared_ptr<NetworkManager>& kPNmRet);

    /**
     * Send a message to a node. Increments message send count on successful
     * send.
     *
     * WARNING: This method will block if the OS send buffer is full.
     *
     * @param   kNode                       Node to send message to.
     * @param   kBuf                        Data to send.
     *
     * @ret     E_SUCCESS                   Message successfully sent.
     *          E_EMPTY_BUFFER              kBuf empty.
     *          E_INVALID_NODE              No channel for node.
     *          E_FAILED_TO_SEND_MSG        Failed to send message.
     *          E_UNEXPECTED_SEND_SIZE      Message send length != kBuf size.
     *          E_DATA_VECTOR_WRITE         Failed to increment msgs sent 
     *                                      counter.
     */
    Error_t send (Node_t kNode, std::vector<uint8_t>& kBuf);

    /**
     * Receive a message from a node. kBufRet must already have size equal to
     * expected message size. Blocks until a message is received.
     *
     * @param   kNode                         Node to receive message from.
     * @param   kBufRet                       Buffer to fill with message.
     *
     * @ret     E_SUCCESS                     Message successfully received.
     *          E_EMPTY_BUFFER                kBuf empty.
     *          E_GREATER_THAN_MAX_RECV_BYTES Expected message size too large.
     *          E_INVALID_NODE                No channel for node.
     *          E_FAILED_TO_GET_SOCKET_FLAGS  Failed to read socket flags.
     *          E_FAILED_TO_SET_SOCKET_FLAGS  Failed to write socket flags.
     *          E_FAILED_TO_RECV_MSG          Failed to receive message.
     *          E_UNEXPECTED_RECV_SIZE        Message recv length != kBuf size.
     *          E_DATA_VECTOR_WRITE           Failed to increment msgs rx'd
     *                                        counter.
     */
    Error_t recvBlock (Node_t kNode, std::vector<uint8_t>& kBufRet);

    /**
     * Attempt to receive a message from a node. kBufRet must already have size 
     * equal to expected message size. Returns immediately even if no message 
     * received. 
     *
     * @param   kNode                         Node to receive message from.
     * @param   kBufRet                       Buffer to fill with message.
     * @param   kMsgRecvdRet                  Set to true if a message was
     *                                        received..
     *
     * @ret     E_SUCCESS                     Successfully executed function.
     *                                        Message may or may not have been 
     *                                        received.
     *          E_EMPTY_BUFFER                kBuf empty.
     *          E_GREATER_THAN_MAX_RECV_BYTES Expected message size too large.
     *          E_INVALID_NODE                No channel for node.
     *          E_FAILED_TO_GET_SOCKET_FLAGS  Failed to read socket flags.
     *          E_FAILED_TO_SET_SOCKET_FLAGS  Failed to write socket flags.
     *          E_FAILED_TO_RECV_MSG          Failed to receive message.
     *          E_UNEXPECTED_RECV_SIZE        Message recv length != kBuf size.
     *          E_DATA_VECTOR_WRITE           Failed to increment msgs rx'd
     *                                        counter.
     */
    Error_t recvNoBlock (Node_t kNode, std::vector<uint8_t>& kBufRet, 
                         bool& kMsgReceivedRet);

    /**
     * For the given timeout, attempt to receive messages from each of the 
     * provided nodes. If multiple messages are received from a single node, the
     * last message will be what is returned in kBufsRet. Each buffer in 
     * kBufsRet must already have size equal to the expected message size. 
     * kNodes, kBufsRet, and kMsgReceived must be the same size.
     *
     * Note: The select call has up to 250us of overhead.
     *
     * @param   kTimeoutNs                  Timeout in nanoseconds. Max timeout 
     *                                      is 100 seconds. Underlying timeout 
     *                                      uses microsecond increments.
     * @param   kNodes                      Nodes to receive messages from.
     * @param   kBufsRet                    Buffers to fill with messages.
     * @param   kNumMsgsReceivedRet         Number of messages received from 
     *                                      each node.
     *
     * @ret     E_SUCCESS                   Messages successfully received and
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
     *          E_DATA_VECTOR_WRITE         Failed to increment msgs rx'd
     *                                      counter.
     */
    Error_t recvMult (Time::TimeNs_t kTimeoutNs,
                      std::vector<Node_t> kNodes, 
                      std::vector<std::vector<uint8_t>>& kBufsRet, 
                      std::vector<uint32_t>& kNumMsgsReceivedRet);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF NETWORK MANAGER
     *
     * Verify provided config.
     *
     * @param   kConfig                     Config to check.
     * @param   kPDv                        Pointer to Data Vector.
     *
     *
     * @ret     E_SUCCESS                   Config valid.
     *          E_DATA_VECTOR_NULL          kPDv null.
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
    static Error_t verifyConfig (Config_t& kConfig, 
                                 std::shared_ptr<DataVector> kPDv);

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
     * Data Vector.
     */
    std::shared_ptr<DataVector> mPDataVector;

    /**
     * Data Vector element storing messages sent counter.
     */
    DataVectorElement_t mDvElemMsgTxCount;

    /**
     * Data Vector element storing messages received counter.
     */
    DataVectorElement_t mDvElemMsgRxCount;

    /**
     * Constructor. 
     *
     * @param    kConfig  Network Manager config.
     * @param    kPDv     Pointer to Data Vector.
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
    NetworkManager (Config_t& kConfig, std::shared_ptr<DataVector> kPDv, 
                    Error_t& kRet);

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

    /**
     * Verify the params passed to recvBlock and recvNoBlock. 
     *
     * @param   kNode                         Node to receive message from.
     * @param   kBuf                          Buffer to fill with message.
     *
     * @ret     E_SUCCESS                     Params valid.
     *          E_EMPTY_BUFFER                kBuf empty.
     *          E_GREATER_THAN_MAX_RECV_BYTES Expected message size too large.
     *          E_INVALID_NODE                No channel for node.
     */
    Error_t verifyRecvParams (Node_t kNode, std::vector<uint8_t>& kBuf);

};
#endif
