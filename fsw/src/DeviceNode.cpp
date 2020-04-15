#include <set>

#include "DeviceNode.hpp"

/******************************** CONSTANTS ***********************************/

/**
 * Struct to represent Device Node specific Data Vector Regions and Elements.
 */
typedef struct DvInfo
{
    DataVectorRegion_t recvRegion;
    DataVectorRegion_t sendRegion;
    DataVectorElement_t loopElem;
    DataVectorElement_t errorElem;
} DvInfo_t;

/**
 * Map from Device Node x to Data Vector info.
 */
static const std::unordered_map<Node_t, DvInfo_t, EnumClassHash> 
    NODE_TO_DV_INFO =
{
    {NODE_DEVICE0, 
    {
        DV_REG_CN_TO_DN0, 
        DV_REG_DN0_TO_CN, 
        DV_ELEM_DN0_LOOP_COUNT, 
        DV_ELEM_DN0_ERROR_COUNT
    }},
    {NODE_DEVICE1,
    {
        DV_REG_CN_TO_DN1, 
        DV_REG_DN1_TO_CN, 
        DV_ELEM_DN1_LOOP_COUNT, 
        DV_ELEM_DN1_ERROR_COUNT
    }},
    {NODE_DEVICE2,
    {
        DV_REG_CN_TO_DN2, 
        DV_REG_DN2_TO_CN, 
        DV_ELEM_DN2_LOOP_COUNT, 
        DV_ELEM_DN2_ERROR_COUNT
    }},
};

/********************************* GLOBALS ************************************/

/**
 * Device Node x. Used as key into NODE_TO_DV_INFO.
 */
static Node_t gMe = NODE_LAST;

/**
 * Pointer to Time Module.
 */
static Time* gPTime = nullptr;

/**
 * Pointer to Data Vector.
 */
static std::shared_ptr<DataVector> gPDv = nullptr;

/**
 * Pointer to Network Manager.
 */
static std::shared_ptr<NetworkManager> gPNm = nullptr;

/**
 * Vector of pointers to Controllers.
 */
static std::vector<std::unique_ptr<Controller>> gPCtrls;

/**
 * Vector of pointers to sensor Devices.
 */
static std::vector<std::unique_ptr<Device>> gPSensorDevs;

/**
 * Vector of pointers to actuator Devices.
 */
static std::vector<std::unique_ptr<Device>> gPActuatorDevs;

/**
 * FPGA session.
 */
static NiFpga_Session gFpgaSession;

/**
 * Statically allocated buffer for receiving data from Control Node.
 */
static std::vector<uint8_t> gRecvBuf;

/**
 * Statically allocated buffer for sending data to Control Node.
 */
static std::vector<uint8_t> gSendBuf;

/***************************** PRIVATE FUNCIONS *******************************/

/**
 * Helper to verify Network Manager config matches required Platform v1 
 * topology.
 *
 * @param  kNmConfig        NM config to verify.
 *
 * @ret    E_SUCCESS        Config valid.
 *         E_INVALD_CONFIG  Config invalid.
 */
static Error_t verifyNmConfig (NetworkManager::Config_t& kNmConfig)
{
    // Verify contains me and Control Node.
    std::unordered_map<Node_t, NetworkManager::IP_t, EnumClassHash> nodes = 
        kNmConfig.nodeToIp;
    if (nodes.find (NODE_CONTROL) == nodes.end () ||
        nodes.find (kNmConfig.me) == nodes.end ())
    {
        return E_INVALID_CONFIG;
    }

    // Verify contains Control Node <--> Device Node x channel.
    std::set<std::set<Node_t>> expectedChannelsSet =
    {
        {NODE_CONTROL, kNmConfig.me},
    };

    // Remove channels that exist in config from expectedChannelsSet.
    for (NetworkManager::ChannelConfig_t channel : kNmConfig.channels)
    {
        expectedChannelsSet.erase ({channel.node1, channel.node2});
    }

    // Verify set is now empty.
    if (expectedChannelsSet.empty () != true)
    {
        return E_INVALID_CONFIG;
    }

    return E_SUCCESS;
}

/**
 * Helper to verify Data Vector config contains required regions and elements.
 *
 * @param  kDvConfig      DV config to verify.
 *
 * @ret  E_SUCCESS        Config valid.
 *       E_INVALD_CONFIG  Config invalid.
 */
static Error_t verifyDvConfig (DataVector::Config_t& kDvConfig)
{
    // Initialize sets of required regions and elements.
    std::set<DataVectorRegion_t> requiredRegionSet =
    {
        NODE_TO_DV_INFO.at (gMe).recvRegion,
        NODE_TO_DV_INFO.at (gMe).sendRegion,
    };
    std::set<DataVectorElement_t> requiredElementSet =
    {
        NODE_TO_DV_INFO.at (gMe).loopElem,
        NODE_TO_DV_INFO.at (gMe).errorElem,
    };

    // Loop over regions and elements, removing them from the required sets.
    for (DataVector::RegionConfig_t regionConfig : kDvConfig)
    {
        requiredRegionSet.erase (regionConfig.region);
        for (DataVector::ElementConfig_t elemConfig : regionConfig.elems)
        {
            requiredElementSet.erase (elemConfig.elem);
        }
    }

    // Verify required sets are now empty.
    if (requiredRegionSet.empty ()  != true || 
        requiredElementSet.empty () != true)
    {
        return E_INVALID_CONFIG;
    }

    return E_SUCCESS;
}

/**
 * Helper to initialize static buffers for rx'ing/tx'ing data over the network.
 *
 * @ret    E_SUCCESS           Successfully initialized buffers.
 *         E_DATA_VECTOR_READ  Failed to read size data from Data Vector.
 *         E_INVALID_CONFIG    Unexpected buffer.
 */
static Error_t initializeBuffers ()
{
    // Get size of buffers.
    uint32_t recvBufSize = 0;
    uint32_t sendBufSize = 0;
    if (gPDv->getRegionSizeBytes (NODE_TO_DV_INFO.at (gMe).recvRegion,
                                  recvBufSize) != E_SUCCESS ||
        gPDv->getRegionSizeBytes (NODE_TO_DV_INFO.at (gMe).sendRegion,
                                  sendBufSize) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Resize buffers.
    gRecvBuf.resize (recvBufSize);
    gSendBuf.resize (sendBufSize);

    return E_SUCCESS;
}

/**
 * Helper to recv Data Vector data from Control Node and send the relevant data
 * back. Optimized for faster response time to Control Node.
 *
 * NOTE: Blocks until message received.
 *
 * @ret  E_SUCCESS                  Successfully received data.
 *       E_DATA_VECTOR_READ         Failed to copy data from Data Vector.
 *       E_NETWORK_MANAGER_RX_FAIL  Failed to recv data.
 *       E_DATA_VECTOR_WRITE        Failed to write data to Data Vector.
 *       E_NETWORK_MANAGER_TX_FAIL  Failed to send data to nodes.
 */
static Error_t recvAndSendDataVectorData ()
{
    // 1) Copy tx Data Vector data to buffer. Do this first so can respond as
    //    soon as data is received.
    if (gPDv->readRegion (NODE_TO_DV_INFO.at (gMe).sendRegion, gSendBuf) 
            != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // 2) Receive data from Control Node.
    if (gPNm->recvBlock (NODE_CONTROL, gRecvBuf) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_RX_FAIL;
    }

    // 3) Send data to Control Node.
    if (gPNm->send (NODE_CONTROL, gSendBuf) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_TX_FAIL;
    }

    // 4) There is a known issue with the sbRIO Ethernet hardware that can
    //    result in a message getting "stuck" in the RX FIFO queue. The message
    //    gets unstuck when the next Ethernet frame comes in. This should not
    //    happen due to the noop message sent by the Network Manager after each
    //    send, but in case it does, call recv one more time without blocking to 
    //    make sure the Device Node is not operating on the previous loop's 
    //    data.
    bool _msgRxd = false;
    if (gPNm->recvNoBlock (NODE_CONTROL, gRecvBuf, _msgRxd) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_RX_FAIL;
    }

    // 5) Copy data from buffer to Data Vector.
    if (gPDv->writeRegion (NODE_TO_DV_INFO.at (gMe).recvRegion, gRecvBuf) 
            != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

/**
 * Device Node logic that runs in a loop synchronized with Control Node's
 * periodic loop. Runs logic in the following order:
 *
 *   1) Receive Data Vector region from Control Node. Block until receive.
 *   2) Send Data Vector region to Control Node.
 *   3) Run Sensor Devices.
 *   4) Run Controllers.
 *   5) Run Actuator Devices.
 *
 * This function never returns. If Errors::incrementOnError fails, fails 
 * silently.
 *
 * @param   _kArgs                          Unused.
 *
 */
static void* loop (void* _kArgs)
{
    DataVectorElement_t errorElem = NODE_TO_DV_INFO.at (gMe).errorElem;
    DataVectorElement_t loopElem  = NODE_TO_DV_INFO.at (gMe).loopElem;
    while (1)
    {
        // 1) Receive rx Region from Control Node send tx Region. 
        Errors::incrementOnError (recvAndSendDataVectorData (), gPDv, 
                                  errorElem);

        // 2) Run the Sensor Devices. Run this before the Controllers so that
        //    they have the most up-to-date data.
        for (std::unique_ptr<Device>& pSensorDev : gPSensorDevs)
        {
            Errors::incrementOnError (pSensorDev->run (), gPDv, errorElem);
        }
        
        // 3) Run the Controllers.
        for (std::unique_ptr<Controller>& pCtrl : gPCtrls)
        {
            Errors::incrementOnError (pCtrl->run (), gPDv, errorElem);
        }

        // 4) Run the Actuator Devices. Run this after the Controllers so that
        //    the system can react quickly.
        for (std::unique_ptr<Device>& pActuatorDev : gPActuatorDevs)
        {
            Errors::incrementOnError (pActuatorDev->run (), gPDv, errorElem);
        }

        // 5) Increment loop counter.
        Errors::incrementOnError (gPDv->increment (loopElem), gPDv, errorElem);
    }
}

/***************************** PUBLIC FUNCIONS ********************************/

void DeviceNode::entry (NetworkManager::Config_t  kNmConfig, 
                        DataVector::Config_t      kDvConfig,
                        fInitializeCtrlsAndDevs_t kFInitCtrlsAndDevs,
                        bool                      kSkipClockSync)
{
    // 0) Set "me" global
    gMe = kNmConfig.me;
    if (gMe != NODE_DEVICE0 && gMe != NODE_DEVICE1 && gMe != NODE_DEVICE2)
    {
        Errors::exitOnError (E_INVALID_NODE, "Me node must be a Device Node.");
    }

    // 1) Verify Network Manager config matches required topology.
    Errors::exitOnError (
                verifyNmConfig (kNmConfig), 
                "Network Manager config does not match required topology.");

    // 2) Verify Data Vector config contains required regions and elements.
    Errors::exitOnError (
        verifyDvConfig (kDvConfig),
        "Data Vector config does not contain required regions or elements.");
    

    // 3) Init Thread Manager. Do this early on so that the kernel scheduling 
    //    environment is set up immediately.
    ThreadManager* pTm = nullptr;
    Errors::exitOnError (ThreadManager::getInstance (pTm),
                         "Thread Manager failed to initialize.");

    // 4) Init Data Vector. This is required for Network Manager, Controller, 
    // and Device initialization.
    Errors::exitOnError (DataVector::createNew (kDvConfig, gPDv),
                         "Data Vector failed to initialize.");

    // 5) Init buffers that will be used in loop to send and receive data over
    //    the network.
    Errors::exitOnError (initializeBuffers (), "Failed to initialize buffers.");

    // 6) Init Network Manager. This is required for clock synchronization.
    Errors::exitOnError (NetworkManager::createNew (kNmConfig, gPDv, gPNm), 
                         "Network Manager failed to initialize.");

    // 7) Synchronize to the Control Node's clock. This must be done before the 
    //    Time Module is initialized.
    if (kSkipClockSync == false)
    {
        NetworkManager::IP_t serverIp = kNmConfig.nodeToIp[NODE_CONTROL];
        Errors::exitOnError (ClockSync::syncClient (gPNm, NODE_CONTROL, 
                                                    serverIp),
                             "Clock synchronization failed.");
    }

    // 8) Init Time Module.
    Errors::exitOnError (Time::getInstance (gPTime), 
                         "Time Module failed to initialize.");
    
    // 9) Init FPGA session. Required to init Devices.
    NiFpga_Status status = NiFpga_Status_Success;
    Errors::exitOnError (FPGASession::getSession (gFpgaSession, status), 
                         "FPGA session failed to initialize.");
    if (status != NiFpga_Status_Success)
    {
        Errors::exitOnError (E_FPGA_INIT, "FPGA status error.");
    }

    // 10) Init Controllers and Devices.
    Errors::exitOnError (kFInitCtrlsAndDevs (gPDv, gFpgaSession, gPCtrls, 
                                             gPSensorDevs, gPActuatorDevs),
                         "Controllers or Devices failed to initialize.");

    // 11) Create thread to run loop function.
    pthread_t loopThread;
    ThreadManager::ThreadFunc_t fLoop = (ThreadManager::ThreadFunc_t) loop;
    Errors::exitOnError (pTm->createThread (
                                      loopThread, fLoop, nullptr, 0,
                                      ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                      ThreadManager::Affinity_t::CORE_0),
                         "Failed to start thread.");

    // 12) Wait for thread and check return status. On success, this will cause
    //     the main thread to block and should never return.
    Error_t loopThreadRet = E_SUCCESS;
    Errors::exitOnError (pTm->waitForThread (loopThread, loopThreadRet),
                         "Failed to wait on loop thread.");
    Errors::exitOnError (loopThreadRet, "Loop thread returned error.");

    // 13) If the function gets this far, the loop thread return an unexpected
    //     success status. Exit the process.
    exit (EXIT_FAILURE);
}

