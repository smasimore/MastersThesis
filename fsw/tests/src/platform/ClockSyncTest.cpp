#include <iostream>
#include <memory>
#include <unistd.h>
#include <vector>

#include "Errors.hpp"
#include "NetworkManager.hpp"
#include "ClockSync.hpp"

/* All #include statements should come before the TestHelpers.hpp */
#include "TestHelpers.hpp"

#define PIDOF_CMD         "pidof -x /usr/sbin/ntpd > /dev/null 2>&1"
#define NTPD_START_CMD    "/etc/init.d/ntpd start > /dev/null 2>&1"
#define NTPD_STOP_CMD     "/etc/init.d/ntpd stop > /dev/null 2>&1"
#define PIDOF_RET_SUCCESS 0
#define PIDOF_RET_FAIL    256

/********************************* MACROS *************************************/

/**
 * Initialize CN and DN Network Managers.
 */
#define INIT_NETWORK_MANAGERS                                                  \
    INIT_DATA_VECTOR (gDvConfig);                                              \
    std::shared_ptr<NetworkManager> pNmCtrl;                                   \
    std::shared_ptr<NetworkManager> pNmDev0;                                   \
    std::shared_ptr<NetworkManager> pNmDev1;                                   \
    std::shared_ptr<NetworkManager> pNmDev2;                                   \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigCtrl, pDv,        \
                                              pNmCtrl));                       \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev0, pDv,        \
                                              pNmDev0));                       \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev1, pDv,        \
                                              pNmDev1));                       \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev2, pDv,        \
                                              pNmDev2));

/**
 * Create client thread.
 *
 * @param  kThread  pthread_t to fill.
 * @param  kPNm     Pointer to Network Manager.
 * @param  kFunc    Function to run in thread.
 */
#define CREATE_CLIENT_THREAD(kThread, kPNm, kFunc)                             \
{                                                                              \
    struct ThreadFuncArgs argsThread = {kPNm.get ()};                          \
    ThreadManager::ThreadFunc_t *pThreadFunc =                                 \
        (ThreadManager::ThreadFunc_t *) &kFunc;                                \
    CHECK_SUCCESS (pTm->createThread (kThread, pThreadFunc, &argsThread,       \
                                      sizeof (argsThread),                     \
                                      ThreadManager::MIN_NEW_THREAD_PRIORITY,  \
                                      ThreadManager::Affinity_t::CORE_0));     \
}

/**
 * Verify ntpd state.
 *
 * @param  kRunningExp  Expected running state.
 *
 */
#define VERIFY_NTPD_STATE(kRunningExp)                                         \
{                                                                              \
    int32_t expectedPidofRet = kRunningExp == true                             \
        ? PIDOF_RET_SUCCESS                                                    \
        : PIDOF_RET_FAIL;                                                      \
    CHECK_EQUAL (expectedPidofRet, system (PIDOF_CMD));                        \
}

/**
 * Check Data Vector values.
 *
 * @param  kX  DV_ELEM_TESTX expected value.
 */
#define CHECK_DV(k0, k1, k2, k3, k4, k5, k6, k7)                               \
{                                                                              \
    uint32_t act0 = 0;                                                         \
    uint32_t act1 = 0;                                                         \
    uint32_t act2 = 0;                                                         \
    uint32_t act3 = 0;                                                         \
    uint32_t act4 = 0;                                                         \
    uint32_t act5 = 0;                                                         \
    uint32_t act6 = 0;                                                         \
    uint32_t act7 = 0;                                                         \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0, act0));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST1, act1));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST2, act2));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST3, act3));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST4, act4));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST5, act5));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST6, act6));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST7, act7));                           \
    CHECK_EQUAL (k0, act0);                                                    \
    CHECK_EQUAL (k1, act1);                                                    \
    CHECK_EQUAL (k2, act2);                                                    \
    CHECK_EQUAL (k3, act3);                                                    \
    CHECK_EQUAL (k4, act4);                                                    \
    CHECK_EQUAL (k5, act5);                                                    \
    CHECK_EQUAL (k6, act6);                                                    \
    CHECK_EQUAL (k7, act7);                                                    \
}

/********************************* CONFIGS ************************************/

/**
 * Data Vector config. 
 */
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
        DV_ADD_UINT32 (DV_ELEM_TEST6, 0),
        DV_ADD_UINT32 (DV_ELEM_TEST7, 0),
    }},
};

/**
 * Vector of clients.
 */
static std::vector<NetworkManager::Node_t> gClients =
{
    NetworkManager::Node_t::DEVICE_NODE_0,
    NetworkManager::Node_t::DEVICE_NODE_1,
    NetworkManager::Node_t::DEVICE_NODE_2,
};

/* Loopback nodes to use for send/recv tests. */
static std::unordered_map<NetworkManager::Node_t, 
                          NetworkManager::IP_t, 
                          EnumClassHash> gLoopbackNodes =
{
    {NetworkManager::Node_t::CONTROL_NODE,  "127.0.0.1"},
    {NetworkManager::Node_t::DEVICE_NODE_0, "127.0.0.2"},
    {NetworkManager::Node_t::DEVICE_NODE_1, "127.0.0.3"},
    {NetworkManager::Node_t::DEVICE_NODE_2, "127.0.0.4"},
};

/* Loopback channels to use for send/recv tests. */
static std::vector<NetworkManager::ChannelConfig_t> gLoopbackChannels =
{
    {NetworkManager::Node_t::CONTROL_NODE, 
     NetworkManager::Node_t::DEVICE_NODE_0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    {NetworkManager::Node_t::CONTROL_NODE, 
     NetworkManager::Node_t::DEVICE_NODE_1, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
    {NetworkManager::Node_t::CONTROL_NODE, 
     NetworkManager::Node_t::DEVICE_NODE_2, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 2)},
};

/* Loopback config for Control Node to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigCtrl =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::CONTROL_NODE,
    DV_ELEM_TEST0,
    DV_ELEM_TEST1,
};

/* Loopback config for Device Node 0 to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigDev0 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_0,
    DV_ELEM_TEST2,
    DV_ELEM_TEST3,
};

/* Loopback config for Device Node 1 to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigDev1 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_1,
    DV_ELEM_TEST4,
    DV_ELEM_TEST5,
};

/* Loopback config for Device Node 2 to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigDev2 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_2,
    DV_ELEM_TEST6,
    DV_ELEM_TEST7,
};

/******************************* SERVER TESTS *********************************/

TEST_GROUP (ClockSyncServerTest)
{
    /**
     * Stop ntpd if running.
     */
    void setup ()
    {
        system (NTPD_STOP_CMD);
    }

    /**
     * Stop ntpd if running.
     */
    void teardown ()
    {
        system (NTPD_STOP_CMD);
    }

};

/* Test null NM. */
TEST (ClockSyncServerTest, NullNm)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::shared_ptr<NetworkManager> pNm = nullptr;
    CHECK_ERROR (ClockSync::syncServer (pNm, gClients),
                 E_NETWORK_MANAGER_NULL);

    CHECK_DV (0, 0, 0, 0, 0, 0, 0, 0);
}

/* Test no clients. */
TEST (ClockSyncServerTest, NoClients)
{
    INIT_NETWORK_MANAGERS;

    std::vector<NetworkManager::Node_t> clients = {};

    CHECK_ERROR (ClockSync::syncServer (pNmCtrl, clients),
                 E_NO_CLIENTS);

    CHECK_DV (0, 0, 0, 0, 0, 0, 0, 0);
}

/* Test NM fail on tx by using a node the NM is not configured for. */
TEST (ClockSyncServerTest, NmTxFail)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unordered_map<NetworkManager::Node_t, 
                       NetworkManager::IP_t, 
                       EnumClassHash> nodes =
    {
        {NetworkManager::Node_t::CONTROL_NODE,  "127.0.0.1"},
        {NetworkManager::Node_t::DEVICE_NODE_0, "127.0.0.2"},
    };
    std::vector<NetworkManager::ChannelConfig_t> channels =
    {
        {NetworkManager::Node_t::CONTROL_NODE, 
         NetworkManager::Node_t::DEVICE_NODE_0, 
         static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    };
    NetworkManager::Config_t config =
    {
        nodes,
        channels,
        NetworkManager::Node_t::CONTROL_NODE,
        DV_ELEM_TEST0,
        DV_ELEM_TEST1,
    };
    std::shared_ptr<NetworkManager> pNm;
    CHECK_SUCCESS (NetworkManager::createNew (config, pDv, pNm));

    std::vector<NetworkManager::Node_t> clients =
    {
        NetworkManager::Node_t::DEVICE_NODE_0,
        NetworkManager::Node_t::DEVICE_NODE_1,
    };

    CHECK_ERROR (ClockSync::syncServer (pNm, clients),
                 E_NETWORK_MANAGER_TX_FAIL);

    // Expect first tx to be successful.
    CHECK_DV (1, 0, 0, 0, 0, 0, 0, 0);
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
    NetworkManager* pNm;
};

/**
 * Thread that sends a sync success message.
 */
static void* threadFuncClientSendSuccess (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs = (struct ThreadFuncArgs *) rawArgs;
    NetworkManager* pNm = pArgs->pNm;

    // Rx server ready cmd from CN.
    std::vector<uint8_t> rxBuf (1);
    std::vector<uint8_t> expectedRxBuf = {ClockSync::Msg_t::SERVER_READY};
    ret = pNm->recv (NetworkManager::Node_t::CONTROL_NODE, rxBuf);
    if (ret != E_SUCCESS)
    {
        return (void *) ret;
    }
    if (rxBuf != expectedRxBuf)
    {
        return (void *) E_FAILED_TO_TX_MSG;
    }

    // Tx success msg.
    std::vector<uint8_t> txBuf = {ClockSync::Msg_t::CLIENT_SYNC_SUCCESS};
    ret = pNm->send (NetworkManager::Node_t::CONTROL_NODE, txBuf);
    return (void *) ret;
}

/**
 * Thread that sends a sync fail message.
 */
static void* threadFuncClientSendFail (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs = (struct ThreadFuncArgs *) rawArgs;
    NetworkManager* pNm = pArgs->pNm;

    // Rx server ready cmd from CN.
    std::vector<uint8_t> rxBuf (1);
    std::vector<uint8_t> expectedRxBuf = {ClockSync::Msg_t::SERVER_READY};
    ret = pNm->recv (NetworkManager::Node_t::CONTROL_NODE, rxBuf);
    if (ret != E_SUCCESS)
    {
        return (void *) ret;
    }
    if (rxBuf != expectedRxBuf)
    {
        return (void *) E_FAILED_TO_TX_MSG;
    }

    // Tx fail msg.
    std::vector<uint8_t> sendBuf = {ClockSync::Msg_t::CLIENT_SYNC_FAIL};
    ret = pNm->send (NetworkManager::Node_t::CONTROL_NODE, sendBuf);
    return (void *) ret;
}

/* Test all msgs rx'd and one is a fail. */
TEST (ClockSyncServerTest, OneClientFailed)
{
    INIT_NETWORK_MANAGERS;
    INIT_THREAD_MANAGER;

    pthread_t clientThread0;
    pthread_t clientThread1;
    pthread_t clientThread2;
    CREATE_CLIENT_THREAD (clientThread0, pNmDev0, threadFuncClientSendSuccess);
    CREATE_CLIENT_THREAD (clientThread1, pNmDev1, threadFuncClientSendSuccess);
    CREATE_CLIENT_THREAD (clientThread2, pNmDev2, threadFuncClientSendFail);

    CHECK_ERROR (ClockSync::syncServer (pNmCtrl, gClients), 
                 E_CLIENT_FAILED_TO_SYNC);

    WAIT_FOR_THREAD (clientThread0, pTm);
    WAIT_FOR_THREAD (clientThread1, pTm);
    WAIT_FOR_THREAD (clientThread2, pTm);

    // Expect all messages to have been tx'd/rx'd.
    CHECK_DV (3, 3, 1, 1, 1, 1, 1, 1);
}

/* Test all msgs rx'd and all are success. */
TEST (ClockSyncServerTest, AllClientsSyncd)
{
    INIT_NETWORK_MANAGERS;
    INIT_THREAD_MANAGER;

    pthread_t clientThread0;
    pthread_t clientThread1;
    pthread_t clientThread2;
    CREATE_CLIENT_THREAD (clientThread0, pNmDev0, threadFuncClientSendSuccess);
    CREATE_CLIENT_THREAD (clientThread1, pNmDev1, threadFuncClientSendSuccess);
    CREATE_CLIENT_THREAD (clientThread2, pNmDev2, threadFuncClientSendSuccess);

    CHECK_SUCCESS (ClockSync::syncServer (pNmCtrl, gClients));

    WAIT_FOR_THREAD (clientThread0, pTm);
    WAIT_FOR_THREAD (clientThread1, pTm);
    WAIT_FOR_THREAD (clientThread2, pTm);

    // Expect all messages to have been tx'd/rx'd.
    CHECK_DV (3, 3, 1, 1, 1, 1, 1, 1);
}

/******************************* CLIENT TESTS *********************************/

/* NOTE: Due to the nature of our unit tests (run on 1 sbRIO), it is not 
   possible to sync a client to a server. Successful synchronization must be 
   tested in an integrated environment with > 1 sbRIO. */
TEST_GROUP (ClockSyncClientTest)
{
};

/* Test null NM. */
TEST (ClockSyncClientTest, NullNm)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::shared_ptr<NetworkManager> pNm = nullptr;
    CHECK_ERROR (ClockSync::syncClient (pNm, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_NETWORK_MANAGER_NULL);

    // Expect no msgs to have gone through.
    CHECK_DV (0, 0, 0, 0, 0, 0, 0, 0);
}

/* Test NM fail on rx by using a node the NM is not configured for. */
TEST (ClockSyncClientTest, NmRxFail)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unordered_map<NetworkManager::Node_t, 
                       NetworkManager::IP_t, 
                       EnumClassHash> nodes =
    {
        {NetworkManager::Node_t::DEVICE_NODE_0, "127.0.0.2"},
        {NetworkManager::Node_t::DEVICE_NODE_1, "127.0.0.3"},
    };
    std::vector<NetworkManager::ChannelConfig_t> channels =
    {
        {NetworkManager::Node_t::DEVICE_NODE_0, 
         NetworkManager::Node_t::DEVICE_NODE_1, 
         static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    };
    NetworkManager::Config_t config =
    {
        nodes,
        channels,
        NetworkManager::Node_t::DEVICE_NODE_0,
        DV_ELEM_TEST2,
        DV_ELEM_TEST3,
    };
    std::shared_ptr<NetworkManager> pNm;
    CHECK_SUCCESS (NetworkManager::createNew (config, pDv, pNm));

    CHECK_ERROR (ClockSync::syncClient (pNm, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_NETWORK_MANAGER_RX_FAIL);

    // Expect no msgs to have gone through.
    CHECK_DV (0, 0, 0, 0, 0, 0, 0, 0);
}

/* Test invalid server message. */
TEST (ClockSyncClientTest, InvalidServerMsg)
{
    INIT_NETWORK_MANAGERS;

    // Send server ready msg to client.
    std::vector<uint8_t> readyMsg = {ClockSync::Msg_t::LAST};
    CHECK_SUCCESS (pNmCtrl->send (NetworkManager::Node_t::DEVICE_NODE_0, 
                                  readyMsg));

    // Sync client.
    CHECK_ERROR (ClockSync::syncClient (pNmDev0, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_INVALID_SERVER_MSG);

    // Expect 1 msg to have been tx'd/rx'd.
    CHECK_DV (1, 0, 0, 1, 0, 0, 0, 0);
}

/* Test client sync successful until ntpdate call (see note above). */
TEST (ClockSyncClientTest, FailToSync)
{
    INIT_NETWORK_MANAGERS;

    // Send server ready msg to client.
    std::vector<uint8_t> readyMsg = {ClockSync::Msg_t::SERVER_READY};
    CHECK_SUCCESS (pNmCtrl->send (NetworkManager::Node_t::DEVICE_NODE_0, 
                   readyMsg));

    // Sync client.
    CHECK_ERROR (ClockSync::syncClient (pNmDev0, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_CLIENT_FAILED_TO_SYNC);

    // Expect server ready msg to have been tx'd/rx'd, and for client fail msg
    // to have been tx'd.
    CHECK_DV (1, 0, 1, 1, 0, 0, 0, 0);
}

