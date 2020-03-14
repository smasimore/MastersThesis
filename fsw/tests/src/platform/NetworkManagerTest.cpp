/* All #include statements should come before the CppUTest include */
#include <cstring>
#include <sstream>

#include "Errors.hpp"
#include "NetworkManager.hpp"
#include "TimeNs.hpp"
#include "EnumClassHash.hpp"

#include "TestHelpers.hpp"

/********************************* MACROS *************************************/
#define INIT_NETWORK_MANAGERS                                                  \
    std::shared_ptr<NetworkManager> pNmCtrl;                                   \
    std::shared_ptr<NetworkManager> pNmDev0;                                   \
    std::shared_ptr<NetworkManager> pNmDev1;                                   \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigCtrl, pNmCtrl));  \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev0, pNmDev0));  \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev1, pNmDev1));


/*************************** VERIFY CONFIG TESTS ******************************/

/* Valid config to use for verify config tests. */
NetworkManager::Config_t gValidConfig =
{
    // Nodes
    {
        {NetworkManager::Node_t::CONTROL_NODE,  "10.0.0.1"},
        {NetworkManager::Node_t::DEVICE_NODE_0, "10.0.0.2"},
    },

    // Channels
    {
        {NetworkManager::Node_t::CONTROL_NODE, 
         NetworkManager::Node_t::DEVICE_NODE_0, 
         NetworkManager::MIN_PORT},
    },

    // Me
    NetworkManager::Node_t::CONTROL_NODE,
};

/* Group of tests verifying verifyConfig method. */
TEST_GROUP (NetworkManager_verifyConfig)
{

};

/* Test initializing with empty node map. */
TEST (NetworkManager_verifyConfig, NoNodes)
{
    // Explicitly initialize empty node mpa.
    std::unordered_map<
        NetworkManager::Node_t, 
        NetworkManager::IP_t, 
        EnumClassHash> emptyNodeToIP;

    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp = emptyNodeToIP;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_EMPTY_NODE_CONFIG);
}

/* Test initializing with empty channels list. */
TEST (NetworkManager_verifyConfig, NoChannels)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels = {};

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_EMPTY_CHANNEL_CONFIG);
}

/* Test initializing with invalid node enum. */
TEST (NetworkManager_verifyConfig, InvalidNode)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::LAST] = "10.0.0.3";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_ENUM);
}

/* Test initializing with duplicate IP's. */
TEST (NetworkManager_verifyConfig, DupeIP)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::DEVICE_NODE_0] = "10.0.0.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_DUPLICATE_IP);
}

/* Test initializing with non-numeric IP. */
TEST (NetworkManager_verifyConfig, NonNumericIP)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::DEVICE_NODE_0] = "10.a.0.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_NON_NUMERIC_IP);
}

/* Test initializing with IP region value too high. */
TEST (NetworkManager_verifyConfig, InvalidIPRegion)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::DEVICE_NODE_0] = "10.0.0.256";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_REGION);
}

/* Test initializing with empty IP. */
TEST (NetworkManager_verifyConfig, EmptyIP)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::DEVICE_NODE_0] = "";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_SIZE);
}

/* Test initializing with too few IP regions. */
TEST (NetworkManager_verifyConfig, TooFewIPRegions)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::DEVICE_NODE_0] = "10.0.0";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_SIZE);
}

/* Test initializing with too many IP regions. */
TEST (NetworkManager_verifyConfig, TooManyIPRegions)
{
    NetworkManager::Config_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::DEVICE_NODE_0] = "10.0.0.1.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_SIZE);
}

/* Test channel with undefined node1. */
TEST (NetworkManager_verifyConfig, UndefinedNode1)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels[0].node1 = NetworkManager::Node_t::DEVICE_NODE_1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), 
                 E_UNDEFINED_NODE_IN_CHANNEL);
}

/* Test channel with undefined node2. */
TEST (NetworkManager_verifyConfig, UndefinedNode2)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels[0].node2 = NetworkManager::Node_t::DEVICE_NODE_1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), 
                 E_UNDEFINED_NODE_IN_CHANNEL);
}

/* Test initializing with port below min. */
TEST (NetworkManager_verifyConfig, InvalidPortMin)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels[0].port = NetworkManager::MIN_PORT - 1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_PORT);
}

/* Test initializing with port above max. */
TEST (NetworkManager_verifyConfig, InvalidPortMax)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels[0].port = NetworkManager::MAX_PORT + 1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_PORT);
}

/* Test initializing with undefined "me" node. */
TEST (NetworkManager_verifyConfig, UndefinedMeNode)
{
    NetworkManager::Config_t config = gValidConfig;
    config.me = NetworkManager::Node_t::DEVICE_NODE_1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_UNDEFINED_ME_NODE);
}

/* Test multiple channels per node pair, same order. */
TEST (NetworkManager_verifyConfig, DuplicateChannelSameOrder)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels.push_back ({NetworkManager::Node_t::CONTROL_NODE, 
                               NetworkManager::Node_t::DEVICE_NODE_0, 
                               NetworkManager::MIN_PORT});

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_DUPLICATE_CHANNEL);
}

/* Test multiple channels per node pair, different order. */
TEST (NetworkManager_verifyConfig, DuplicateChannelDifferentOrder)
{
    NetworkManager::Config_t config = gValidConfig;
    config.channels.push_back ({NetworkManager::Node_t::DEVICE_NODE_0, 
                                NetworkManager::Node_t::CONTROL_NODE, 
                                NetworkManager::MIN_PORT});

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_DUPLICATE_CHANNEL);
}

/* Test initializing with valid config. */
TEST (NetworkManager_verifyConfig, Success)
{
    CHECK_SUCCESS (NetworkManager::verifyConfig (gValidConfig));
}

/*********************** IP STRING TO UINT32 TESTS ***************************/

/* Group of tests verifying ipConvert method in successfuly cases. Invalid
   cases tested in verifyConfig test group. */
TEST_GROUP (NetworkManager_ipConvert)
{

};

/* Test converting valid IP addresses. */
TEST (NetworkManager_ipConvert, Success)
{
    std::unordered_map<std::string, uint32_t> testCases =
    {
        {"0.0.0.0",         0x0       },
        {"0.0.0.1",         0x1       },
        {"0.0.1.0",         0x100     },
        {"0.1.0.0",         0x10000   },
        {"1.0.0.0",         0x1000000 },
        {"255.255.255.255", 0xffffffff},
        {"10.0.0.1",        0xa000001 },
        {"10.0.0.10",       0xa00000a },
        {"10.0.0.255",      0xa0000ff },
        {"127.0.0.1",       0x7f000001},
    };

    for (std::pair<std::string, uint32_t> testCase : testCases)
    {
        std::string ipStr = testCase.first;
        uint32_t expected = testCase.second;
        uint32_t actual   = 0;
        CHECK_SUCCESS (NetworkManager::convertIPStringToUInt32 (ipStr, 
                                                                actual));
        CHECK_EQUAL (expected, actual);
    }
}

/************************* SEND/RECV/RECVMULT TESTS ***************************/

/**
 * Measured select call overhead to use for defined timeout vs. actual time
 * taken assertions.
 */
static const TimeNs::TimeNs_t SELECT_OVERHEAD_NS = 250000;

/* Loopback nodes to use for send/recv tests. */
std::unordered_map<NetworkManager::Node_t, 
                   NetworkManager::IP_t, 
                   EnumClassHash> gLoopbackNodes =
{
    {NetworkManager::Node_t::CONTROL_NODE,  "127.0.0.1"},
    {NetworkManager::Node_t::DEVICE_NODE_0, "127.0.0.2"},
    {NetworkManager::Node_t::DEVICE_NODE_1, "127.0.0.3"},
};


/* Loopback channels to use for send/recv tests. */
std::vector<NetworkManager::ChannelConfig_t> gLoopbackChannels =
{
    {NetworkManager::Node_t::CONTROL_NODE, 
     NetworkManager::Node_t::DEVICE_NODE_0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    {NetworkManager::Node_t::CONTROL_NODE, 
     NetworkManager::Node_t::DEVICE_NODE_1, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
};

/* Loopback config for Control Node to use for send/recv tests. */
NetworkManager::Config_t gLoopbackConfigCtrl =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::CONTROL_NODE,
};

/* Loopback config for Device Node 0 to use for send/recv tests. */
NetworkManager::Config_t gLoopbackConfigDev0 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_0,
};

/* Loopback config for Device Node 1 to use for send/recv tests. */
NetworkManager::Config_t gLoopbackConfigDev1 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_1,
};

/* Group of tests to verify send and recv functions. */
TEST_GROUP (NetworkManager_SendRecv)
{

};

/* Test sending with empty buffer. */
TEST (NetworkManager_SendRecv, SendEmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> sendBuf;
    CHECK_ERROR (pNmCtrl->send (NetworkManager::Node_t::CONTROL_NODE, 
                                sendBuf), E_EMPTY_BUFFER);
}

/* Test sending with invalid node. */
TEST (NetworkManager_SendRecv, SendInvalidNode)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> sendBuf = {0xff};
    CHECK_ERROR (pNmCtrl->send (NetworkManager::Node_t::DEVICE_NODE_2, 
                            sendBuf), E_INVALID_NODE);
}

/* Test recv'ing with empty buffer. */
TEST (NetworkManager_SendRecv, RecvEmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf;
    CHECK_ERROR (pNmCtrl->recv (NetworkManager::Node_t::CONTROL_NODE, 
                            recvBuf), E_EMPTY_BUFFER);
}

/* Test recv'ing with invalid node. */
TEST (NetworkManager_SendRecv, RecvInvalidNode)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf (1);
    CHECK_ERROR (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_2, 
                            recvBuf), E_INVALID_NODE);
}

/* Test receiving a message bigger than expected. */
TEST (NetworkManager_SendRecv, RecvBufferTooSmall)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf));
    CHECK_ERROR (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_0, recvBuf), 
                            E_UNEXPECTED_RECV_SIZE);
}

/* Test receiving a message smaller than expected. */
TEST (NetworkManager_SendRecv, RecvBufferTooBig)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (3, 0);
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf));
    CHECK_ERROR (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_0, recvBuf), 
                            E_UNEXPECTED_RECV_SIZE);
}

/* Send and receive a message successfully. */
TEST (NetworkManager_SendRecv, SuccessOneMessagePerChannel)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf0 = {0xff};
    std::vector<uint8_t> sendBuf1 = {0x01};
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf1));

    // Receive and verify buffers.
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_0, 
                   recvBuf));
    CHECK (sendBuf0 == recvBuf);
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_1, 
                   recvBuf));
    CHECK (sendBuf1 == recvBuf);
}

/* Send and receive two messages successfully. */
TEST (NetworkManager_SendRecv, SuccessTwoMessagesPerChannel)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf0Msg1 = {0xff};
    std::vector<uint8_t> sendBuf0Msg2 = {0xff, 0x10};
    std::vector<uint8_t> recvBuf0Msg1 (1, 0);
    std::vector<uint8_t> recvBuf0Msg2 (2, 0);
    std::vector<uint8_t> sendBuf1Msg1 = {0x11};
    std::vector<uint8_t> sendBuf1Msg2 = {0x11, 0x01};
    std::vector<uint8_t> recvBuf1Msg1 (1, 0);
    std::vector<uint8_t> recvBuf1Msg2 (2, 0);
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf0Msg1));
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf0Msg2));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf1Msg1));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::Node_t::CONTROL_NODE, 
                   sendBuf1Msg2));
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_0, 
                   recvBuf0Msg1));
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_0, 
                   recvBuf0Msg2));
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_1, 
                   recvBuf1Msg1));
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_1, 
                   recvBuf1Msg2));

    // Receive and verify buffers.
    CHECK (sendBuf0Msg1 == recvBuf0Msg1);
    CHECK (sendBuf0Msg2 == recvBuf0Msg2);
    CHECK (sendBuf1Msg1 == recvBuf1Msg1);
    CHECK (sendBuf1Msg2 == recvBuf1Msg2);
}

/**
 * Params to pass Network Manager to thread functions.
 *
 * NOTE: NetworkManager must be passed as a pointer and not a shared pointer.
 *       Otherwise, on re-casting to a shared_ptr, a new shared_ptr with its
 *       own reference count is created. On exiting the thread, it will free
 *       the memory and then on exiting the main thread it will again attempt
 *       to free the memory.
 */
struct ThreadFuncArgs 
{
    Log* log;
    NetworkManager* pNm;
};

/**
 * Thread that sends a message.
 */
static void* threadFuncSend (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs = (struct ThreadFuncArgs *) rawArgs;
    Log* log = pArgs->log;
    NetworkManager* pNmDev0 = pArgs->pNm;

    log->logEvent (Log::LogEvent_t::CALLED_SEND, 0);    
    std::vector<uint8_t> sendBuf = {0xff};
    ret = pNmDev0->send (NetworkManager::Node_t::CONTROL_NODE, sendBuf);
    return (void *) ret;
}

/* Verify thread blocks when no data in recv buffer. */
TEST (NetworkManager_SendRecv, BlockOnRecv)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_NETWORK_MANAGERS

    // Create send thread. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread;
    struct ThreadFuncArgs argsThread = {&testLog, pNmDev0.get ()}; 
    ThreadManager::ThreadFunc_t *pThreadFuncSend = 
        (ThreadManager::ThreadFunc_t *) &threadFuncSend;
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread, pThreadFuncSend,
                                    &argsThread, sizeof (argsThread),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));

    // Block on recv call.
    testLog.logEvent (Log::LogEvent_t::CALLED_RECV, 0);    
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmCtrl->recv (NetworkManager::Node_t::DEVICE_NODE_0, 
                   recvBuf));
    testLog.logEvent (Log::LogEvent_t::RECEIVED, 0);    

    // Verify received expected buffer.
    std::vector<uint8_t> expectedBuf = {0xff};
    CHECK (expectedBuf == recvBuf);

    // Verify testLog matches expected.
    expectedLog.logEvent (Log::LogEvent_t::CALLED_RECV, 0);
    expectedLog.logEvent (Log::LogEvent_t::CALLED_SEND, 0);
    expectedLog.logEvent (Log::LogEvent_t::RECEIVED, 0);
    VERIFY_LOGS;

    // Clean up thread.
    WAIT_FOR_THREAD (thread, pThreadManager);
}

/* Group of tests to verify recvMult. */
TEST_GROUP (NetworkManager_RecvMult)
{

};

/* Test with different param vector sizes. */
TEST (NetworkManager_RecvMult, DiffVectorSizes)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes;
    std::vector<std::vector<uint8_t>> bufs;
    std::vector<bool> msgsReceived;

    // Num nodes different.
    nodes.resize (1);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, bufs, 
                                    msgsReceived),
                 E_VECTORS_DIFF_SIZES);

    // Num buffs different.
    msgsReceived.resize (1);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, bufs, 
                                    msgsReceived),
                 E_VECTORS_DIFF_SIZES);

    // Num msgsReceived different.
    msgsReceived.resize (2);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, bufs, 
                                    msgsReceived),
                 E_VECTORS_DIFF_SIZES);
}

/* Test timeout too large. */
TEST (NetworkManager_RecvMult, LargeTimeout)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes;
    std::vector<std::vector<uint8_t>> bufs;
    std::vector<bool> msgsReceived;

    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US + 1, nodes, 
                                    bufs, msgsReceived),
                 E_TIMEOUT_TOO_LARGE);
}

/* Test empty buffers. */
TEST (NetworkManager_RecvMult, EmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<std::vector<uint8_t>> bufsFirstEmpty (2);
    bufsFirstEmpty[1].resize (1);
    std::vector<std::vector<uint8_t>> bufsSecondEmpty (2); 
    bufsSecondEmpty[0].resize (1);
    std::vector<bool> msgsReceived (2);

    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, 
                                    bufsFirstEmpty, msgsReceived),
                 E_EMPTY_BUFFER);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, 
                                    bufsSecondEmpty, msgsReceived),
                 E_EMPTY_BUFFER);
}

/* Test invalid nodes. */
TEST (NetworkManager_RecvMult, InvalidNode)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<NetworkManager::Node_t> nodesFirstInvalid =
    {
        NetworkManager::CONTROL_NODE,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<NetworkManager::Node_t> nodesSecondInvalid =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_2,
    };
    std::vector<std::vector<uint8_t>> bufs = {{0xff}, {0xff}};
    std::vector<bool> msgsReceived (2);

    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, 
                                    nodesFirstInvalid, bufs, msgsReceived),
                 E_INVALID_NODE);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, 
                                    nodesSecondInvalid, bufs, msgsReceived),
                 E_INVALID_NODE);
}

/* Test buffer size smaller than received message. */
TEST (NetworkManager_RecvMult, BufferSizeTooSmall)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<uint8_t> buf = {0xff, 0xff};
    std::vector<std::vector<uint8_t>> bufsFirstTooSmall (2);
    bufsFirstTooSmall[0].resize (1);
    bufsFirstTooSmall[1].resize (2);
    std::vector<std::vector<uint8_t>> bufsSecondTooSmall (2);
    bufsSecondTooSmall[0].resize (2);
    bufsSecondTooSmall[1].resize (1);
    std::vector<bool> msgsReceived (2);

    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, buf));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, buf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, 
                                    nodes, bufsFirstTooSmall, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);

    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, buf));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, buf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, 
                                    nodes, bufsSecondTooSmall, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);
}

/* Test buffer size larger than received message. */
TEST (NetworkManager_RecvMult, BufferSizeTooLarge)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };

    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<std::vector<uint8_t>> bufsFirstTooLarge (2);
    bufsFirstTooLarge[0].resize (3);
    bufsFirstTooLarge[1].resize (2);
    std::vector<std::vector<uint8_t>> bufsSecondTooLarge (2);
    bufsSecondTooLarge[0].resize (2);
    bufsSecondTooLarge[1].resize (3);
    std::vector<bool> msgsReceived (2);

    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, sendBuf));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, sendBuf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, 
                                    nodes, bufsFirstTooLarge, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);

    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, sendBuf));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, sendBuf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, 
                                    nodes, bufsSecondTooLarge, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);
}

/* Test receiving two messages sent before recvMult called. */
TEST (NetworkManager_RecvMult, MsgsRxdBeforeRecvMult)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    TimeNs* pTime;
    TimeNs::getInstance (pTime);

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<uint8_t> sendBuf0 = {0x10, 0x01};
    std::vector<uint8_t> sendBuf1 = {0x01, 0x10};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<bool> msgsReceived (2);

    // Send messages to Control Node.
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, sendBuf1));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    TimeNs::TimeNs_t startNs;
    TimeNs::TimeNs_t endNs;
    pTime->getTimeSinceInit (startNs);
    CHECK_SUCCESS (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, 
                                      bufs, msgsReceived));
    pTime->getTimeSinceInit (endNs);

    // Verify buffers match.
    CHECK (bufs[0] == sendBuf0);
    CHECK (bufs[1] == sendBuf1);

    // Verify time taken is less than timeout.
    CHECK (endNs - startNs < NetworkManager::MAX_TIMEOUT_US);

    // Verify msgsReceived all set to true.
    for (uint8_t i = 0; i < msgsReceived.size (); i++)
    {
        CHECK (msgsReceived[i]);
    }
}

/* Test receiving no messages. */
TEST (NetworkManager_RecvMult, NoMsgs)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    TimeNs* pTime;
    TimeNs::getInstance (pTime);

    // Set up params.
    const uint32_t TIMEOUT_US = 1000;
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<bool> msgsReceived (2);

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    TimeNs::TimeNs_t startNs;
    TimeNs::TimeNs_t endNs;
    pTime->getTimeSinceInit (startNs);
    pNmCtrl->recvMult (TIMEOUT_US, nodes, bufs, msgsReceived);
    pTime->getTimeSinceInit (endNs);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    TimeNs::TimeNs_t elapsedNs = endNs - startNs;
    TimeNs::TimeNs_t timeoutNs = TIMEOUT_US * TimeNs::NS_IN_US;
    CHECK (elapsedNs > timeoutNs);
    CHECK_IN_BOUND (timeoutNs, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msgsReceived all set to false.
    for (uint8_t i = 0; i < msgsReceived.size (); i++)
    {
        CHECK_FALSE (msgsReceived[i]);
    }
}

/* Test sending multiple messages on one channel. Expect to only recv first. */
TEST (NetworkManager_RecvMult, MultMsgsOneChannel)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    TimeNs* pTime;
    TimeNs::getInstance (pTime);

    // Set up params.
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<uint8_t> sendBuf0 = {0x10, 0x01};
    std::vector<uint8_t> sendBuf0SecondMsg = {0x11, 0x11};
    std::vector<uint8_t> sendBuf1 = {0x01, 0x10};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<bool> msgsReceived (2);

    // Send messages to Control Node.
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, sendBuf1));
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, 
                                  sendBuf0SecondMsg));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    TimeNs::TimeNs_t startNs;
    TimeNs::TimeNs_t endNs;
    pTime->getTimeSinceInit (startNs);
    CHECK_SUCCESS (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_US, nodes, 
                                      bufs, msgsReceived));
    pTime->getTimeSinceInit (endNs);

    // Verify buffers match.
    CHECK (bufs[0] == sendBuf0);
    CHECK (bufs[1] == sendBuf1);

    // Verify time taken is less than timeout.
    CHECK (endNs - startNs < NetworkManager::MAX_TIMEOUT_US);

    // Verify msgsReceived all set to true.
    for (uint8_t i = 0; i < msgsReceived.size (); i++)
    {
        CHECK (msgsReceived[i]);
    }
}

/* Test receiving one message one one channel and none on the other. */
TEST (NetworkManager_RecvMult, OneMsgRxdOneNot)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    TimeNs* pTime;
    TimeNs::getInstance (pTime);

    // Set up params.
    const uint32_t TIMEOUT_US = 1000;
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<uint8_t> sendBuf0 = {0x10, 0x01};
    std::vector<uint8_t> sendBuf1 = {0x01, 0x10};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<bool> msgsReceived (2);

    // Send message to Control Node from Device Node 0.
    CHECK_SUCCESS (pNmDev0->send (NetworkManager::CONTROL_NODE, sendBuf0));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    TimeNs::TimeNs_t startNs;
    TimeNs::TimeNs_t endNs;
    pTime->getTimeSinceInit (startNs);
    pNmCtrl->recvMult (TIMEOUT_US, nodes, bufs, msgsReceived);
    pTime->getTimeSinceInit (endNs);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    TimeNs::TimeNs_t elapsedNs = endNs - startNs;
    TimeNs::TimeNs_t timeoutNs = TIMEOUT_US * TimeNs::NS_IN_US;
    CHECK (elapsedNs > timeoutNs);
    CHECK_IN_BOUND (timeoutNs, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msg received only from Device Node 0.
    CHECK (sendBuf0 == bufs[0]);
    CHECK (msgsReceived[0]);
    CHECK_FALSE (msgsReceived[1]);

    // Send message to Control Node from Device Node 1.
    CHECK_SUCCESS (pNmDev1->send (NetworkManager::CONTROL_NODE, sendBuf1));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    pTime->getTimeSinceInit (startNs);
    pNmCtrl->recvMult (TIMEOUT_US, nodes, bufs, msgsReceived);
    pTime->getTimeSinceInit (endNs);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    elapsedNs = endNs - startNs;
    CHECK (elapsedNs > timeoutNs);
    CHECK_IN_BOUND (timeoutNs, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msg received only from Device Node 1.
    CHECK (sendBuf1 == bufs[1]);
    CHECK (msgsReceived[1]);
    CHECK_FALSE (msgsReceived[0]);
}

/* Verify can receive multiple msgs after recvMult called. */
TEST (NetworkManager_RecvMult, MsgsRxdAfterRecvMult)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    TimeNs* pTime;
    TimeNs::getInstance (pTime);

    // Create send threads. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread0;
    pthread_t thread1;
    struct ThreadFuncArgs argsThread0 = {&testLog, pNmDev0.get ()}; 
    struct ThreadFuncArgs argsThread1 = {&testLog, pNmDev1.get ()}; 
    ThreadManager::ThreadFunc_t *pThreadFuncSend = 
        (ThreadManager::ThreadFunc_t *) &threadFuncSend;
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread0, pThreadFuncSend,
                                    &argsThread0, sizeof (argsThread0),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread1, pThreadFuncSend,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));

    // Set up params. Threads send buffer = {0xff}.
    const uint32_t TIMEOUT_US = 1000;
    std::vector<NetworkManager::Node_t> nodes =
    {
        NetworkManager::DEVICE_NODE_0,
        NetworkManager::DEVICE_NODE_1,
    };
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (1);
    bufs[1].resize (1);
    std::vector<bool> msgsReceived (2);

    // Block on recvMult call.
    testLog.logEvent (Log::LogEvent_t::CALLED_RECVMULT, 0);    
    CHECK_SUCCESS (pNmCtrl->recvMult (TIMEOUT_US, nodes, bufs, msgsReceived)); 
    testLog.logEvent (Log::LogEvent_t::RECEIVED, 0);    

    // Verify received expected buffers.
    std::vector<uint8_t> expectedBuf = {0xff};
    CHECK (expectedBuf == bufs[0]);
    CHECK (expectedBuf == bufs[1]);

    // Verify testLog matches expected.
    expectedLog.logEvent (Log::LogEvent_t::CALLED_RECVMULT, 0);
    expectedLog.logEvent (Log::LogEvent_t::CALLED_SEND, 0);
    expectedLog.logEvent (Log::LogEvent_t::CALLED_SEND, 0);
    expectedLog.logEvent (Log::LogEvent_t::RECEIVED, 0);
    VERIFY_LOGS;

    // Clean up thread.
    WAIT_FOR_THREAD (thread0, pThreadManager);
    WAIT_FOR_THREAD (thread1, pThreadManager);
}
