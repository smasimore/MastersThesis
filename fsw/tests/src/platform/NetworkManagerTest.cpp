/* All #include statements should come before the CppUTest include */
#include <cstring>
#include <sstream>

#include "Errors.hpp"
#include "NetworkManager.hpp"
#include "Time.hpp"
#include "EnumClassHash.hpp"

#include "TestHelpers.hpp"

/********************************* MACROS *************************************/

/**
 * Initialize 3 Network Managers, one for each simulated node.
 */
#define INIT_NETWORK_MANAGERS                                                  \
    INIT_DATA_VECTOR (gDvConfig);                                              \
    std::shared_ptr<NetworkManager> pNmCtrl;                                   \
    std::shared_ptr<NetworkManager> pNmDev0;                                   \
    std::shared_ptr<NetworkManager> pNmDev1;                                   \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigCtrl, pDv,        \
                                              pNmCtrl));                       \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev0, pDv,        \
                                              pNmDev0));                       \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev1, pDv,        \
                                              pNmDev1));                       

/**
 * Check Data Vector values.
 *
 * @param  kX  DV_ELEM_TESTX expected value.
 */
#define CHECK_DV(k0, k1, k2, k3, k4, k5)                                       \
{                                                                              \
    uint32_t act0 = 0;                                                         \
    uint32_t act1 = 0;                                                         \
    uint32_t act2 = 0;                                                         \
    uint32_t act3 = 0;                                                         \
    uint32_t act4 = 0;                                                         \
    uint32_t act5 = 0;                                                         \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0, act0));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST1, act1));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST2, act2));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST3, act3));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST4, act4));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST5, act5));                           \
    CHECK_EQUAL (k0, act0);                                                    \
    CHECK_EQUAL (k1, act1);                                                    \
    CHECK_EQUAL (k2, act2);                                                    \
    CHECK_EQUAL (k3, act3);                                                    \
    CHECK_EQUAL (k4, act4);                                                    \
    CHECK_EQUAL (k5, act5);                                                    \
}

/*************************** VERIFY CONFIG TESTS ******************************/

/* Valid DV config to use for verify config tests. */
static DataVector::Config_t gDvConfig =
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT32 (DV_ELEM_TEST0, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST1, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST2, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST3, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST4, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST5, 0),
    }},
};

/* Valid NM config to use for verify config tests. */
static NetworkManager::Config_t gNmConfig =
{
    // Nodes
    {
        {NODE_CONTROL,  "10.0.0.1"},
        {NODE_DEVICE0, "10.0.0.2"},
    },

    // Channels
    {
        {NODE_CONTROL, 
         NODE_DEVICE0, 
         NetworkManager::MIN_PORT},
    },

    // Me
    NODE_CONTROL,

    // Msg tx count dv elem
    DV_ELEM_TEST0,

    // Msg rx count dv elem
    DV_ELEM_TEST1,
};

/* Group of tests verifying verifyConfig method. */
TEST_GROUP (NetworkManager_verifyConfig)
{

};

/* Test initializing with null DV pointer. */
TEST (NetworkManager_verifyConfig, DvNull)
{
    CHECK_ERROR (NetworkManager::verifyConfig (gNmConfig, nullptr), 
                 E_DATA_VECTOR_NULL);
}

/* Test initializing with empty node map. */
TEST (NetworkManager_verifyConfig, NoNodes)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Explicitly initialize empty node mpa.
    std::unordered_map<Node_t, 
                       NetworkManager::IP_t, 
                       EnumClassHash> emptyNodeToIP;

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp = emptyNodeToIP;

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_EMPTY_NODE_CONFIG);
}

/* Test initializing with empty channels list. */
TEST (NetworkManager_verifyConfig, NoChannels)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels = {};

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_EMPTY_CHANNEL_CONFIG);
}

/* Test initializing with invalid node enum. */
TEST (NetworkManager_verifyConfig, InvalidNode)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_LAST] = "10.0.0.3";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_INVALID_ENUM);
}

/* Test initializing with duplicate IP's. */
TEST (NetworkManager_verifyConfig, DupeIP)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_DEVICE0] = "10.0.0.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_DUPLICATE_IP);
}

/* Test initializing with non-numeric IP. */
TEST (NetworkManager_verifyConfig, NonNumericIP)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_DEVICE0] = "10.a.0.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_NON_NUMERIC_IP);
}

/* Test initializing with IP region value too high. */
TEST (NetworkManager_verifyConfig, InvalidIPRegion)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_DEVICE0] = "10.0.0.256";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_INVALID_IP_REGION);
}

/* Test initializing with empty IP. */
TEST (NetworkManager_verifyConfig, EmptyIP)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_DEVICE0] = "";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_INVALID_IP_SIZE);
}

/* Test initializing with too few IP regions. */
TEST (NetworkManager_verifyConfig, TooFewIPRegions)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_DEVICE0] = "10.0.0";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), E_INVALID_IP_SIZE);
}

/* Test initializing with too many IP regions. */
TEST (NetworkManager_verifyConfig, TooManyIPRegions)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.nodeToIp[NODE_DEVICE0] = "10.0.0.1.1";

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), E_INVALID_IP_SIZE);
}

/* Test channel with undefined node1. */
TEST (NetworkManager_verifyConfig, UndefinedNode1)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels[0].node1 = NODE_DEVICE1;

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_UNDEFINED_NODE_IN_CHANNEL);
}

/* Test channel with undefined node2. */
TEST (NetworkManager_verifyConfig, UndefinedNode2)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels[0].node2 = NODE_DEVICE1;

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_UNDEFINED_NODE_IN_CHANNEL);
}

/* Test initializing with port below min. */
TEST (NetworkManager_verifyConfig, InvalidPortMin)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels[0].port = NetworkManager::MIN_PORT - 1;

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), E_INVALID_PORT);
}

/* Test initializing with port above max. */
TEST (NetworkManager_verifyConfig, InvalidPortMax)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels[0].port = NetworkManager::MAX_PORT + 1;

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), E_INVALID_PORT);
}

/* Test initializing with undefined "me" node. */
TEST (NetworkManager_verifyConfig, UndefinedMeNode)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.me = NODE_DEVICE1;

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_UNDEFINED_ME_NODE);
}

/* Test multiple channels per node pair, same order. */
TEST (NetworkManager_verifyConfig, DuplicateChannelSameOrder)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels.push_back ({NODE_CONTROL, 
                                NODE_DEVICE0, 
                                NetworkManager::MIN_PORT});

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_DUPLICATE_CHANNEL);
}

/* Test multiple channels per node pair, different order. */
TEST (NetworkManager_verifyConfig, DuplicateChannelDifferentOrder)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.channels.push_back ({NODE_DEVICE0, 
                                NODE_CONTROL, 
                                NetworkManager::MIN_PORT});

    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_DUPLICATE_CHANNEL);
}

/* Test using DV elems not in DV. */
TEST (NetworkManager_verifyConfig, InvalidElems)
{
    INIT_DATA_VECTOR (gDvConfig);

    NetworkManager::Config_t config = gNmConfig;
    config.dvElemMsgTxCount = DV_ELEM_TEST6;
    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_INVALID_ELEM);

    config.dvElemMsgTxCount = DV_ELEM_TEST5;
    config.dvElemMsgRxCount = DV_ELEM_TEST6;
    CHECK_ERROR (NetworkManager::verifyConfig (config, pDv), 
                 E_INVALID_ELEM);
}

/* Test initializing with valid config. */
TEST (NetworkManager_verifyConfig, Success)
{
    INIT_DATA_VECTOR (gDvConfig);

    CHECK_SUCCESS (NetworkManager::verifyConfig (gNmConfig, pDv));
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
static const Time::TimeNs_t SELECT_OVERHEAD_NS = 250000;

/* Loopback nodes to use for send/recv tests. */
std::unordered_map<Node_t, 
                   NetworkManager::IP_t, 
                   EnumClassHash> gLoopbackNodes =
{
    {NODE_CONTROL, "127.0.0.1"},
    {NODE_DEVICE0, "127.0.0.2"},
    {NODE_DEVICE1, "127.0.0.3"},
};


/* Loopback channels to use for send/recv tests. */
std::vector<NetworkManager::ChannelConfig_t> gLoopbackChannels =
{
    {NODE_CONTROL, 
     NODE_DEVICE0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    {NODE_CONTROL, 
     NODE_DEVICE1, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
};

/* Loopback config for Control Node to use for send/recv tests. */
NetworkManager::Config_t gLoopbackConfigCtrl =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NODE_CONTROL,
    DV_ELEM_TEST0,
    DV_ELEM_TEST1,
};

/* Loopback config for Device Node 0 to use for send/recv tests. */
NetworkManager::Config_t gLoopbackConfigDev0 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NODE_DEVICE0,
    DV_ELEM_TEST2,
    DV_ELEM_TEST3,
};

/* Loopback config for Device Node 1 to use for send/recv tests. */
NetworkManager::Config_t gLoopbackConfigDev1 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NODE_DEVICE1,
    DV_ELEM_TEST4,
    DV_ELEM_TEST5,
};

/* Group of tests to verify send param checking. */
TEST_GROUP (NetworkManager_Send)
{

};

/* Test sending with empty buffer. */
TEST (NetworkManager_Send, SendEmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> sendBuf;
    CHECK_ERROR (pNmCtrl->send (NODE_CONTROL, sendBuf), E_EMPTY_BUFFER);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test sending with invalid node. */
TEST (NetworkManager_Send, SendInvalidNode)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> sendBuf = {0xff};
    CHECK_ERROR (pNmCtrl->send (NODE_DEVICE2, sendBuf), E_INVALID_NODE);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Group of tests to verify recvBlock param checking. */
TEST_GROUP (NetworkManager_RecvBlock)
{

};

/* Test recvBlock with empty buffer. */
TEST (NetworkManager_RecvBlock, EmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf;
    CHECK_ERROR (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf), E_EMPTY_BUFFER);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test recvBlock with buffer larger than max. */
TEST (NetworkManager_RecvBlock, BufferOverMax)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf (NetworkManager::MAX_RECV_BYTES + 1);
    CHECK_ERROR (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf),
                 E_GREATER_THAN_MAX_RECV_BYTES);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test recvBlock with invalid node. */
TEST (NetworkManager_RecvBlock, InvalidNode)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf (1);
    CHECK_ERROR (pNmCtrl->recvBlock (NODE_DEVICE2, recvBuf), E_INVALID_NODE);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test receiving a message bigger than expected. */
TEST (NetworkManager_RecvBlock, BufferTooSmall)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf));
    CHECK_ERROR (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf), 
                 E_UNEXPECTED_RECV_SIZE);

    // Expect 1 msg sent from dn0.
    CHECK_DV (0, 0, 1, 0, 0, 0);
}

/* Test receiving a message smaller than expected. */
TEST (NetworkManager_RecvBlock, BufferTooBig)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (3, 0);
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf));
    CHECK_ERROR (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf), 
                 E_UNEXPECTED_RECV_SIZE);

    // Expect 1 msg sent from dn0.
    CHECK_DV (0, 0, 1, 0, 0, 0);
}

/* Group of tests to verify recvNoBlock param checking. */
TEST_GROUP (NetworkManager_RecvNoBlock)
{

};

/* Test recvNoBlock with empty buffer. */
TEST (NetworkManager_RecvNoBlock, EmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf;
    bool msgRecvd = false;
    CHECK_ERROR (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd), 
                 E_EMPTY_BUFFER);

    // Expect no msgs tx'd/rx'd.
    CHECK_FALSE (msgRecvd);
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test recvNoBlock with buffer larger than max. */
TEST (NetworkManager_RecvNoBlock, BufferOverMax)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf (NetworkManager::MAX_RECV_BYTES + 1);
    bool msgRecvd = false;
    CHECK_ERROR (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd),
                 E_GREATER_THAN_MAX_RECV_BYTES);

    // Expect no msgs tx'd/rx'd.
    CHECK_FALSE (msgRecvd);
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test recvNoBlock with invalid node. */
TEST (NetworkManager_RecvNoBlock, InvalidNode)
{
    INIT_NETWORK_MANAGERS

    std::vector<uint8_t> recvBuf (1);
    bool msgRecvd = false;
    CHECK_ERROR (pNmCtrl->recvNoBlock (NODE_DEVICE2, recvBuf, msgRecvd), 
                 E_INVALID_NODE);

    // Expect no msgs tx'd/rx'd.
    CHECK_FALSE (msgRecvd);
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test receiving a message bigger than expected. */
TEST (NetworkManager_RecvNoBlock, BufferTooSmall)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (1, 0);
    bool msgRecvd = false;
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf));
    CHECK_ERROR (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd), 
                 E_UNEXPECTED_RECV_SIZE);

    // Expect 1 msg sent from dn0.
    CHECK_FALSE (msgRecvd);
    CHECK_DV (0, 0, 1, 0, 0, 0);
}

/* Test receiving a message smaller than expected. */
TEST (NetworkManager_RecvNoBlock, BufferTooBig)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<uint8_t> recvBuf (3, 0);
    bool msgRecvd = false;
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf));
    CHECK_ERROR (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd), 
                 E_UNEXPECTED_RECV_SIZE);

    // Expect 1 msg sent from dn0.
    CHECK_FALSE (msgRecvd);
    CHECK_DV (0, 0, 1, 0, 0, 0);
}

/* Group of tests to verify sending and receiving messages. */
TEST_GROUP (NetworkManager_SendRecv)
{

};

/* Send and receive a message successfully. */
TEST (NetworkManager_SendRecv, SuccessOneMessagePerChannel)
{
    INIT_NETWORK_MANAGERS

    // Send and receive a message using loopback.
    std::vector<uint8_t> sendBuf0 = {0xff};
    std::vector<uint8_t> sendBuf1 = {0x01};
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1));

    // Receive using recvBlock and verify buffers.
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf));
    CHECK (sendBuf0 == recvBuf);
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE1, recvBuf));
    CHECK (sendBuf1 == recvBuf);

    // Repeat with recvNoBlock.
    bool msg0Recvd = false;
    bool msg1Recvd = false;
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1));
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msg0Recvd));
    CHECK (msg0Recvd);
    CHECK (sendBuf0 == recvBuf);
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE1, recvBuf, msg1Recvd));
    CHECK (msg1Recvd);
    CHECK (sendBuf1 == recvBuf);

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 4, 2, 0, 2, 0);
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
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0Msg1));
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0Msg2));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1Msg1));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1Msg2));
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf0Msg1));
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf0Msg2));
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE1, recvBuf1Msg1));
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE1, recvBuf1Msg2));

    // Receive with recvBlock and verify buffers.
    CHECK (sendBuf0Msg1 == recvBuf0Msg1);
    CHECK (sendBuf0Msg2 == recvBuf0Msg2);
    CHECK (sendBuf1Msg1 == recvBuf1Msg1);
    CHECK (sendBuf1Msg2 == recvBuf1Msg2);

    // Repeat with recvNoBlock.
    bool msg01Recvd = false;
    bool msg02Recvd = false;
    bool msg11Recvd = false;
    bool msg12Recvd = false;
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0Msg1));
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0Msg2));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1Msg1));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1Msg2));
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf0Msg1, 
                                         msg01Recvd));
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf0Msg2,
                                         msg02Recvd));
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE1, recvBuf1Msg1,
                                         msg11Recvd));
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE1, recvBuf1Msg2,
                                         msg12Recvd));
    CHECK (msg01Recvd);
    CHECK (msg02Recvd);
    CHECK (msg11Recvd);
    CHECK (msg12Recvd);
    CHECK (sendBuf0Msg1 == recvBuf0Msg1);
    CHECK (sendBuf0Msg2 == recvBuf0Msg2);
    CHECK (sendBuf1Msg1 == recvBuf1Msg1);
    CHECK (sendBuf1Msg2 == recvBuf1Msg2);

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 8, 4, 0, 4, 0);
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
static void* funcSend (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs = (struct ThreadFuncArgs *) rawArgs;
    Log* log = pArgs->log;
    NetworkManager* pNmDev0 = pArgs->pNm;

    log->logEvent (Log::LogEvent_t::CALLED_SEND, 0);    
    std::vector<uint8_t> sendBuf = {0xff};
    ret = pNmDev0->send (NODE_CONTROL, sendBuf);
    return (void *) ret;
}

/* Verify thread blocks when no data in recv buffer and recvBlock called. */
TEST (NetworkManager_SendRecv, BlockOnRecvBlock)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_NETWORK_MANAGERS

    // Create send thread. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread;
    struct ThreadFuncArgs argsThread = {&testLog, pNmDev0.get ()}; 
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread, 
                                    (ThreadManager::ThreadFunc_t) funcSend,
                                    &argsThread, sizeof (argsThread),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));

    // Block on recv call.
    testLog.logEvent (Log::LogEvent_t::CALLED_RECV, 0);    
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf));
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

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 1, 1, 0, 0, 0);
}

/* Verify thread does not block when no data in recv buffer and recvNoBlock 
   called. */
TEST (NetworkManager_SendRecv, NoBlockOnRecvNoBlock)
{
    INIT_NETWORK_MANAGERS

    // Do not block on recv call.
    std::vector<uint8_t> recvBuf (1, 0);
    bool msgRecvd = false;
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd));

    // Verify no message received. If get this far, did not block as expected.
    CHECK_FALSE (msgRecvd);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Verify thread can call recvBlock then recvNoBlock. */
TEST (NetworkManager_SendRecv, BlockThenNoBlock)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_NETWORK_MANAGERS

    // Create send thread. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread;
    struct ThreadFuncArgs argsThread = {&testLog, pNmDev0.get ()}; 
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread, 
                                    (ThreadManager::ThreadFunc_t) funcSend,
                                    &argsThread, sizeof (argsThread),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));

    // Block on recv call.
    std::vector<uint8_t> recvBuf (1, 0);
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf));

    // Do not block on next recv call.
    bool msgRecvd = false;
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd));
    CHECK_FALSE (msgRecvd);

    // Clean up thread.
    WAIT_FOR_THREAD (thread, pThreadManager);

    // Expect 1 msg tx'd/rx'd.
    CHECK_DV (0, 1, 1, 0, 0, 0);
}

/* Verify thread can call recvNoBlock then recvBlock. */
TEST (NetworkManager_SendRecv, NoBlockThenBlock)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_NETWORK_MANAGERS

    // Do not block on first recv call.
    std::vector<uint8_t> recvBuf (1, 0);
    bool msgRecvd = false;
    CHECK_SUCCESS (pNmCtrl->recvNoBlock (NODE_DEVICE0, recvBuf, msgRecvd));
    CHECK_FALSE (msgRecvd);

    // Create send thread. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread;
    struct ThreadFuncArgs argsThread = {&testLog, pNmDev0.get ()}; 
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread, 
                                    (ThreadManager::ThreadFunc_t) funcSend,
                                    &argsThread, sizeof (argsThread),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));

    // Block on next recv call.
    CHECK_SUCCESS (pNmCtrl->recvBlock (NODE_DEVICE0, recvBuf));

    // Clean up thread.
    WAIT_FOR_THREAD (thread, pThreadManager);

    // Expect 1 msg tx'd/rx'd.
    CHECK_DV (0, 1, 1, 0, 0, 0);
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
    std::vector<Node_t> nodes;
    std::vector<std::vector<uint8_t>> bufs;
    std::vector<uint32_t> msgsReceived;

    // Num nodes different.
    nodes.resize (1);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, nodes, bufs, 
                                    msgsReceived),
                 E_VECTORS_DIFF_SIZES);

    // Num buffs different.
    msgsReceived.resize (1);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, nodes, bufs, 
                                    msgsReceived),
                 E_VECTORS_DIFF_SIZES);

    // Num msgsReceived different.
    msgsReceived.resize (2);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, nodes, bufs, 
                                    msgsReceived),
                 E_VECTORS_DIFF_SIZES);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test timeout too large. */
TEST (NetworkManager_RecvMult, LargeTimeout)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<Node_t> nodes;
    std::vector<std::vector<uint8_t>> bufs;
    std::vector<uint32_t> msgsReceived;

    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS + 1, nodes, 
                                    bufs, msgsReceived),
                 E_TIMEOUT_TOO_LARGE);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test empty buffers. */
TEST (NetworkManager_RecvMult, EmptyBuffer)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<std::vector<uint8_t>> bufsFirstEmpty (2);
    bufsFirstEmpty[1].resize (1);
    std::vector<std::vector<uint8_t>> bufsSecondEmpty (2); 
    bufsSecondEmpty[0].resize (1);
    std::vector<uint32_t> msgsReceived (2);

    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, nodes, 
                                    bufsFirstEmpty, msgsReceived),
                 E_EMPTY_BUFFER);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, nodes, 
                                    bufsSecondEmpty, msgsReceived),
                 E_EMPTY_BUFFER);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test invalid nodes. */
TEST (NetworkManager_RecvMult, InvalidNode)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<Node_t> nodesFirstInvalid = {NODE_CONTROL, NODE_DEVICE1};
    std::vector<Node_t> nodesSecondInvalid = {NODE_DEVICE0, NODE_DEVICE2};
    std::vector<std::vector<uint8_t>> bufs = {{0xff}, {0xff}};
    std::vector<uint32_t> msgsReceived (2);

    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, 
                                    nodesFirstInvalid, bufs, msgsReceived),
                 E_INVALID_NODE);
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, 
                                    nodesSecondInvalid, bufs, msgsReceived),
                 E_INVALID_NODE);

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test buffer size smaller than received message. */
TEST (NetworkManager_RecvMult, BufferSizeTooSmall)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<uint8_t> buf = {0xff, 0xff};
    std::vector<std::vector<uint8_t>> bufsFirstTooSmall (2);
    bufsFirstTooSmall[0].resize (1);
    bufsFirstTooSmall[1].resize (2);
    std::vector<std::vector<uint8_t>> bufsSecondTooSmall (2);
    bufsSecondTooSmall[0].resize (2);
    bufsSecondTooSmall[1].resize (1);
    std::vector<uint32_t> msgsReceived (2);

    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, buf));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, buf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, 
                                    nodes, bufsFirstTooSmall, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);

    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, buf));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, buf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, 
                                    nodes, bufsSecondTooSmall, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);

    // Expect all msgs tx'd but only 1 rx'd before recvMult fails during the
    // second call.
    CHECK_DV (0, 1, 2, 0, 2, 0);
}

/* Test buffer size larger than received message. */
TEST (NetworkManager_RecvMult, BufferSizeTooLarge)
{
    INIT_NETWORK_MANAGERS

    // Set up params.
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};

    std::vector<uint8_t> sendBuf = {0xff, 0xff};
    std::vector<std::vector<uint8_t>> bufsFirstTooLarge (2);
    bufsFirstTooLarge[0].resize (3);
    bufsFirstTooLarge[1].resize (2);
    std::vector<std::vector<uint8_t>> bufsSecondTooLarge (2);
    bufsSecondTooLarge[0].resize (2);
    bufsSecondTooLarge[1].resize (3);
    std::vector<uint32_t> msgsReceived (2);

    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, 
                                    nodes, bufsFirstTooLarge, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);

    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf));
    CHECK_ERROR (pNmCtrl->recvMult (NetworkManager::MAX_TIMEOUT_NS, 
                                    nodes, bufsSecondTooLarge, msgsReceived),
                 E_UNEXPECTED_RECV_SIZE);

    // Expect all msgs tx'd but only 1 rx'd before recvMult fails during the
    // second call.
    CHECK_DV (0, 1, 2, 0, 2, 0);
}

/* Test receiving two messages sent before recvMult called. */
TEST (NetworkManager_RecvMult, MsgsRxdBeforeRecvMult)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    Time* pTime;
    Time::getInstance (pTime);

    // Set up params.
    const Time::TimeNs_t TIMEOUT_NS  = 2 * Time::NS_IN_MS;
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<uint8_t> sendBuf0 = {0x10, 0x01};
    std::vector<uint8_t> sendBuf1 = {0x01, 0x10};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<uint32_t> msgsReceived (2);

    // Send messages to Control Node.
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    Time::TimeNs_t startNs;
    Time::TimeNs_t endNs;
    pTime->getTimeNs (startNs);
    CHECK_SUCCESS (pNmCtrl->recvMult (TIMEOUT_NS, nodes, 
                                      bufs, msgsReceived));
    pTime->getTimeNs (endNs);

    // Verify buffers match.
    CHECK (bufs[0] == sendBuf0);
    CHECK (bufs[1] == sendBuf1);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    Time::TimeNs_t elapsedNs = endNs - startNs;
    CHECK (elapsedNs > TIMEOUT_NS);
    CHECK_IN_BOUND (TIMEOUT_NS, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msgsReceived all set to 1.
    for (uint8_t i = 0; i < msgsReceived.size (); i++)
    {
        CHECK_EQUAL (1, msgsReceived[i]);
    }

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 2, 1, 0, 1, 0);
}

/* Test receiving no messages. */
TEST (NetworkManager_RecvMult, NoMsgs)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    Time* pTime;
    Time::getInstance (pTime);

    // Set up params.
    const Time::TimeNs_t TIMEOUT_NS = 1 * Time::NS_IN_MS;
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<uint32_t> msgsReceived (2);

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    Time::TimeNs_t startNs;
    Time::TimeNs_t endNs;
    pTime->getTimeNs (startNs);
    pNmCtrl->recvMult (TIMEOUT_NS, nodes, bufs, msgsReceived);
    pTime->getTimeNs (endNs);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    Time::TimeNs_t elapsedNs = endNs - startNs;
    CHECK (elapsedNs > TIMEOUT_NS);
    CHECK_IN_BOUND (TIMEOUT_NS, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msgsReceived all set to 0.
    for (uint8_t i = 0; i < msgsReceived.size (); i++)
    {
        CHECK_EQUAL (0, msgsReceived[i]);
    }

    // Expect no msgs tx'd/rx'd.
    CHECK_DV (0, 0, 0, 0, 0, 0);
}

/* Test sending multiple messages on one channel. Expect to only recv first. */
TEST (NetworkManager_RecvMult, MultMsgsPerChannel)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    Time* pTime;
    Time::getInstance (pTime);

    // Set up params.
    const Time::TimeNs_t TIMEOUT_NS = 1 * Time::NS_IN_MS;
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<uint8_t> sendBuf0 = {0x10, 0x01};
    std::vector<uint8_t> sendBuf0SecondMsg = {0x11, 0x22};
    std::vector<uint8_t> sendBuf1 = {0x01, 0x10};
    std::vector<uint8_t> sendBuf1SecondMsg = {0x33, 0x44};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<uint32_t> msgsReceived (2);

    // Send messages to Control Node.
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1));
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0SecondMsg));
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1SecondMsg));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    Time::TimeNs_t startNs;
    Time::TimeNs_t endNs;
    pTime->getTimeNs (startNs);
    CHECK_SUCCESS (pNmCtrl->recvMult (TIMEOUT_NS, nodes, bufs, msgsReceived));
    pTime->getTimeNs (endNs);

    // Verify buffers match.
    CHECK (bufs[0] == sendBuf0SecondMsg);
    CHECK (bufs[1] == sendBuf1SecondMsg);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    Time::TimeNs_t elapsedNs = endNs - startNs;
    CHECK (elapsedNs > TIMEOUT_NS);
    CHECK_IN_BOUND (TIMEOUT_NS, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify received 2 msgs from each node.
    CHECK_EQUAL (2, msgsReceived[0]);
    CHECK_EQUAL (2, msgsReceived[1]);

    // Expect all msgs tx'd and rx'd.
    CHECK_DV (0, 4, 2, 0, 2, 0);
}

/* Test receiving one message on one channel and none on the other. */
TEST (NetworkManager_RecvMult, OneMsgRxdOneNot)
{
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    Time* pTime;
    Time::getInstance (pTime);

    // Set up params.
    const Time::TimeNs_t TIMEOUT_NS = 1 * Time::NS_IN_MS;
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<uint8_t> sendBuf0 = {0x10, 0x01};
    std::vector<uint8_t> sendBuf1 = {0x01, 0x10};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (2);
    bufs[1].resize (2);
    std::vector<uint32_t> msgsReceived (2);

    // Send message to Control Node from Device Node 0.
    CHECK_SUCCESS (pNmDev0->send (NODE_CONTROL, sendBuf0));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    Time::TimeNs_t startNs;
    Time::TimeNs_t endNs;
    pTime->getTimeNs (startNs);
    pNmCtrl->recvMult (TIMEOUT_NS, nodes, bufs, msgsReceived);
    pTime->getTimeNs (endNs);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    Time::TimeNs_t elapsedNs = endNs - startNs;
    CHECK (elapsedNs > TIMEOUT_NS);
    CHECK_IN_BOUND (TIMEOUT_NS, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msg received only from Device Node 0.
    CHECK (sendBuf0 == bufs[0]);
    CHECK_EQUAL (1, msgsReceived[0]);
    CHECK_EQUAL (0, msgsReceived[1]);

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 1, 1, 0, 0, 0);

    // Send message to Control Node from Device Node 1.
    CHECK_SUCCESS (pNmDev1->send (NODE_CONTROL, sendBuf1));

    // Receive messages from Device Nodes. Time receive to ensure returns well
    // before timeout.
    pTime->getTimeNs (startNs);
    pNmCtrl->recvMult (TIMEOUT_NS, nodes, bufs, msgsReceived);
    pTime->getTimeNs (endNs);

    // Verify time taken is greater than or equal to timeout and within 
    // expected bounds.
    elapsedNs = endNs - startNs;
    CHECK (elapsedNs > TIMEOUT_NS);
    CHECK_IN_BOUND (TIMEOUT_NS, elapsedNs, SELECT_OVERHEAD_NS);

    // Verify msg received only from Device Node 1.
    CHECK (sendBuf1 == bufs[1]);
    CHECK_EQUAL (1, msgsReceived[1]);
    CHECK_EQUAL (0, msgsReceived[0]);

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 2, 1, 0, 1, 0);
}

/* Verify can receive multiple msgs after recvMult called. */
TEST (NetworkManager_RecvMult, MsgsRxdAfterRecvMult)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_NETWORK_MANAGERS

    // Init Time Module to measure time recvMult takes.
    Time* pTime;
    Time::getInstance (pTime);

    // Create send threads. Thread should not run until cpputest thread blocks,
    // since it is lower pri than the cpputest thread.
    pthread_t thread0;
    pthread_t thread1;
    struct ThreadFuncArgs argsThread0 = {&testLog, pNmDev0.get ()}; 
    struct ThreadFuncArgs argsThread1 = {&testLog, pNmDev1.get ()}; 
    ThreadManager::ThreadFunc_t threadFuncSend = 
        (ThreadManager::ThreadFunc_t) funcSend;
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread0, threadFuncSend,
                                    &argsThread0, sizeof (argsThread0),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    CHECK_SUCCESS (pThreadManager->createThread (
                                    thread1, threadFuncSend,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));

    // Set up params. Threads send buffer = {0xff}.
    const Time::TimeNs_t TIMEOUT_NS = 1 * Time::NS_IN_MS;
    std::vector<Node_t> nodes = {NODE_DEVICE0, NODE_DEVICE1};
    std::vector<std::vector<uint8_t>> bufs (2);
    bufs[0].resize (1);
    bufs[1].resize (1);
    std::vector<uint32_t> msgsReceived (2);

    // Block on recvMult call.
    testLog.logEvent (Log::LogEvent_t::CALLED_RECVMULT, 0);    
    CHECK_SUCCESS (pNmCtrl->recvMult (TIMEOUT_NS, nodes, bufs, msgsReceived)); 
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

    // Expect all msgs tx'd/rx'd.
    CHECK_DV (0, 2, 1, 0, 1, 0);
}
