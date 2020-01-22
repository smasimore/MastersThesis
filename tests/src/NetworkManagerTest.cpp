/* All #include statements should come before the CppUTest include */
#include <cstring>
#include <sstream>

#include "Errors.h"
#include "NetworkManager.hpp"
#include "EnumClassHash.hpp"

#include "TestHelpers.hpp"

/*************************** VERIFY CONFIG TESTS ******************************/

/* Valid config to use for verify config tests. */
NetworkManager::NetworkManagerConfig_t gValidConfig =
{
    // Nodes
    {
        {NetworkManager::Node_t::FLIGHT_COMPUTER, "10.0.0.1"},
        {NetworkManager::Node_t::REMOTE_IO_0,     "10.0.0.2"},
    },

    // Channels
    {
        {NetworkManager::Node_t::FLIGHT_COMPUTER, 
         NetworkManager::Node_t::REMOTE_IO_0, 
         NetworkManager::MIN_PORT},
    },

    // Me
    NetworkManager::Node_t::FLIGHT_COMPUTER,
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

    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp = emptyNodeToIP;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_EMPTY_NODE_CONFIG);
}

/* Test initializing with empty channels list. */
TEST (NetworkManager_verifyConfig, NoChannels)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels = {};

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_EMPTY_CHANNEL_CONFIG);
}

/* Test initializing with invalid node enum. */
TEST (NetworkManager_verifyConfig, InvalidNode)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::LAST] = "10.0.0.3";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_ENUM);
}

/* Test initializing with duplicate IP's. */
TEST (NetworkManager_verifyConfig, DupeIP)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::REMOTE_IO_0] = "10.0.0.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_DUPLICATE_IP);
}

/* Test initializing with non-numeric IP. */
TEST (NetworkManager_verifyConfig, NonNumericIP)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::REMOTE_IO_0] = "10.a.0.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_NON_NUMERIC_IP);
}

/* Test initializing with IP region value too high. */
TEST (NetworkManager_verifyConfig, InvalidIPRegion)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::REMOTE_IO_0] = "10.0.0.256";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_REGION);
}

/* Test initializing with empty IP. */
TEST (NetworkManager_verifyConfig, EmptyIP)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::REMOTE_IO_0] = "";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_SIZE);
}

/* Test initializing with too few IP regions. */
TEST (NetworkManager_verifyConfig, TooFewIPRegions)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::REMOTE_IO_0] = "10.0.0";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_SIZE);
}

/* Test initializing with too many IP regions. */
TEST (NetworkManager_verifyConfig, TooManyIPRegions)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.nodeToIp[NetworkManager::Node_t::REMOTE_IO_0] = "10.0.0.1.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_IP_SIZE);
}

/* Test channel with undefined node1. */
TEST (NetworkManager_verifyConfig, UndefinedNode1)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels[0].node1 = NetworkManager::Node_t::REMOTE_IO_1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), 
                 E_UNDEFINED_NODE_IN_CHANNEL);
}

/* Test channel with undefined node2. */
TEST (NetworkManager_verifyConfig, UndefinedNode2)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels[0].node2 = NetworkManager::Node_t::REMOTE_IO_1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), 
                 E_UNDEFINED_NODE_IN_CHANNEL);
}

/* Test initializing with port below min. */
TEST (NetworkManager_verifyConfig, InvalidPortMin)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels[0].port = NetworkManager::MIN_PORT - 1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_PORT);
}

/* Test initializing with port above max. */
TEST (NetworkManager_verifyConfig, InvalidPortMax)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels[0].port = NetworkManager::MAX_PORT + 1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_INVALID_PORT);
}

/* Test initializing with undefined "me" node. */
TEST (NetworkManager_verifyConfig, UndefinedMeNode)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.me = NetworkManager::Node_t::REMOTE_IO_1;

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_UNDEFINED_ME_NODE);
}

/* Test multiple channels per node pair, same order. */
TEST (NetworkManager_verifyConfig, DuplicateChannelSameOrder)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels.push_back ({NetworkManager::Node_t::FLIGHT_COMPUTER, 
                               NetworkManager::Node_t::REMOTE_IO_0, 
                               NetworkManager::MIN_PORT});

    CHECK_ERROR (NetworkManager::verifyConfig (config), E_DUPLICATE_CHANNEL);
}

/* Test multiple channels per node pair, different order. */
TEST (NetworkManager_verifyConfig, DuplicateChannelDifferentOrder)
{
    NetworkManager::NetworkManagerConfig_t config = gValidConfig;
    config.channels.push_back ({NetworkManager::Node_t::REMOTE_IO_0, 
                                NetworkManager::Node_t::FLIGHT_COMPUTER, 
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

/*********************** SINGLE NODE SEND/RECV TESTS **************************/

/* Loopback config to use for send/recv tests. */
NetworkManager::NetworkManagerConfig_t gLoopbackConfig =
{
    {
        {NetworkManager::Node_t::FLIGHT_COMPUTER, "127.0.0.1"},
    },

    {
        {NetworkManager::Node_t::FLIGHT_COMPUTER, 
         NetworkManager::Node_t::FLIGHT_COMPUTER, 
         NetworkManager::MIN_PORT},
    },

    NetworkManager::Node_t::FLIGHT_COMPUTER,
};

/* Group of tests to verify send and recv functions using 1 nod. */
TEST_GROUP (NetworkManager_SendRecv)
{

};

/* Test sending with empty buffer. */
TEST (NetworkManager_SendRecv, SendEmptyBuffer)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    std::vector<uint8_t> sendBuf;
    CHECK_ERROR (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                            sendBuf), E_EMPTY_BUFFER);
}

/* Test sending with invalid node. */
TEST (NetworkManager_SendRecv, SendInvalidNode)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    std::vector<uint8_t> sendBuf = {0xff};
    CHECK_ERROR (pNm->send (NetworkManager::Node_t::REMOTE_IO_0, 
                            sendBuf), E_INVALID_NODE);
}

/* Test recv'ing with empty buffer. */
TEST (NetworkManager_SendRecv, RecvEmptyBuffer)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    std::vector<uint8_t> recvBuf;
    CHECK_ERROR (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                            recvBuf), E_EMPTY_BUFFER);
}

/* Test recv'ing with invalid node. */
TEST (NetworkManager_SendRecv, RecvInvalidNode)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    std::vector<uint8_t> recvBuf (1);
    CHECK_ERROR (pNm->recv (NetworkManager::Node_t::REMOTE_IO_0, 
                            recvBuf), E_INVALID_NODE);
}

/* Test receiving a message bigger than expected. */
TEST (NetworkManager_SendRecv, RecvBufferTooSmall)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf));
    CHECK_ERROR (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, recvBuf), 
                            E_UNEXPECTED_RECV_SIZE);
}

/* Test receiving a message smaller than expected. */
TEST (NetworkManager_SendRecv, RecvBufferTooBig)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (3, 0);
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf));
    CHECK_ERROR (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, recvBuf), 
                            E_UNEXPECTED_RECV_SIZE);
}

/* Send and receive a message successfully. */
TEST (NetworkManager_SendRecv, SuccessOneMessage)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff};
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf));
    CHECK_SUCCESS (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   recvBuf));

    // Verify received sent buffer.
    CHECK (sendBuf == recvBuf);
}

/* Send and receive two messages successfully. */
TEST (NetworkManager_SendRecv, SuccessTwoMessages)
{
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf1 = {0xff};
    std::vector<uint8_t> recvBuf1 (1, 0);
    std::vector<uint8_t> sendBuf2 = {0xff, 0x00};
    std::vector<uint8_t> recvBuf2 (2, 0);
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf1));
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf2));
    CHECK_SUCCESS (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   recvBuf1));
    CHECK_SUCCESS (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   recvBuf2));

    CHECK (sendBuf1 == recvBuf1);
    CHECK (sendBuf2 == recvBuf2);
}

/* Send and receive max packet size and 2x max packet size. */
TEST (NetworkManager_SendRecv, MaxPacketSize)
{
    const uint32_t MAX_PACKET_SIZE = 1500;
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf1 (MAX_PACKET_SIZE, 0xff);
    std::vector<uint8_t> recvBuf1 (MAX_PACKET_SIZE, 0);
    std::vector<uint8_t> sendBuf2 (MAX_PACKET_SIZE * 2, 0xff);
    std::vector<uint8_t> recvBuf2 (MAX_PACKET_SIZE * 2, 0);
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf1));
    CHECK_SUCCESS (pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   sendBuf2));
    CHECK_SUCCESS (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   recvBuf1));
    CHECK_SUCCESS (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
                   recvBuf2));

    CHECK (sendBuf1 == recvBuf1);
    CHECK (sendBuf2 == recvBuf2);
}

/**
 * Params to pass Network Manager to thread functions.
 */
struct ThreadFuncArgs 
{
    Log* log;
    std::shared_ptr<NetworkManager> pNm;
};

/**
 * Thread that sends a message.
 */
static void* threadFuncSend (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs     = (struct ThreadFuncArgs *) rawArgs;
    Log* log = pArgs->log;
    std::shared_ptr<NetworkManager> pNm = pArgs->pNm;

    log->logEvent (Log::LogEvent_t::CALLED_SEND, 0);    
    std::vector<uint8_t> sendBuf = {0xff};
    ret = pNm->send (NetworkManager::Node_t::FLIGHT_COMPUTER, sendBuf);
    return (void *) ret;
}

/* Verify thread blocks when no data in recv buffer. */
TEST (NetworkManager_SendRecv, BlockOnRecv)
{
    INIT_THREAD_MANAGER_AND_LOGS;

    // Init Network Manager.
    std::shared_ptr<NetworkManager> pNm; 
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfig, pNm));

    // Create send thread. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread;
    struct ThreadFuncArgs argsThread = {&testLog, pNm}; 
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
    CHECK_SUCCESS (pNm->recv (NetworkManager::Node_t::FLIGHT_COMPUTER, 
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
    Error_t threadReturn;
    ret = pThreadManager->waitForThread (thread, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (E_SUCCESS, threadReturn);
}
