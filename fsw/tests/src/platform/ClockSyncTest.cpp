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
    std::shared_ptr<NetworkManager> pNmCtrl;                                   \
    std::shared_ptr<NetworkManager> pNmDev0;                                   \
    std::shared_ptr<NetworkManager> pNmDev1;                                   \
    std::shared_ptr<NetworkManager> pNmDev2;                                   \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigCtrl, pNmCtrl));  \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev0, pNmDev0));  \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev1, pNmDev1));  \
    CHECK_SUCCESS (NetworkManager::createNew (gLoopbackConfigDev2, pNmDev2));

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

/********************************* CONFIGS ************************************/

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
};

/* Loopback config for Device Node 0 to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigDev0 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_0,
};

/* Loopback config for Device Node 1 to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigDev1 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_1,
};

/* Loopback config for Device Node 2 to use for send/recv tests. */
static NetworkManager::Config_t gLoopbackConfigDev2 =
{
    gLoopbackNodes,
    gLoopbackChannels,
    NetworkManager::Node_t::DEVICE_NODE_2,
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
    std::shared_ptr<NetworkManager> pNm = nullptr;
    CHECK_ERROR (ClockSync::syncServer (pNm, gClients),
                 E_NETWORK_MANAGER_NULL);
}

/* Test no clients. */
TEST (ClockSyncServerTest, NoClients)
{
    INIT_NETWORK_MANAGERS;

    std::vector<NetworkManager::Node_t> clients = {};

    CHECK_ERROR (ClockSync::syncServer (pNmCtrl, clients),
                 E_NO_CLIENTS);
}

/* Test NM fail on tx by using a node the NM is not configured for. */
TEST (ClockSyncServerTest, NmTxFail)
{
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
    };
    std::shared_ptr<NetworkManager> pNm;
    CHECK_SUCCESS (NetworkManager::createNew (config, pNm));

    std::vector<NetworkManager::Node_t> clients =
    {
        NetworkManager::Node_t::DEVICE_NODE_0,
        NetworkManager::Node_t::DEVICE_NODE_1,
    };

    CHECK_ERROR (ClockSync::syncServer (pNm, clients),
                 E_NETWORK_MANAGER_TX_FAIL);
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
    std::shared_ptr<NetworkManager> pNm = nullptr;
    CHECK_ERROR (ClockSync::syncClient (pNm, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_NETWORK_MANAGER_NULL);
}

/* Test NM fail on rx by using a node the NM is not configured for. */
TEST (ClockSyncClientTest, NmRxFail)
{
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
    };
    std::shared_ptr<NetworkManager> pNm;
    CHECK_SUCCESS (NetworkManager::createNew (config, pNm));

    CHECK_ERROR (ClockSync::syncClient (pNm, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_NETWORK_MANAGER_RX_FAIL);
}

/* Test invalid server message. */
TEST (ClockSyncClientTest, InvalidServerMsg)
{
    INIT_NETWORK_MANAGERS;

    // Send server ready msg to client.
    std::vector<uint8_t> readyMsg = {ClockSync::Msg_t::LAST};
    CHECK_SUCCESS (pNmCtrl->send (NetworkManager::Node_t::DEVICE_NODE_0, readyMsg));

    // Sync client.
    CHECK_ERROR (ClockSync::syncClient (pNmDev0, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_INVALID_SERVER_MSG);
}

/* Test client sync successful until ntpdate call (see note above). */
TEST (ClockSyncClientTest, FailToSync)
{
    INIT_NETWORK_MANAGERS;

    // Send server ready msg to client.
    std::vector<uint8_t> readyMsg = {ClockSync::Msg_t::SERVER_READY};
    CHECK_SUCCESS (pNmCtrl->send (NetworkManager::Node_t::DEVICE_NODE_0, readyMsg));

    // Sync client.
    CHECK_ERROR (ClockSync::syncClient (pNmDev0, 
                                        NetworkManager::Node_t::CONTROL_NODE, 
                                        "127.0.0.1"),
                 E_CLIENT_FAILED_TO_SYNC);
}

