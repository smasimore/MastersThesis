#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "ControlNode.hpp"

#include "TestHelpers.hpp"

/********************************* MACROS *************************************/

/**
 * Fork a process, run ControlNode::entry, and verify process exits as 
 * expected.
 *
 * @param  kNmConfig          Network Manager config.
 * @param  kDvConfig          Data Vector config.
 * @param  kSmConfig          State Machine config.
 * @param  kChConfig          Command Handler config.
 * @param  kFInitControllers  Function pointer to controller init function.
 */
#define TEST_ENTRY_EXIT_ON_ERROR(kNmConfig, kDvConfig, kSmConfig, kChConfig,   \
                                 kFInitControllers)                            \
{                                                                              \
    pid_t pid = fork ();                                                       \
    if (pid == 0)                                                              \
    {                                                                          \
        ControlNode::entry (kNmConfig, kDvConfig, kSmConfig, kChConfig,        \
                            kFInitControllers);                                \
        exit (EXIT_SUCCESS);                                                   \
    }                                                                          \
    else if (pid > 0)                                                          \
    {                                                                          \
        int32_t status = 0;                                                    \
        pid_t waitedPid = waitpid (pid, &status, 0);                           \
        if (waitedPid != pid)                                                  \
        {                                                                      \
            FAIL ("Unknown PID waited for.");                                  \
        }                                                                      \
        if (WIFEXITED (status) != 0)                                           \
        {                                                                      \
            CHECK_EQUAL (EXIT_FAILURE, WEXITSTATUS (status));                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            FAIL ("Process exited unexpectedly.");                             \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        FAIL ("Fork failed.");                                                 \
    }                                                                          \
}

/**
 * Create a thread to simulate the Device and Ground nodes. 
 *
 * @param  kSyncSuccess  Whether or not to successfully clock sync with the CN.
 * @param  kEnterLoop    Whether or not to enter node simulation loop.
 */
#define CREATE_SIM_THREAD(kSyncSuccess, kEnterLoop)                            \
    ThreadManager* pTm = nullptr;                                              \
    CHECK_SUCCESS (ThreadManager::getInstance (pTm));                          \
    pthread_t thread;                                                          \
    FuncArgs_t args = {kSyncSuccess, kEnterLoop};                              \
    CHECK_SUCCESS (pTm->createThread (thread, gFNodesSim, &args,               \
                                      sizeof (args),                           \
                                      ThreadManager::MIN_NEW_THREAD_PRIORITY,  \
                                      ThreadManager::Affinity_t::CORE_1));

/************************** CONTROL NODE CONFIGS ******************************/

/**
 * Loopback Network Manager config. 
 */
static NetworkManager::Config_t gNmConfig =
{
    // Nodes
    {
        {NODE_CONTROL, "127.0.0.1"},
        {NODE_DEVICE0, "127.0.0.2"},
        {NODE_DEVICE1, "127.0.0.3"},
        {NODE_DEVICE2, "127.0.0.4"},
        {NODE_GROUND,  "127.0.0.5"},
    },
    // Channels
    {
        {NODE_CONTROL, NODE_DEVICE0, 
         static_cast<uint16_t> (NetworkManager::MIN_PORT)},
        {NODE_CONTROL, NODE_DEVICE1, 
         static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
        {NODE_CONTROL, NODE_DEVICE2, 
         static_cast<uint16_t> (NetworkManager::MIN_PORT + 2)},
        {NODE_CONTROL, NODE_GROUND, 
         static_cast<uint16_t> (NetworkManager::MIN_PORT + 3)},
    },
    // Me
    NODE_CONTROL,
    // Msg Tx and Rx Counters.
    DV_ELEM_CN_MSG_TX_COUNT,
    DV_ELEM_CN_MSG_RX_COUNT,
};

/**
 * Data Vector config. 
 */
static DataVector::Config_t gDvConfig =
{
///////////////////////////////// DV_REG_CN ////////////////////////////////////
    {DV_REG_CN,
    // TYPE          ELEM                           INITIAL_VAL
    {DV_ADD_UINT32 ( DV_ELEM_CN_LOOP_COUNT,         0            ),
     DV_ADD_UINT32 ( DV_ELEM_CN_ERROR_COUNT,        0            ),
     DV_ADD_UINT32 ( DV_ELEM_CN_MSG_TX_COUNT,       0            ),
     DV_ADD_UINT32 ( DV_ELEM_CN_MSG_RX_COUNT,       0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN0_RX_MISS_COUNT,     0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN1_RX_MISS_COUNT,     0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN2_RX_MISS_COUNT,     0            ),
     DV_ADD_UINT32 ( DV_ELEM_CN_DEADLINE_MISSES,    0            ),
     DV_ADD_UINT8  ( DV_ELEM_CMD,                   CMD_NONE     ),
     DV_ADD_UINT32 ( DV_ELEM_LAST_CMD_PROC_NUM,     0            ),
     DV_ADD_UINT8  ( DV_ELEM_DN_RESP_CTRL_MODE,     MODE_SAFED   ),
     DV_ADD_UINT8  ( DV_ELEM_ERROR_CTRL_MODE,       MODE_SAFED   ),
     DV_ADD_UINT8  ( DV_ELEM_MISS_CTRL_MODE,        MODE_SAFED   ),
     DV_ADD_UINT8  ( DV_ELEM_THREAD_KILL_CTRL_MODE, MODE_SAFED   ),
     DV_ADD_UINT64 ( DV_ELEM_CN_TIME_NS,            0            ),
     DV_ADD_UINT32 ( DV_ELEM_STATE,                 STATE_A      ),
     DV_ADD_BOOL   ( DV_ELEM_TEST6,                 false        )}},

    {DV_REG_CN_TO_DN0,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_BOOL   ( DV_ELEM_TEST0,                 false        )}},

    {DV_REG_CN_TO_DN1,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_BOOL   ( DV_ELEM_TEST1,                 false        )}},

    {DV_REG_CN_TO_DN2,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_BOOL   ( DV_ELEM_TEST2,                 false        )}},

    {DV_REG_DN0_TO_CN,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_DN0_MSG_TX_COUNT,      0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN0_MSG_RX_COUNT,      0            ),
     DV_ADD_BOOL   ( DV_ELEM_TEST3,                 false        )}},

    {DV_REG_DN1_TO_CN,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_DN1_MSG_TX_COUNT,      0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN1_MSG_RX_COUNT,      0            ),
     DV_ADD_BOOL   ( DV_ELEM_TEST4,                 false        )}},

    {DV_REG_DN2_TO_CN,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_DN2_MSG_TX_COUNT,      0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN2_MSG_RX_COUNT,      0            ),
     DV_ADD_BOOL   ( DV_ELEM_TEST5,                 false        )}},

    {DV_REG_GROUND_TO_CN,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_GROUND_MSG_TX_COUNT,   0            ),
     DV_ADD_UINT32 ( DV_ELEM_GROUND_MSG_RX_COUNT,   0            ),
     DV_ADD_UINT8  ( DV_ELEM_CMD_REQ,               CMD_NONE     ),
     DV_ADD_UINT32 ( DV_ELEM_LAST_CMD_REQ_NUM,      0            ),
     DV_ADD_UINT32 ( DV_ELEM_CMD_WRITE_ELEM,        DV_ELEM_LAST ),
     DV_ADD_UINT64 ( DV_ELEM_CMD_WRITE_VAL,         0            )}},
};

/**
 * Command Handler config.
 */
static CommandHandler::Config_t gChConfig =
{
    DV_ELEM_CMD,
    DV_ELEM_CMD_REQ,
    DV_ELEM_CMD_WRITE_ELEM,
    DV_ELEM_CMD_WRITE_VAL,
    DV_ELEM_LAST_CMD_REQ_NUM,    
    DV_ELEM_LAST_CMD_PROC_NUM,
};

/**
 * State Machine config. 
 */
static StateMachine::Config_t gSmConfig =
{
    //////////////////////////////// STATE_A ///////////////////////////////////
    //
    // Initial state transitions to STATE_B after 10 loops.
    //
    // ID
    {STATE_A,
    // 
    // ACTIONS
    {},
    // 
    // TRANSITIONS
    {TR_CREATE_UINT32 ( DV_ELEM_CN_LOOP_COUNT,  CMP_EQUALS,  10,  STATE_B )}},


    //////////////////////////////// STATE_B ///////////////////////////////////
    //
    // Loops until receives CMD_LAUNCH from ground. Then transitions to
    // STATE_C.
    //
    // ID
    {STATE_B,
    //
    // ACTIONS
    {},
    //
    // TRANSITIONS
    {TR_CREATE_UINT8 ( DV_ELEM_CMD,  CMP_EQUALS,  CMD_LAUNCH,  STATE_C )}},


    //////////////////////////////// STATE_C ///////////////////////////////////
    //
    // Sets flags to true and sends them to Device Nodes. Controller waits for 
    // each Device Node to respond with an ack and then sets the transition 
    // flag. On reading transition flag, transitions to STATE_D.
    //
    // ID
    {STATE_C,
    //
    // ACTIONS
    {{0 * Time::NS_IN_SECOND,
        {ACT_CREATE_UINT8  ( DV_ELEM_DN_RESP_CTRL_MODE, MODE_ENABLED )}},

     {.01 * Time::NS_IN_SECOND,
         {ACT_CREATE_BOOL  ( DV_ELEM_TEST0,  true ),
          ACT_CREATE_BOOL  ( DV_ELEM_TEST1,  true ),
          ACT_CREATE_BOOL  ( DV_ELEM_TEST2,  true )}}},
    //
    // TRANSITIONS
    {TR_CREATE_BOOL ( DV_ELEM_TEST6,  CMP_EQUALS,  true,  STATE_D )}},


    //////////////////////////////// STATE_D ///////////////////////////////////
    //
    // Enables ErrorController and then DeadlineMissController. Transitions 
    // after a deadline is missed. 
    //
    // ID
    {STATE_D,
    //
    // ACTIONS
    {{0 * Time::NS_IN_SECOND,
        {ACT_CREATE_UINT8  ( DV_ELEM_DN_RESP_CTRL_MODE,     MODE_SAFED   ),
         ACT_CREATE_UINT8  ( DV_ELEM_ERROR_CTRL_MODE,       MODE_ENABLED )}},
     {.01 * Time::NS_IN_SECOND,
        {ACT_CREATE_UINT8  ( DV_ELEM_MISS_CTRL_MODE,        MODE_ENABLED )}}},
    //
    // TRANSITIONS
    {TR_CREATE_UINT32  ( DV_ELEM_CN_DEADLINE_MISSES,  CMP_EQUALS,  1,  STATE_E )}},


    //////////////////////////////// STATE_E ///////////////////////////////////
    //
    // Enables ThreadKillController, which kills the thread.
    //
    // ID
    {STATE_E,
    //
    // ACTIONS
    {{.01 * Time::NS_IN_SECOND,
        {ACT_CREATE_UINT8  ( DV_ELEM_THREAD_KILL_CTRL_MODE, MODE_ENABLED )}}},
    //
    // TRANSITIONS
    {}},
};

/******************************* CONTROLLERS **********************************/

/**
 * Controller to aggregate responses from simulated Device Nodes and set
 * transition flag.
 */
class CheckDeviceNodeResponsesController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Check DN flags and if all set, set transition flag.
         */
        Error_t runEnabled ()
        {
            // Read to see if DN's have responded.
            bool dn0 = false;
            bool dn1 = false;
            bool dn2 = false;
            if (mPDataVector->read (DV_ELEM_TEST3, dn0) != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_TEST4, dn1) != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_TEST5, dn2) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            // If all have, set flag to true.
            if (dn0 == true && dn1 == true && dn2 == true)
            {
                if (mPDataVector->write (DV_ELEM_TEST6, (bool) true) != 
                        E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }
            }

            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t runSafed ()
        {
            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t verifyConfig ()
        {
            return E_SUCCESS;
        }

        CheckDeviceNodeResponsesController (Config_t kConfig,
                                            std::shared_ptr<DataVector> kPDv,
                                            DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem),
            mConfig (kConfig) {}
    
    private:

        /**
         * Unused.
         */
        Config_t mConfig;
};

/**
 * Controller to test error logging on controller error.
 */
class ErrorController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Return error.
         */
        Error_t runEnabled ()
        {
            return E_INVALID_ELEM;
        }

        /**
         * Do nothing.
         */
        Error_t runSafed ()
        {
            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t verifyConfig ()
        {
            return E_SUCCESS;
        }

        ErrorController (Config_t kConfig,
                         std::shared_ptr<DataVector> kPDv,
                         DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem),
            mConfig (kConfig) {}
    
    private:

        /**
         * Unused.
         */
        Config_t mConfig;
};

/**
 * Controller to test error logging on deadline miss.
 */
class DeadlineMissController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Sleep for 20ms, causing a deadline miss.
         */
        Error_t runEnabled ()
        {
            TestHelpers::sleepMs (20);
            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t runSafed ()
        {
            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t verifyConfig ()
        {
            return E_SUCCESS;
        }

        DeadlineMissController (Config_t kConfig,
                                std::shared_ptr<DataVector> kPDv,
                                DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem),
            mConfig (kConfig) {}
    
    private:

        /**
         * Unused.
         */
        Config_t mConfig;
};

/**
 * Controller to immediately kill thread.
 */
class ThreadKillController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Kill thread.
         */
        Error_t runEnabled ()
        {
            pthread_exit ((void*) E_SUCCESS);
        }

        /**
         * Do nothing.
         */
        Error_t runSafed ()
        {
            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t verifyConfig ()
        {
            return E_SUCCESS;
        }

        ThreadKillController (Config_t kConfig,
                              std::shared_ptr<DataVector> kPDv,
                              DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem),
            mConfig (kConfig) {}
    
    private:

        /**
         * Unused.
         */
        Config_t mConfig;
};
            

/**
 * Success & failure function and function pointers to initialize Controllers.
 */
static Error_t initializeControllersSuccess (
                  std::shared_ptr<DataVector> kPDv,
                  std::vector<std::unique_ptr<Controller>>& kPCtlrsVecRet)
{
    std::unique_ptr<CheckDeviceNodeResponsesController> pDnResponsesCtrlr;
    CheckDeviceNodeResponsesController::Config_t dnResponsesCtrlConfig = {};
    CHECK_SUCCESS (Controller::createNew (dnResponsesCtrlConfig, kPDv, 
                                          DV_ELEM_DN_RESP_CTRL_MODE, 
                                          pDnResponsesCtrlr));

    std::unique_ptr<ErrorController> pErrorCtrlr;
    ErrorController::Config_t errorCtrlConfig = {};
    CHECK_SUCCESS (Controller::createNew (errorCtrlConfig, kPDv, 
                                          DV_ELEM_ERROR_CTRL_MODE, 
                                          pErrorCtrlr));

    std::unique_ptr<DeadlineMissController> pMissCtrlr;
    DeadlineMissController::Config_t missCtrlConfig = {};
    CHECK_SUCCESS (Controller::createNew (missCtrlConfig, kPDv, 
                                          DV_ELEM_MISS_CTRL_MODE, 
                                          pMissCtrlr));

    std::unique_ptr<ThreadKillController> pThreadKillCtrlr;
    ThreadKillController::Config_t threadKillCtrlConfig = {};
    CHECK_SUCCESS (Controller::createNew (threadKillCtrlConfig, kPDv, 
                                          DV_ELEM_THREAD_KILL_CTRL_MODE, 
                                          pThreadKillCtrlr));

    // Add controllers to return vector. std::move necessary since controllers 
    // are stored in unique_ptr. Move transfers ownership of the object.
    kPCtlrsVecRet.push_back (std::move (pDnResponsesCtrlr));
    kPCtlrsVecRet.push_back (std::move (pErrorCtrlr));
    kPCtlrsVecRet.push_back (std::move (pMissCtrlr));
    kPCtlrsVecRet.push_back (std::move (pThreadKillCtrlr));
    
    return E_SUCCESS;
}
static ControlNode::fInitializeControllers_t gFInitControllersSuccess = 
    (ControlNode::fInitializeControllers_t) initializeControllersSuccess;

/**
 * Failing controller initialization function.
 */
static Error_t initializeControllersFail (
                  std::shared_ptr<DataVector> kPDv,
                  std::vector<std::unique_ptr<Controller>>& kPControllersVec)
{
    return E_INVALID_ELEM;
}
static ControlNode::fInitializeControllers_t gFInitControllersFail = 
    (ControlNode::fInitializeControllers_t) initializeControllersFail;

/******************* DEVICE AND GROUND NODE SIMULATION ************************/

/**
 * Global Data Vector to store telemetry sent from Control Node. Used to verify 
 * the Control Node entry and loop functionality.
 */
static std::shared_ptr<DataVector> gPTelemDv = nullptr;

/**
 * Number of loops executed by sim thread. Used to determine how many tx/rx
 * msgs to expect in final telemetry snapshot.
 */
static uint32_t gNumSimLoops = 0;

/**
 * Function args for node simulation thread. If syncSuccess is true, Device 
 * Nodes will respond with success during clock synchronization, failure
 * otherwise. If enterLoop is true, thread will simulate Device Node loops.
 */
typedef struct FuncArgs
{
    bool syncSuccess;
    bool enterLoop;
} FuncArgs_t;

/**
 * Node simulation thread. Simulates clock synchronization and nodes tx'ing/
 * rx'ing data with Control Node.
 */
static void* fNodesSim (void* _args)
{
    // Cast args struct.
    FuncArgs_t args = *((FuncArgs_t*) _args);

    // Init DV. Use the same for all simulated Device Nodes.
    INIT_DATA_VECTOR (gDvConfig);

    // Init telem DV that is used to receive the telemetry snapshots from the 
    // Control Node.
    CHECK_SUCCESS (DataVector::createNew (gDvConfig, gPTelemDv));

    // Create NM's.
    NetworkManager::Config_t dn0Config = gNmConfig;
    NetworkManager::Config_t dn1Config = gNmConfig;
    NetworkManager::Config_t dn2Config = gNmConfig;
    NetworkManager::Config_t gndConfig = gNmConfig;
    dn0Config.me = NODE_DEVICE0;
    dn0Config.dvElemMsgTxCount = DV_ELEM_DN0_MSG_TX_COUNT;
    dn0Config.dvElemMsgRxCount = DV_ELEM_DN0_MSG_RX_COUNT;
    dn1Config.me = NODE_DEVICE1;
    dn1Config.dvElemMsgTxCount = DV_ELEM_DN1_MSG_TX_COUNT;
    dn1Config.dvElemMsgRxCount = DV_ELEM_DN1_MSG_RX_COUNT;
    dn2Config.me = NODE_DEVICE2;
    dn2Config.dvElemMsgTxCount = DV_ELEM_DN2_MSG_TX_COUNT;
    dn2Config.dvElemMsgRxCount = DV_ELEM_DN2_MSG_RX_COUNT;
    gndConfig.me = NODE_GROUND;
    gndConfig.dvElemMsgTxCount = DV_ELEM_GROUND_MSG_TX_COUNT;
    gndConfig.dvElemMsgRxCount = DV_ELEM_GROUND_MSG_RX_COUNT;
    std::shared_ptr<NetworkManager> pDn0Nm = nullptr;
    std::shared_ptr<NetworkManager> pDn1Nm = nullptr;
    std::shared_ptr<NetworkManager> pDn2Nm = nullptr;
    std::shared_ptr<NetworkManager> pGndNm = nullptr;
    CHECK_SUCCESS (NetworkManager::createNew (dn0Config, pDv, pDn0Nm));
    CHECK_SUCCESS (NetworkManager::createNew (dn1Config, pDv, pDn1Nm));
    CHECK_SUCCESS (NetworkManager::createNew (dn2Config, pDv, pDn2Nm));
    CHECK_SUCCESS (NetworkManager::createNew (gndConfig, pDv, pGndNm));

    // Wait for clock sync SERVER_READY messages.
    std::vector<uint8_t> msg (1);
    CHECK_SUCCESS (pDn0Nm->recv (NODE_CONTROL, msg));
    CHECK_SUCCESS (pDn1Nm->recv (NODE_CONTROL, msg));
    CHECK_SUCCESS (pDn2Nm->recv (NODE_CONTROL, msg));

    // Send clock sync responses.
    msg[0] = args.syncSuccess == true 
        ? ClockSync::Msg_t::CLIENT_SYNC_SUCCESS
        : ClockSync::Msg_t::CLIENT_SYNC_FAIL;
    CHECK_SUCCESS (pDn0Nm->send (NODE_CONTROL, msg));
    CHECK_SUCCESS (pDn1Nm->send (NODE_CONTROL, msg));
    CHECK_SUCCESS (pDn2Nm->send (NODE_CONTROL, msg));

    // Initialize tx/rx buffers.
    uint32_t cnToDn0BufSize = 0;                                                 
    uint32_t cnToDn1BufSize = 0;                                                 
    uint32_t cnToDn2BufSize = 0;                                                 
    uint32_t telemBufSize   = 0;                                                 
    uint32_t dn0ToCnBufSize = 0;                                                 
    uint32_t dn1ToCnBufSize = 0;                                                    
    uint32_t dn2ToCnBufSize = 0;                                                    
    uint32_t gndToCnBufSize = 0;                                                    
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_CN_TO_DN0, cnToDn0BufSize));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_CN_TO_DN1, cnToDn1BufSize));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_CN_TO_DN2, cnToDn2BufSize));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_DN0_TO_CN, dn0ToCnBufSize));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_DN1_TO_CN, dn1ToCnBufSize));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_DN2_TO_CN, dn2ToCnBufSize));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_GROUND_TO_CN, 
                                            gndToCnBufSize));
    CHECK_SUCCESS (pDv->getDataVectorSizeBytes (telemBufSize));
    std::vector<uint8_t> cnToDn0Buf (cnToDn0BufSize);                                            
    std::vector<uint8_t> cnToDn1Buf (cnToDn1BufSize);                                            
    std::vector<uint8_t> cnToDn2Buf (cnToDn2BufSize);                                            
    std::vector<uint8_t> dn0ToCnBuf (dn0ToCnBufSize);                                            
    std::vector<uint8_t> dn1ToCnBuf (dn1ToCnBufSize);                                            
    std::vector<uint8_t> dn2ToCnBuf (dn2ToCnBufSize);                                            
    std::vector<uint8_t> gndToCnBuf (gndToCnBufSize); 
    std::vector<uint8_t> telemBuf   (telemBufSize);                                            

    // Simulate node loops.
    if (args.enterLoop == true)
    {
        while (1)
        {
            // Receive data from CN and store in pDv.
            CHECK_SUCCESS (pDn0Nm->recv (NODE_CONTROL,
                           cnToDn0Buf));
            CHECK_SUCCESS (pDn1Nm->recv (NODE_CONTROL,
                           cnToDn1Buf));
            CHECK_SUCCESS (pDn2Nm->recv (NODE_CONTROL,
                           cnToDn2Buf));
            CHECK_SUCCESS (pGndNm->recv (NODE_CONTROL,
                           telemBuf));
            CHECK_SUCCESS (pDv->writeRegion (DV_REG_CN_TO_DN0, cnToDn0Buf));
            CHECK_SUCCESS (pDv->writeRegion (DV_REG_CN_TO_DN1, cnToDn1Buf));
            CHECK_SUCCESS (pDv->writeRegion (DV_REG_CN_TO_DN2, cnToDn2Buf));
            CHECK_SUCCESS (gPTelemDv->writeDataVector (telemBuf));

            // If we're in STATE_B, "send" a LAUNCH cmd from the ground node.
            uint32_t state = STATE_A;
            uint32_t reqNum = 0;
            CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_STATE, state));
            CHECK_SUCCESS (pDv->read (DV_ELEM_LAST_CMD_REQ_NUM, reqNum));
            if (state == STATE_B && reqNum == 0)
            {
                CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, 
                                           (uint8_t) CMD_LAUNCH));
                CHECK_SUCCESS (pDv->increment (DV_ELEM_LAST_CMD_REQ_NUM));
                CHECK_SUCCESS (pDv->readRegion (DV_REG_GROUND_TO_CN, 
                                                gndToCnBuf));
                CHECK_SUCCESS (pGndNm->send (
                                         NODE_CONTROL,
                                         gndToCnBuf));
            }

            // If we receive a flag from CN, send ack flag.
            bool dn0Flag = false;
            bool dn1Flag = false;
            bool dn2Flag = false;
            CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0, dn0Flag));
            CHECK_SUCCESS (pDv->read (DV_ELEM_TEST1, dn1Flag));
            CHECK_SUCCESS (pDv->read (DV_ELEM_TEST2, dn2Flag));
            CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, dn0Flag));
            CHECK_SUCCESS (pDv->write (DV_ELEM_TEST4, dn1Flag));
            CHECK_SUCCESS (pDv->write (DV_ELEM_TEST5, dn2Flag));

            // Send DN regions to CN.
            CHECK_SUCCESS (pDv->readRegion (DV_REG_DN0_TO_CN, dn0ToCnBuf));
            CHECK_SUCCESS (pDv->readRegion (DV_REG_DN1_TO_CN, dn1ToCnBuf));
            CHECK_SUCCESS (pDv->readRegion (DV_REG_DN2_TO_CN, dn2ToCnBuf));
            CHECK_SUCCESS (pDn0Nm->send (
                                     NODE_CONTROL,
                                     dn0ToCnBuf));
            CHECK_SUCCESS (pDn1Nm->send (
                                     NODE_CONTROL,
                                     dn1ToCnBuf));
            CHECK_SUCCESS (pDn2Nm->send (
                                     NODE_CONTROL,
                                     dn2ToCnBuf));

            // Increment sim loop counter.
            gNumSimLoops++;

            // Break once we reach STATE_E.
            if (state == STATE_E)
            {
                break;
            }
        }
    }

    return (void*) E_SUCCESS;
}

/**
 * Node sim function pointer.
 */
ThreadManager::ThreadFunc_t gFNodesSim = 
    (ThreadManager::ThreadFunc_t) &fNodesSim;

/********************************* TESTS **************************************/

TEST_GROUP (ControlNode)
{
    /** 
     * Clear global memory.
     */
    void teardown ()
    {
        gPTelemDv.reset ();
    }
};

/* Test entry with a bad DV config that does not contain a required region. No 
   need to init clock sync thread since DV initialized first. */
TEST (ControlNode, BadDvConfigMissingRequiredRegion)
{
    // Remove a required region.
    DataVector::Config_t dvConfig = gDvConfig;
    dvConfig.erase (dvConfig.begin ());

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, dvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);
};

/* Test entry with a bad DV config that does not contain a required elem. No 
   need to init clock sync thread since DV initialized first. */
TEST (ControlNode, BadDvConfigMissingRequiredElem)
{
    // Remove a required element.
    DataVector::Config_t dvConfig = gDvConfig;
    dvConfig[0].elems.erase (dvConfig[0].elems.begin ());

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, dvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);
};

/* Test entry with a bad DV config. No need to init clock sync thread since
   DV initialized first. */
TEST (ControlNode, BadDvConfig)
{
    // Keep required regions and elements to pass entry () checks, but add a 
    // dupe region to fail DV checks.
    DataVector::Config_t dvConfig = gDvConfig;
    DataVector::RegionConfig_t dupeRegion = {DV_REG_CN, {}};
    dvConfig.push_back (dupeRegion);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, dvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);
};

/* Test entry with a bad NM config that does not contain all required nodes. No 
   need to init clock sync thread since NM initialized pre clock sync. */
TEST (ControlNode, BadNmConfigMissingNode)
{
    // Remove a required node.
    NetworkManager::Config_t nmConfig = gNmConfig;
    nmConfig.nodeToIp.erase (nmConfig.nodeToIp.begin ());

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nmConfig, gDvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);
};

/* Test entry with a bad NM config that does not contain all required channels. 
   No need to init clock sync thread since NM initialized pre clock sync. */
TEST (ControlNode, BadNmConfigMissingChannel)
{
    // Remove a required channel.
    NetworkManager::Config_t nmConfig = gNmConfig;
    nmConfig.channels.erase (nmConfig.channels.begin ());

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nmConfig, gDvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);
};

/* Test entry with a bad NM config. No need to init clock sync thread since
   NM initialized pre clock sync. */
TEST (ControlNode, BadNmConfig)
{
    // Keep required nodes and channels to pass entry () checks, but use a dupe 
    // IP to fail NM checks.
    NetworkManager::Config_t nmConfig = gNmConfig;
    nmConfig.nodeToIp[NODE_DEVICE0] = "127.0.0.1";

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nmConfig, gDvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);
};

/* Test entry with failed clock sync. */
TEST (ControlNode, ClockSyncFail)
{
    // Create thread to simulate Device Nodes during clock sync. Blocks on 
    // waiting for clock sync msg.
    CREATE_SIM_THREAD (false, false);

    // Create process that calls entry. Expect this process to exit due to a 
    // failed clock sync step.
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, gDvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with a bad CH config. */
TEST (ControlNode, BadChConfig)
{
    // Create thread to simulate Device Nodes during clock sync. Blocks on 
    // waiting for clock sync msg.
    CREATE_SIM_THREAD (true, false);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    CommandHandler::Config_t emptyChConfig = {};
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, gDvConfig, emptyChConfig, gSmConfig, 
                              gFInitControllersSuccess);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with a bad SM config. */
TEST (ControlNode, BadSmConfig)
{
    // Create thread to simulate Device Nodes during clock sync. Blocks on 
    // waiting for clock sync msg.
    CREATE_SIM_THREAD (true, false);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    StateMachine::Config_t emptySmConfig = {};
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, gDvConfig, gChConfig, emptySmConfig, 
                              gFInitControllersSuccess);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with a error on controller initializations.. */
TEST (ControlNode, BadControllerInit)
{
    // Create thread to simulate Device Nodes during clock sync. Blocks on 
    // waiting for clock sync msg.
    CREATE_SIM_THREAD (true, false);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, gDvConfig, gChConfig, gSmConfig, 
                              gFInitControllersFail);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test running through test State Machine successfuly. */
TEST (ControlNode, Success)
{
    // Create thread to simulate Device and Ground Nodes. 
    CREATE_SIM_THREAD (true, true);

    // Create process that calls entry. Expect this process to exit once the
    // Control Node reaches STATE_D.
    TEST_ENTRY_EXIT_ON_ERROR (gNmConfig, gDvConfig, gChConfig, gSmConfig, 
                              gFInitControllersSuccess);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);

    // Expect 3 clock sync msgs on initialization and 4 data msgs per 
    // sim thread loop except for the final loop. This is due to the CN's last
    // msg rx's/tx's not being reflected in the last telem snapshot, since they
    // are incremented as a part of sending the snapshot.
    uint32_t cnMsgsTxCount = 0;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_CN_MSG_TX_COUNT, cnMsgsTxCount));
    CHECK_EQUAL (3 + 4 * (gNumSimLoops - 1), cnMsgsTxCount);

    // Expect 3 clock sync msgs on initialization, 1 from ground, and 3 DN msgs
    // per sim thread loop except for the final loop. 
    uint32_t cnMsgsRxCount = 0;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_CN_MSG_RX_COUNT, cnMsgsRxCount));
    CHECK_EQUAL (4 + 3 * (gNumSimLoops - 1), cnMsgsRxCount);

    // Expect no missed msgs.
    uint32_t dn0Misses = 0;
    uint32_t dn1Misses = 0;
    uint32_t dn2Misses = 0;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_DN0_RX_MISS_COUNT, dn0Misses));
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_DN1_RX_MISS_COUNT, dn1Misses));
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_DN2_RX_MISS_COUNT, dn2Misses));
    CHECK_EQUAL (0, dn0Misses);
    CHECK_EQUAL (0, dn1Misses);
    CHECK_EQUAL (0, dn2Misses);

    // Expect 3 errors due to ErrorController (2 in STATE_D, 1 in STATE_E).
    uint32_t numErrors = 0;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_CN_ERROR_COUNT, numErrors));
    CHECK_EQUAL (3, numErrors);

    // Expect 2 deadline misses due to DeadlineMissController. (1 in STATE_D, 1
    // in STATE_E).
    uint32_t numMisses = 0;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_CN_DEADLINE_MISSES, numMisses));
    CHECK_EQUAL (2, numMisses);

    // Expect to end in STATE_E.
    uint32_t state = STATE_A;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_STATE, state));
    CHECK_EQUAL (STATE_E, state);

    // Expect CheckDeviceNodeResponsesController to be safed and ErrorController
    // and DV_ELEM_MISS_CTRL_MODE to be enabled. ThreadKillController will also 
    // be safed since the thread is killed before we receive another telem 
    // snapshot.
    uint8_t dnRespCtrlMode     = MODE_SAFED;
    uint8_t errorCtrlMode      = MODE_SAFED;
    uint8_t missCtrlMode       = MODE_SAFED;
    uint8_t threadKillCtrlMode = MODE_SAFED;
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_DN_RESP_CTRL_MODE, dnRespCtrlMode));
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_ERROR_CTRL_MODE, errorCtrlMode));
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_MISS_CTRL_MODE, missCtrlMode));
    CHECK_SUCCESS (gPTelemDv->read (DV_ELEM_THREAD_KILL_CTRL_MODE, 
                                    threadKillCtrlMode));
    CHECK_EQUAL (MODE_SAFED,   dnRespCtrlMode);
    CHECK_EQUAL (MODE_SAFED,   threadKillCtrlMode);
    CHECK_EQUAL (MODE_ENABLED, errorCtrlMode);
    CHECK_EQUAL (MODE_ENABLED, missCtrlMode);
};
