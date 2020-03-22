#include <set>

#include "ControlNode.hpp"

/******************************** CONSTANTS ***********************************/

/**
 * Period of loop function.
 */
static const uint32_t LOOP_PERIOD_MS = 10;

/**
 * Microseconds to wait for Device and Ground node messages at top of loop.
 */
static const uint32_t DATA_RX_TIMEOUT_US = 2 * Time::US_IN_MS;

/**
 * Nodes to receive messages from at the top of loop.
 */
static const std::vector<Node_t> NODES_TO_RECV_FROM =
{
    NODE_DEVICE0,
    NODE_DEVICE1,
    NODE_DEVICE2,
    NODE_GROUND,
};

/********************************* GLOBALS ************************************/

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
 * Pointer to Command Handler.
 */
static std::unique_ptr<CommandHandler> gPCh = nullptr;

/**
 * Pointer to State Machine.
 */
std::unique_ptr<StateMachine> gPSm = nullptr;

/**
 * Vector of pointers to Controllers.
 */
static std::vector<std::unique_ptr<Controller>> gPCtrls;

/**
 * Statically allocated buffers for receiving data over the network.
 */
static std::vector<std::vector<uint8_t>> gRecvBufs (NODES_TO_RECV_FROM.size ());

/**
 * Statically allocated buffers for sending data over the network.
 */
static std::vector<uint8_t> gCnToDn0Buf;
static std::vector<uint8_t> gCnToDn1Buf;
static std::vector<uint8_t> gCnToDn2Buf;
static std::vector<uint8_t> gCnToGndBuf;

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
    // Verify nodes.
    std::unordered_map<Node_t, NetworkManager::IP_t, EnumClassHash> nodes = 
        kNmConfig.nodeToIp;
    if (nodes.find (NODE_CONTROL) == nodes.end () ||
        nodes.find (NODE_DEVICE0) == nodes.end () ||
        nodes.find (NODE_DEVICE1) == nodes.end () ||
        nodes.find (NODE_DEVICE2) == nodes.end () ||
        nodes.find (NODE_GROUND)  == nodes.end ())
    {
        return E_INVALID_CONFIG;
    }

    // Verify channels
    std::set<std::set<Node_t>> expectedChannelsSet =
    {
        {NODE_CONTROL, NODE_DEVICE0},
        {NODE_CONTROL, NODE_DEVICE1},
        {NODE_CONTROL, NODE_DEVICE2},
        {NODE_CONTROL, NODE_GROUND}
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
 * @param  kDvConfig        DV config to verify.
 *
 * @ret    E_SUCCESS        Config valid.
 *         E_INVALD_CONFIG  Config invalid.
 */
static Error_t verifyDvConfig (DataVector::Config_t& kDvConfig)
{
    // Initialize sets of required regions and elements.
    std::set<DataVectorRegion_t> requiredRegionSet =
    {
        DV_REG_CN,
        DV_REG_CN_TO_DN0,
        DV_REG_CN_TO_DN1,
        DV_REG_CN_TO_DN2,
        DV_REG_DN0_TO_CN,
        DV_REG_DN1_TO_CN,
        DV_REG_DN2_TO_CN,
        DV_REG_GROUND_TO_CN,
    };
    std::set<DataVectorElement_t> requiredElementSet =
    {
        DV_ELEM_STATE,
        DV_ELEM_CN_LOOP_COUNT,
        DV_ELEM_CN_ERROR_COUNT,
        DV_ELEM_DN0_RX_MISS_COUNT,
        DV_ELEM_DN1_RX_MISS_COUNT,
        DV_ELEM_DN2_RX_MISS_COUNT,
        DV_ELEM_CN_TIME_NS,
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
    if (requiredRegionSet.empty () != true || 
        requiredElementSet.empty () != true)
    {
        return E_INVALID_CONFIG;
    }

    return E_SUCCESS;
}

/**
 * Helper to initialize static buffers for rx'ing/tx'ing data over the network.
 *
 * @ret  E_SUCCESS           Successfully initialized buffers.
 *       E_DATA_VECTOR_READ  Failed to read size data from Data Vector.
 *       E_INVALID_CONFIG    Unexpected buffer.
 */
static Error_t initializeBuffers ()
{
    // Get size of buffers.
    uint32_t cnToDn0BufSize = 0;
    uint32_t cnToDn1BufSize = 0;
    uint32_t cnToDn2BufSize = 0;
    uint32_t cnToGndBufSize = 0;
    uint32_t dn0ToCnBufSize = 0;
    uint32_t dn1ToCnBufSize = 0;
    uint32_t dn2ToCnBufSize = 0;
    uint32_t gndToCnBufSize = 0;
    if (gPDv->getRegionSizeBytes (DV_REG_CN_TO_DN0, cnToDn0BufSize) 
            != E_SUCCESS ||
        gPDv->getRegionSizeBytes (DV_REG_CN_TO_DN1, cnToDn1BufSize) 
            != E_SUCCESS ||
        gPDv->getRegionSizeBytes (DV_REG_CN_TO_DN2, cnToDn2BufSize) 
            != E_SUCCESS ||
        gPDv->getRegionSizeBytes (DV_REG_DN0_TO_CN, dn0ToCnBufSize) 
            != E_SUCCESS ||
        gPDv->getRegionSizeBytes (DV_REG_DN1_TO_CN, dn1ToCnBufSize) 
            != E_SUCCESS ||
        gPDv->getRegionSizeBytes (DV_REG_DN2_TO_CN, dn2ToCnBufSize) 
            != E_SUCCESS ||
        gPDv->getRegionSizeBytes (DV_REG_GROUND_TO_CN, gndToCnBufSize) 
            != E_SUCCESS ||
        gPDv->getDataVectorSizeBytes (cnToGndBufSize) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Resize send buffers.
    gCnToDn0Buf.resize (cnToDn0BufSize);
    gCnToDn1Buf.resize (cnToDn1BufSize);
    gCnToDn2Buf.resize (cnToDn2BufSize);
    gCnToGndBuf.resize (cnToGndBufSize);

    // Resize receive buffers.
    for (uint8_t i = 0; i < NODES_TO_RECV_FROM.size (); i++)
    {
        switch (NODES_TO_RECV_FROM[i])
        {
            case NODE_DEVICE0:
                gRecvBufs[i].resize (dn0ToCnBufSize);
                break;
            case NODE_DEVICE1:
                gRecvBufs[i].resize (dn1ToCnBufSize);
                break;
            case NODE_DEVICE2:
                gRecvBufs[i].resize (dn2ToCnBufSize);
                break;
            case NODE_GROUND:
                gRecvBufs[i].resize (gndToCnBufSize);
                break;
            default:
                // This should never happen.
                return E_INVALID_CONFIG;
        }
    }

    return E_SUCCESS;
}

/**
 * Helper to copy relevant Data Vector data and send it to respective nodes.
 *
 * @ret  E_SUCCESS                  Successfully sent data.
 *       E_DATA_VECTOR_READ         Failed to copy data from Data Vector.
 *       E_NETWORK_MANAGER_TX_FAIL  Failed to send data to nodes.
 */
static Error_t sendDataVectorData ()
{
    // Copy Data Vector data to buffers.
    if (gPDv->readRegion (DV_REG_CN_TO_DN0, gCnToDn0Buf) != E_SUCCESS ||
        gPDv->readRegion (DV_REG_CN_TO_DN1, gCnToDn1Buf) != E_SUCCESS ||
        gPDv->readRegion (DV_REG_CN_TO_DN2, gCnToDn2Buf) != E_SUCCESS ||
        gPDv->readDataVector (gCnToGndBuf)               != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Send data to respective nodes.
    if (gPNm->send (NODE_DEVICE0, gCnToDn0Buf) != E_SUCCESS ||
        gPNm->send (NODE_DEVICE1, gCnToDn1Buf) != E_SUCCESS ||
        gPNm->send (NODE_DEVICE2, gCnToDn2Buf) != E_SUCCESS ||
        gPNm->send (NODE_GROUND, gCnToGndBuf)  != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_TX_FAIL;
    }

    return E_SUCCESS;
}

/**
 * Helper to recv Data Vector data from nodes and save it to the Control Node's
 * Data Vector. 
 *
 * @ret  E_SUCCESS                  Successfully received data.
 *       E_NETWORK_MANAGER_RX_FAIL  Failed to recv data from nodes.
 *       E_DATA_VECTOR_WRITE        Failed to write data to Data Vector.
 */
static Error_t recvDataVectorData ()
{
    // Msg received params. Initialize to false.
    std::vector<bool> msgsReceived (NODES_TO_RECV_FROM.size (), false);

    // Receive data from nodes.
    if (gPNm->recvMult (DATA_RX_TIMEOUT_US, NODES_TO_RECV_FROM, gRecvBufs, 
                        msgsReceived) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_RX_FAIL;
    }

    // Copy data from buffers to Data Vector or handle a missed message.
    for (uint8_t i = 0; i < NODES_TO_RECV_FROM.size (); i++)
    {
        Error_t ret = E_SUCCESS;
        switch (NODES_TO_RECV_FROM[i])
        {
            case NODE_DEVICE0:
                ret = msgsReceived[i] == true
                    ? gPDv->writeRegion (DV_REG_DN0_TO_CN, gRecvBufs[i])
                    : gPDv->increment (DV_ELEM_DN0_RX_MISS_COUNT);
                break;
            case NODE_DEVICE1:
                ret = msgsReceived[i] == true
                    ? gPDv->writeRegion (DV_REG_DN1_TO_CN, gRecvBufs[i])
                    : gPDv->increment (DV_ELEM_DN1_RX_MISS_COUNT);
                break;
            case NODE_DEVICE2:
                ret = msgsReceived[i] == true
                    ? gPDv->writeRegion (DV_REG_DN2_TO_CN, gRecvBufs[i])
                    : gPDv->increment (DV_ELEM_DN2_RX_MISS_COUNT);
                break;
            case NODE_GROUND:
                // Don't increment an error if don't rx msg from GROUND as this
                // is expected unless a command has been sent.
                ret = gPDv->writeRegion (DV_REG_GROUND_TO_CN, gRecvBufs[i]);
                break;
            default:
                // This should never happen.
                return E_DATA_VECTOR_WRITE;
        }

        // Handle write error.
        if (ret != E_SUCCESS)
        {
            return E_DATA_VECTOR_WRITE;
        }
    }

    return E_SUCCESS;
}

/**
 * Control Node logic that runs in a periodic loop. Runs logic in the
 * following order:
 *
 *   1) Send Data Vector data to all other nodes on network.
 *   2) Receive Data Vector regions from all other nodes on the network.
 *   3) Run Command Handler to process commands from ground computer.
 *   4) Step State Machine.
 *   5) Run each Controller.
 *
 * On success, function never returns.
 *
 * @param   _kArgs                          Unused.
 *
 * @ret     E_FAILED_TO_CREATE_TIMERFD      Failed to create timer.
 *          E_FAILED_TO_ARM_TIMERFD         Failed to set and arm timer.
 *          E_FAILED_TO_GET_TIMER_FLAGS     Failed to get timer flags.
 *          E_FAILED_TO_SET_TIMER_FLAGS     Failed to set timer flags.
 *          E_FAILED_TO_READ_TIMERFD        Failed to read timer.
 *          E_MISSED_SCHEDULER_DEADLINE     Thread ended after period
 *                                          elapsed or started after timer
 *                                          triggered more than once.
 */
void* loop (void* _kArgs)
{
    // 1) Send copies of relevant Data Vector regions to Device and Ground 
    //    Nodes. This signal doubles as a loop synchronizer, as all Device
    //    Nodes begin their loop on receiving a message from the Control Node.
    Errors::incrementOnError (sendDataVectorData (), gPDv, 
                              DV_ELEM_CN_ERROR_COUNT);

    // 2) Receive relevant data from Device and Ground Nodes and save to Data
    //    Vector.
    Errors::incrementOnError (recvDataVectorData (), gPDv,
                              DV_ELEM_CN_ERROR_COUNT);

    // 3) Get the current time and store it in the Data Vector.
    Time::TimeNs_t currTimeNs;
    Errors::incrementOnError (gPTime->getTimeNs (currTimeNs), gPDv, 
                              DV_ELEM_CN_ERROR_COUNT);
    Errors::incrementOnError (gPDv->write (DV_ELEM_CN_TIME_NS, 
                                           (uint64_t) currTimeNs), 
                              gPDv, DV_ELEM_CN_ERROR_COUNT);
   
    // 4) Run Command Handler. This will process a command from ground if one
    //    was received. This must run before the State Machine, as some state
    //    transitions are dependent on a ground command.
    Errors::incrementOnError (gPCh->run (), gPDv, DV_ELEM_CN_ERROR_COUNT);
    uint8_t cmdReq = CMD_NONE;
    uint8_t cmd = CMD_NONE;
    gPDv->read (DV_ELEM_CMD, cmd);
    gPDv->read (DV_ELEM_CMD_REQ, cmdReq);
    
    // 5) Step the State Machine.
    Errors::incrementOnError (gPSm->step (currTimeNs), gPDv, 
                              DV_ELEM_CN_ERROR_COUNT);
    
    // 6) Run the controllers.
    for (std::unique_ptr<Controller>& pCtrl : gPCtrls)
    {
        Errors::incrementOnError (pCtrl->run (), gPDv, DV_ELEM_CN_ERROR_COUNT);
    }

    // 7) Increment loop counter.
    Errors::incrementOnError (gPDv->increment (DV_ELEM_CN_LOOP_COUNT), gPDv,
                              DV_ELEM_CN_ERROR_COUNT);

    return nullptr;
}

/***************************** PUBLIC FUNCIONS ********************************/

void ControlNode::entry (NetworkManager::Config_t kNmConfig, 
                         DataVector::Config_t     kDvConfig,
                         CommandHandler::Config_t kChConfig,
                         StateMachine::Config_t   kSmConfig,
                         fInitializeControllers_t kFInitControllers)
{
    // 0) Verify Network Manager config matches required topology.
    Errors::exitOnError (
                verifyNmConfig (kNmConfig), 
                "Network Manager config does not match required topology.");

    // 1) Verify Data Vector config contains required elements.
    Errors::exitOnError (
        verifyDvConfig (kDvConfig),
        "Data Vector config does not contain required regions or elements.");

    // 2) Init Thread Manager. Do this first so that the kernel scheduling 
    //    environment is set up immediately.
    ThreadManager* pTm = nullptr;
    Errors::exitOnError (ThreadManager::getInstance (&pTm),
                         "Thread Manager failed to initialize.");

    // 3) Init Data Vector. This is required for Network Manager, Command 
    //    Handler, Controller, and State Machine initialization.
    Errors::exitOnError (DataVector::createNew (kDvConfig, gPDv),
                         "Data Vector failed to initialize.");

    // 4) Init buffers that will be used in loop to send and receive data over
    //    the network.
    Errors::exitOnError (initializeBuffers (), "Failed to initialize buffers.");

    // 5) Init Network Manager. This is required for clock synchronization.
    Errors::exitOnError (NetworkManager::createNew (kNmConfig, gPDv, gPNm), 
                         "Network Manager failed to initialize.");

    // 6) Synchronize the flight computer clocks. Clients are all device nodes 
    //    in the network. This must be done before the Time Module is 
    //    initialized.
    std::vector<Node_t> clockSyncClients =
    {
        NODE_DEVICE0,
        NODE_DEVICE1,
        NODE_DEVICE2,
    };
    Errors::exitOnError (ClockSync::syncServer (gPNm, clockSyncClients),
                         "Clock synchronization failed.");

    // 7) Init Command Handler.
    Errors::exitOnError (CommandHandler::createNew (kChConfig, gPDv, gPCh),
                         "Command Handler failed to initialize.");

    // 8) Init Controllers.
    Errors::exitOnError (kFInitControllers (gPDv, gPCtrls),
                         "Controllers failed to initialize.");

    // 9) Init Time Module. This is required for State Machine initialization.
    Errors::exitOnError (Time::getInstance (gPTime), 
                         "Time Module failed to initialize.");

    // 10) Get current time and write it to the Data Vector.
    Time::TimeNs_t currTimeNs = 0;
    Errors::exitOnError (gPTime->getTimeNs (currTimeNs), 
                         "Failed to read current time.");
    Errors::exitOnError (gPDv->write (DV_ELEM_CN_TIME_NS, 
                                      (uint64_t) currTimeNs),
                         "Failed to write current time to Data Vector");

    // 11) Initialize the State Machine. Do this last so that the periodic loop 
    //     begins right after the State Machine is initialized, which starts 
    //     counting time in state.
    Errors::exitOnError (StateMachine::createNew (kSmConfig, gPDv, currTimeNs, 
                                                  DV_ELEM_STATE, gPSm),
                         "State Machine failed to initialize.");
            
    // 12) Create periodic thread to run loop function.
    pthread_t loopThread;
    ThreadManager::ThreadFunc_t *fLoop = (ThreadManager::ThreadFunc_t*) &loop;
    Errors::exitOnError (pTm->createPeriodicThread (
                                      loopThread, fLoop, nullptr, 0,
                                      ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                      ThreadManager::Affinity_t::CORE_0,
                                      LOOP_PERIOD_MS),
                         "Failed to start periodic thread.");

    // 13) Wait for thread and check return status. On success, this will cause
    //     the main thread to block and should never return.
    Error_t loopThreadRet = E_SUCCESS;
    Errors::exitOnError (pTm->waitForThread (loopThread, loopThreadRet),
                         "Failed to wait on loop thread.");
    Errors::exitOnError (loopThreadRet, "Loop thread returned error.");

    // 14) If the function gets this far, the loop thread return an unexpected
    //     success status. Exit the process.
    exit (EXIT_FAILURE);
}

