#include <set>

#include "ControlNode.hpp"

/******************************** CONSTANTS ***********************************/

/**
 * Period of loop function.
 */
static const uint32_t LOOP_PERIOD_MS = 10;

/**
 * Device Nodes.
 */
static const std::vector<Node_t> DEVICE_NODES =
{
    NODE_DEVICE0,
    NODE_DEVICE1,
    NODE_DEVICE2,
};

/**
 * Time per loop available to the communications step.
 */
static const Time::TimeNs_t COMMUNICATIONS_TIME_SLICE_NS = 2.2 * Time::NS_IN_MS;

/**
 * Used to calculate the recvMult timeout. The timeout must leave enough time in
 * the communications time slice for the recvMult overhead that is not included
 * in the timeout, received data to be saved to the Data Vector, and any errors 
 * to be logged. This buffer accounts for that.
 */
static const Time::TimeNs_t COMMUNICATIONS_TIME_BUFFER_NS = 
                                                        500 * Time::NS_IN_US;

/**
 * RecvMult will be called with at least this timeout, even if that means the 
 * comms deadline will be missed.
 */
static const Time::TimeNs_t MIN_RECV_TIMEOUT_NS = 100 * Time::NS_IN_US;

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
 * Statically allocated buffer for receiving data from Ground over the network.
 */
static std::vector<uint8_t> gGndToCnBuf;

/**
 * Statically allocated buffers for receiving data from Device Nodes over the 
 * network.
 */
static std::vector<std::vector<uint8_t>> gDnRecvBufs (DEVICE_NODES.size ());

/**
 * Statically allocated buffers for sending data over the network.
 */
static std::vector<uint8_t> gCnToDn0Buf;
static std::vector<uint8_t> gCnToDn1Buf;
static std::vector<uint8_t> gCnToDn2Buf;
static std::vector<uint8_t> gCnToGndBuf;

/***************************** PRIVATE FUNCIONS *******************************/

/**
 * Handle a missed scheduler deadline. No other errors are expected since loop
 * never returns.
 *
 * @param  kError     Error to handle.
 *
 * @ret    E_SUCCESS     
 */
static Error_t periodicErrorHandler (Error_t kError)
{
    // Log deadline miss.
    if (kError == E_MISSED_SCHEDULER_DEADLINE)
    {
        Errors::incrementOnError (gPDv->increment (
                                           DV_ELEM_CN_LOOP_DEADLINE_MISSES),
                                 gPDv, DV_ELEM_CN_ERROR_COUNT);
        return E_SUCCESS;
    }

    return kError;
}

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
        DV_ELEM_CN_LOOP_DEADLINE_MISSES,
        DV_ELEM_CN_COMMS_DEADLINE_MISSES,
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
    gGndToCnBuf.resize (gndToCnBufSize);
    for (uint8_t i = 0; i < DEVICE_NODES.size (); i++)
    {
        switch (DEVICE_NODES[i])
        {
            case NODE_DEVICE0:
                gDnRecvBufs[i].resize (dn0ToCnBufSize);
                break;
            case NODE_DEVICE1:
                gDnRecvBufs[i].resize (dn1ToCnBufSize);
                break;
            case NODE_DEVICE2:
                gDnRecvBufs[i].resize (dn2ToCnBufSize);
                break;
            default:
                // This should never happen.
                return E_INVALID_CONFIG;
        }
    }

    return E_SUCCESS;
}

/**
 * Helper to send/recv Data Vector data to/from Device Nodes and Ground.
 *
 * @ret  E_SUCCESS                  Successfully received data.
 *       E_FAILED_TO_GET_TIME       Could not read time.
 *       E_DATA_VECTOR_READ         Failed to copy data from Data Vector.
 *       E_DATA_VECTOR_WRITE        Failed to write data to Data Vector.
 *       E_NETWORK_MANAGER_RX_FAIL  Failed to recv data from nodes.
 *       E_NETWORK_MANAGER_TX_FAIL  Failed to send data to nodes.
 */
static Error_t sendAndRecvDataVectorData ()
{
    // 1) Get start time, deadline time, and deadline time - buffer. These are 
    //    used for managing the communications so that they are completed within 
    //    the time slice.
    Time::TimeNs_t startTimeNs = 0;
    if (gPTime->getTimeNs (startTimeNs) != E_SUCCESS)
    {
        return E_FAILED_TO_GET_TIME;
    }

    // 2) Copy Data Vector data to buffers.
    if (gPDv->readRegion (DV_REG_CN_TO_DN0, gCnToDn0Buf) != E_SUCCESS ||
        gPDv->readRegion (DV_REG_CN_TO_DN1, gCnToDn1Buf) != E_SUCCESS ||
        gPDv->readRegion (DV_REG_CN_TO_DN2, gCnToDn2Buf) != E_SUCCESS ||
        gPDv->readDataVector (gCnToGndBuf)               != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // 3) Send data to respective nodes. Send to Ground last so that there is
    //    additional time for Device Nodes to respond before end of comms 
    //    deadline.
    if (gPNm->send (NODE_DEVICE0, gCnToDn0Buf) != E_SUCCESS ||
        gPNm->send (NODE_DEVICE1, gCnToDn1Buf) != E_SUCCESS ||
        gPNm->send (NODE_DEVICE2, gCnToDn2Buf) != E_SUCCESS ||
        gPNm->send (NODE_GROUND,  gCnToGndBuf) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_TX_FAIL;
    }

    // 4) Attempt to receive data from Ground. Only done once per loop so that 
    //    sequential commands do not overwrite each other.
    bool msgRecvd = false;
    if (gPNm->recvNoBlock (NODE_GROUND, gGndToCnBuf, msgRecvd) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_RX_FAIL;
    }
    if (msgRecvd == true)
    {
        if (gPDv->writeRegion (DV_REG_GROUND_TO_CN, gGndToCnBuf))
        {
            return E_DATA_VECTOR_WRITE;
        }
    }

    // 5) Calculate remaining time in communications time slice to use as 
    //    timeout for recvMult call. Minimum timeout is MIN_RECV_TIMEOUT_NS,
    //    even if this will cause a communications deadline miss.
    Time::TimeNs_t currTimeNs = 0;
    if (gPTime->getTimeNs (currTimeNs) != E_SUCCESS)
    {
        return E_FAILED_TO_GET_TIME;
    }

    // 5a) If we've already gone into the buffer or we are less than 
    //     MIN_RECV_TIMEOUT_NS away from the buffer, set the timeout to be 
    //     MIN_RECV_TIMEOUT_NS.
    Time::TimeNs_t deadlineTimeNs = startTimeNs + COMMUNICATIONS_TIME_SLICE_NS;
    Time::TimeNs_t deadlineBufferTimeNs = deadlineTimeNs - 
                                          COMMUNICATIONS_TIME_BUFFER_NS;
    Time::TimeNs_t recvMultTimeoutNs = 0;
    if (currTimeNs > deadlineBufferTimeNs ||
        deadlineBufferTimeNs - currTimeNs < MIN_RECV_TIMEOUT_NS)
    {
        recvMultTimeoutNs = MIN_RECV_TIMEOUT_NS;
    }

    // 5b) Otherwise, set the timeout to be the remaining time before we hit the
    //     buffer time.
    else
    {
        recvMultTimeoutNs = deadlineBufferTimeNs - currTimeNs;
    }

    // 6) Receive data from Device Nodes.
    std::vector<uint32_t> numMsgsReceived (DEVICE_NODES.size (), 0);
    if (gPNm->recvMult (recvMultTimeoutNs, DEVICE_NODES, gDnRecvBufs, 
                        numMsgsReceived) != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_RX_FAIL;
    }

    // 7) Copy data from buffers to Data Vector or log a missed message.
    for (uint8_t i = 0; i < DEVICE_NODES.size (); i++)
    {
        Error_t ret = E_SUCCESS;
        switch (DEVICE_NODES[i])
        {
            case NODE_DEVICE0:
                ret = numMsgsReceived[i] > 0
                    ? gPDv->writeRegion (DV_REG_DN0_TO_CN, gDnRecvBufs[i])
                    : gPDv->increment (DV_ELEM_DN0_RX_MISS_COUNT);
                break;
            case NODE_DEVICE1:
                ret = numMsgsReceived[i] > 0
                    ? gPDv->writeRegion (DV_REG_DN1_TO_CN, gDnRecvBufs[i])
                    : gPDv->increment (DV_ELEM_DN1_RX_MISS_COUNT);
                break;
            case NODE_DEVICE2:
                ret = numMsgsReceived[i] > 0
                    ? gPDv->writeRegion (DV_REG_DN2_TO_CN, gDnRecvBufs[i])
                    : gPDv->increment (DV_ELEM_DN2_RX_MISS_COUNT);
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

    // 8) If the communications did not complete before deadline, log an error. 
    //    Otherwise, spin until deadline is reached to reduce jitter in when
    //    State Machine and Controllers run.
    if (gPTime->getTimeNs (currTimeNs) != E_SUCCESS)
    {
        return E_FAILED_TO_GET_TIME;
    }

    if (currTimeNs > deadlineTimeNs)
    {
        // Log deadline miss.
        if (gPDv->increment (DV_ELEM_CN_COMMS_DEADLINE_MISSES) != E_SUCCESS)
        {
            return E_DATA_VECTOR_WRITE;
        }
    }
    else
    {
        // Spin.
        while (currTimeNs < deadlineTimeNs)
        {
            if (gPTime->getTimeNs (currTimeNs) != E_SUCCESS)
            {
                return E_FAILED_TO_GET_TIME;
            }
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
 * @param   _kArgs     Unused.
 *
 * @ret     E_SUCCESS  Loop executed successfully. Errors may have been logged.
 *                     If Errors::incrementOnError fails, fails silently.
 */
static void* loop (void* _kArgs)
{
    // 1) Send and receive Data Vector Regions with Device Nodes and Ground.
    //    This step doubles as a loop synchronizer, as all Device Nodes begin 
    //    their loop on receiving a message from the Control Node.
    Errors::incrementOnError (sendAndRecvDataVectorData (), gPDv,
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
    
    // 5) Step the State Machine.
    Errors::incrementOnError (gPSm->step (currTimeNs), gPDv, 
                              DV_ELEM_CN_ERROR_COUNT);
    
    // 6) Run the Controllers.
    for (std::unique_ptr<Controller>& pCtrl : gPCtrls)
    {
        Errors::incrementOnError (pCtrl->run (), gPDv, DV_ELEM_CN_ERROR_COUNT);
    }

    // 7) Increment loop counter.
    Errors::incrementOnError (gPDv->increment (DV_ELEM_CN_LOOP_COUNT), gPDv,
                              DV_ELEM_CN_ERROR_COUNT);

    return (void *) E_SUCCESS;
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
    Errors::exitOnError (ThreadManager::getInstance (pTm),
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
    ThreadManager::ThreadFunc_t fLoop = (ThreadManager::ThreadFunc_t) loop;
    ThreadManager::ErrorHandler_t fError = 
        (ThreadManager::ErrorHandler_t) &periodicErrorHandler;
    Errors::exitOnError (pTm->createPeriodicThread (
                                      loopThread, fLoop, nullptr, 0,
                                      ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                      ThreadManager::Affinity_t::CORE_1,
                                      LOOP_PERIOD_MS, fError),
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

