#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "DeviceNode.hpp"
#include "DigitalOutDevice.hpp"

#include "TestHelpers.hpp"

/********************************* MACROS *************************************/

/**
 * Fork a process, run DeviceNode::entry, and verify process exits as 
 * expected.
 *
 * @param  kNmConfig           Network Manager config.
 * @param  kDvConfig           Data Vector config.
 * @param  kFInitCtrlsAndDevs  Function pointer to Controller and Device init 
 *                             function.
 * @param  kSkipClockSync      If true, don't attempt clock sync.
 */
#define TEST_ENTRY_EXIT_ON_ERROR(kNmConfig, kDvConfig, kFInitCtrlsAndDevs,     \
                                 kSkipClockSync)                               \
{                                                                              \
    pid_t pid = fork ();                                                       \
    if (pid == 0)                                                              \
    {                                                                          \
        DeviceNode::entry (kNmConfig, kDvConfig, kFInitCtrlsAndDevs,           \
                           kSkipClockSync);                                    \
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
 * Create a thread to simulate the Control Node. 
 *
 * @param  kClockSync    Whether or not to simulate clock sync.
 * @param  kEnterLoop    Whether or not to enter node simulation loop.
 * @param  kNode         Device Node being tested.
 * @param  kDvConfig     Device Node's DV config.
 * @param  kDvRegRecv    DN to CN Region.
 * @param  kDvRegSend    CN to DN Region.
 */
#define CREATE_SIM_THREAD(kClockSync, kEnterLoop, kNode, kDvConfig,            \
                          kDvRegRecv, kDvRegSend)                              \
    ThreadManager* pTm = nullptr;                                              \
    CHECK_SUCCESS (ThreadManager::getInstance (pTm));                          \
    pthread_t thread;                                                          \
    FuncArgs_t args = {kClockSync, kEnterLoop, kNode, kDvConfig, kDvRegRecv,   \
                       kDvRegSend};                                            \
    CHECK_SUCCESS (pTm->createThread (thread, gFControlNodeSim, &args,         \
                                      sizeof (args),                           \
                                      ThreadManager::MIN_NEW_THREAD_PRIORITY,  \
                                      ThreadManager::Affinity_t::CORE_1));

/**
 * Check state of "Control Node" Data Vector.
 *
 * @param  kDvElemError  Device Node's error elem.
 * @param  kDvElemTx     Device Node's msg tx count elem.
 * @param  kDvElemRx     Device Node's msg rx count elem.
 */
#define CHECK_DV(kDvElemError, kDvElemTx, kDvElemRx)                           \
{                                                                              \
    bool flag0 = false;                                                        \
    bool flag1 = false;                                                        \
    bool flag2 = false;                                                        \
    bool flag3 = false;                                                        \
    bool flag4 = false;                                                        \
    bool flag5 = false;                                                        \
    CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST0, flag0));                       \
    CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST1, flag1));                       \
    CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST2, flag2));                       \
    CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST3, flag3));                       \
    CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST4, flag4));                       \
    CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST5, flag5));                       \
    CHECK_EQUAL (true, flag0);                                                 \
    CHECK_EQUAL (true, flag1);                                                 \
    CHECK_EQUAL (true, flag2);                                                 \
    CHECK_EQUAL (true, flag3);                                                 \
    CHECK_EQUAL (true, flag4);                                                 \
    CHECK_EQUAL (true, flag5);                                                 \
    uint32_t errors = 0;                                                       \
    CHECK_SUCCESS (gPCnDv->read (kDvElemError, errors));                       \
    CHECK_EQUAL (1, errors);                                                   \
    uint32_t tx = 0;                                                           \
    uint32_t rx = 0;                                                           \
    CHECK_SUCCESS (gPCnDv->read (kDvElemTx, tx));                              \
    CHECK_SUCCESS (gPCnDv->read (kDvElemRx, rx));                              \
    CHECK_EQUAL (gNumSimLoops - 1, tx);                                        \
    CHECK_EQUAL (gNumSimLoops - 1, rx);                                        \
}

/*************************** DEVICE NODE CONFIGS ******************************/

/**
 * Loopback node to IP map for Network Manager configs.
 */
std::unordered_map<Node_t, NetworkManager::IP_t, EnumClassHash> gNodeToIp =
{
    {NODE_CONTROL, "127.0.0.1"},
    {NODE_DEVICE0, "127.0.0.2"},
    {NODE_DEVICE1, "127.0.0.3"},
    {NODE_DEVICE2, "127.0.0.4"},
    {NODE_GROUND,  "127.0.0.5"},
};

/**
 * Loopback channels for Network Manager configs.
 */
std::vector<NetworkManager::ChannelConfig_t> gChannels =
{
    {NODE_CONTROL, NODE_DEVICE0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    {NODE_CONTROL, NODE_DEVICE1, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
    {NODE_CONTROL, NODE_DEVICE2, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 2)},
    {NODE_CONTROL, NODE_GROUND, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 3)},
};

/**
 * Loopback Network Manager configs. 
 */
static NetworkManager::Config_t gNm0Config =
{
    // Nodes
    gNodeToIp,
    // Channels
    gChannels,
    // Me
    NODE_DEVICE0,
    // Msg Tx and Rx Counters.
    DV_ELEM_DN0_MSG_TX_COUNT,
    DV_ELEM_DN0_MSG_RX_COUNT,
};
static NetworkManager::Config_t gNm1Config =
{
    // Nodes
    gNodeToIp,
    // Channels
    gChannels,
    // Me
    NODE_DEVICE1,
    // Msg Tx and Rx Counters.
    DV_ELEM_DN1_MSG_TX_COUNT,
    DV_ELEM_DN1_MSG_RX_COUNT,
};
static NetworkManager::Config_t gNm2Config =
{
    // Nodes
    gNodeToIp,
    // Channels
    gChannels,
    // Me
    NODE_DEVICE2,
    // Msg Tx and Rx Counters.
    DV_ELEM_DN2_MSG_TX_COUNT,
    DV_ELEM_DN2_MSG_RX_COUNT,
};

/**
 * Device Node 0 Data Vector config. 
 */
static DataVector::Config_t gDv0Config =
{
    {DV_REG_DN0_TO_CN,
    // TYPE          ELEM                            INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_DN0_LOOP_COUNT,         0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN0_ERROR_COUNT,        0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN0_MSG_TX_COUNT,       0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN0_MSG_RX_COUNT,       0            ),
     DV_ADD_BOOL   ( DV_ELEM_LED_CONTROL_VAL,        false        ),
     DV_ADD_BOOL   ( DV_ELEM_LED_FEEDBACK_VAL,       false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST0,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST2,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST3,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST4,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST5,                  false        )}},

    {DV_REG_CN_TO_DN0,
    // TYPE          ELEM                           INITIAL_VAL  
    {DV_ADD_BOOL   ( DV_ELEM_TEST1,                 false         ),
     DV_ADD_UINT8  ( DV_ELEM_DEVICE_NODE_CTRL_MODE,  MODE_SAFED   )}},
};

/**
 * Device Node 1 Data Vector config. 
 */
static DataVector::Config_t gDv1Config =
{
    {DV_REG_DN1_TO_CN,
    // TYPE          ELEM                            INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_DN1_LOOP_COUNT,         0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN1_ERROR_COUNT,        0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN1_MSG_TX_COUNT,       0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN1_MSG_RX_COUNT,       0            ),
     DV_ADD_BOOL   ( DV_ELEM_LED_CONTROL_VAL,        false        ),
     DV_ADD_BOOL   ( DV_ELEM_LED_FEEDBACK_VAL,       false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST0,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST2,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST3,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST4,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST5,                  false        )}},

    {DV_REG_CN_TO_DN1,
    // TYPE          ELEM                            INITIAL_VAL  
    {DV_ADD_BOOL   ( DV_ELEM_TEST1,                  false        ),
     DV_ADD_UINT8  ( DV_ELEM_DEVICE_NODE_CTRL_MODE,  MODE_SAFED   )}},
};

/**
 * Device Node 2 Data Vector config. 
 */
static DataVector::Config_t gDv2Config =
{
    {DV_REG_DN2_TO_CN,
    // TYPE          ELEM                            INITIAL_VAL  
    {DV_ADD_UINT32 ( DV_ELEM_DN2_LOOP_COUNT,         0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN2_ERROR_COUNT,        0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN2_MSG_TX_COUNT,       0            ),
     DV_ADD_UINT32 ( DV_ELEM_DN2_MSG_RX_COUNT,       0            ),
     DV_ADD_BOOL   ( DV_ELEM_LED_CONTROL_VAL,        false        ),
     DV_ADD_BOOL   ( DV_ELEM_LED_FEEDBACK_VAL,       false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST0,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST2,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST3,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST4,                  false        ),
     DV_ADD_BOOL   ( DV_ELEM_TEST5,                  false        )}},

    {DV_REG_CN_TO_DN2,
    // TYPE          ELEM                            INITIAL_VAL  
    {DV_ADD_BOOL   ( DV_ELEM_TEST1,                  false        ),
     DV_ADD_UINT8  ( DV_ELEM_DEVICE_NODE_CTRL_MODE,  MODE_SAFED   )}},
};

/********************************* DEVICES ************************************/

/**
 * Sensor Device that sets DV_ELEM_TEST0 flag to be read by simulated Control 
 * Node Controller. Control Node then sends DV_ELEM_TEST1 flag to Device Node.
 */
class SensorADevice final : public Device
{

    public:

        /**
         * Unused. 
         */
        typedef struct Config {} Config_t;

        /**
         * Set DV_ELEM_TEST0 to true.
         */
        Error_t run ()
        {
            // Write flag to be read by Control Node Controller.
            if (mPDataVector->write (DV_ELEM_TEST0, true) != E_SUCCESS)
            {
                return E_DATA_VECTOR_WRITE;
            }

            return E_SUCCESS;
        }
    
        /**
         * Constructor.
         */
        SensorADevice (NiFpga_Session& kSession, 
                       std::shared_ptr<DataVector> kPDv,
                       Config_t& kConfig, 
                       Error_t& kRet) :
            Device (kSession, kPDv)
        {
            kRet = E_SUCCESS;
        }
};

/**
 * Sensor Device that sets DV_ELEM_TEST3 flag, which is read by 
 * DeviceNodeController, which then sets DV_ELEM_TEST4 flag.
 */
class SensorBDevice final : public Device
{

    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Set DV_ELEM_TEST3 to true.
         */
        Error_t run ()
        {
            // Write flag to be read by DeviceNodeController.
            if (mPDataVector->write (DV_ELEM_TEST3, true) != E_SUCCESS)
            {
                return E_DATA_VECTOR_WRITE;
            }

            return E_SUCCESS;
        }
    
        /**
         * Constructor.
         */
        SensorBDevice (NiFpga_Session& kSession, 
                       std::shared_ptr<DataVector> kPDv,
                       Config_t& kConfig, 
                       Error_t& kRet) :
            Device (kSession, kPDv)
        {
            kRet = E_SUCCESS;
        }
};

/**
 * Actuator Device that waits for DV_ELEM_TEST1 flag from Control Node and then
 * sets DV_ELEM_TEST2 flag.
 */
class ActuatorADevice final : public Device
{

    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Wait for DV_ELEM_TEST1 flag and then set DV_ELEM_TEST2 flag to true.
         */
        Error_t run ()
        {
            // Exit loop after send DV_ELEM_TEST2 ack to Control Node.
            static bool exit = false;
            if (exit == true)
            {
                pthread_exit ((void*) E_SUCCESS);
            }

            bool flag = false;
            if (mPDataVector->read (DV_ELEM_TEST1, flag) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            if (flag == true)
            {
                // Write ack flag.
                if (mPDataVector->write (DV_ELEM_TEST2, true) != E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }
  
                exit = true;
            }

            return E_SUCCESS;
        }
    
        /**
         * Constructor.
         */
        ActuatorADevice (NiFpga_Session& kSession, 
                       std::shared_ptr<DataVector> kPDv,
                       Config_t& kConfig, 
                       Error_t& kRet) :
            Device (kSession, kPDv)
        {
            kRet = E_SUCCESS;
        }
};

/**
 * Actuator Device that waits for DV_ELEM_TEST4 flag from DeviceNodeController
 * and then sets DV_ELEM_TEST5 flag.
 */
class ActuatorBDevice final : public Device
{

    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Wait for DV_ELEM_TEST4 flag and then set DV_ELEM_TEST5 flag to true.
         */
        Error_t run ()
        {
            bool flag = false;
            if (mPDataVector->read (DV_ELEM_TEST4, flag) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            if (flag == true)
            {
                // Write ack flag.
                if (mPDataVector->write (DV_ELEM_TEST5, true) != E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }
            }

            return E_SUCCESS;
        }

        /**
         * Constructor.
         */
        ActuatorBDevice (NiFpga_Session& kSession, 
                       std::shared_ptr<DataVector> kPDv,
                       Config_t& kConfig, 
                       Error_t& kRet) :
            Device (kSession, kPDv)
        {
            kRet = E_SUCCESS;
        }
};

/******************************* CONTROLLERS **********************************/

/**
 * Controller to run on Device Node. Reads DV_ELEM_TEST3 and sets DV_ELEM_TEST4.
 * DV_ELEM_TEST4 is then read by ActuatorBDevice, which ackowledges by setting
 * DV_ELEM_TEST5.
 */
class DeviceNodeController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * If DV_ELEM_TEST3 flag is true, set DV_ELEM_TEST7 to true.
         */
        Error_t runEnabled ()
        {
            // Read flag.
            bool flag = false;
            if (mPDataVector->read (DV_ELEM_TEST3, flag) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            // If true, set DV_ELEM_TEST4 to true.
            if (flag == true)
            {
                if (mPDataVector->write (DV_ELEM_TEST4, (bool) true) != 
                        E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }
            }

            return E_SUCCESS;
        }

        /**
         * Return error to test error handling.
         */
        Error_t runSafed ()
        {
            return E_INVALID_ELEM;
        }

        /**
         * Do nothing.
         */
        Error_t verifyConfig ()
        {
            return E_SUCCESS;
        }

        DeviceNodeController (Config_t kConfig,
                              std::shared_ptr<DataVector> kPDv,
                              DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem) {}
};

/**
 * Successful initialization function.
 */
static Error_t initializeCtrlsAndDevsSuccess (
                  std::shared_ptr<DataVector> kPDv,
                  NiFpga_Session& kFpgaSession,
                  std::vector<std::unique_ptr<Controller>>& kPCtlrsVec,
                  std::vector<std::unique_ptr<Device>>& kPSensorDevs,
                  std::vector<std::unique_ptr<Device>>& kPActuatorDevs)
{
    // Init Controller.
    std::unique_ptr<DeviceNodeController> pDeviceNodeCtrlr;
    DeviceNodeController::Config_t deviceNodeCtrlConfig = {};
    CHECK_SUCCESS (Controller::createNew (deviceNodeCtrlConfig, kPDv, 
                                          DV_ELEM_DEVICE_NODE_CTRL_MODE, 
                                          pDeviceNodeCtrlr));

    // Init Devices.
    std::unique_ptr<SensorADevice> pSensorADev;
    SensorADevice::Config_t sensorACtrlConfig = {};
    CHECK_SUCCESS (Device::createNew (kFpgaSession, kPDv, sensorACtrlConfig,
                                      pSensorADev));
    std::unique_ptr<SensorBDevice> pSensorBDev;
    SensorBDevice::Config_t sensorBCtrlConfig = {};
    CHECK_SUCCESS (Device::createNew (kFpgaSession, kPDv, sensorBCtrlConfig,
                                      pSensorBDev));
    std::unique_ptr<ActuatorADevice> pActuatorADev;
    ActuatorADevice::Config_t actuatorACtrlConfig = {};
    CHECK_SUCCESS (Device::createNew (kFpgaSession, kPDv, actuatorACtrlConfig,
                                      pActuatorADev));
    std::unique_ptr<ActuatorBDevice> pActuatorBDev;
    ActuatorBDevice::Config_t actuatorBCtrlConfig = {};
    CHECK_SUCCESS (Device::createNew (kFpgaSession, kPDv, actuatorBCtrlConfig,
                                      pActuatorBDev));

    // Init one DigitalOutDevice to verify fpga functioning.
    std::unique_ptr<DigitalOutDevice> pDigitalOutDev;
    DigitalOutDevice::Config_t digitalOutConfig = {DV_ELEM_LED_CONTROL_VAL,
                                                   DV_ELEM_LED_FEEDBACK_VAL,
                                                   5};
    CHECK_SUCCESS (Device::createNew (kFpgaSession, kPDv, digitalOutConfig,
                                      pDigitalOutDev));

    // Add controller to return vector. std::move necessary since controllers 
    // are stored in unique_ptr. Move transfers ownership of the object.
    kPCtlrsVec.push_back (std::move (pDeviceNodeCtrlr));

    // Add sensor devices.
    kPSensorDevs.push_back (std::move (pSensorADev));
    kPSensorDevs.push_back (std::move (pSensorBDev));

    // Add actuator devices.
    kPActuatorDevs.push_back (std::move (pActuatorADev));
    kPActuatorDevs.push_back (std::move (pActuatorBDev));
    kPActuatorDevs.push_back (std::move (pDigitalOutDev));
    
    return E_SUCCESS;
}
static DeviceNode::fInitializeCtrlsAndDevs_t gFInitCtrlsDevsS = 
    (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevsSuccess;

/**
 * Failing initialization function.
 */
static Error_t initializeCtrlsAndDevsFail (
                  std::shared_ptr<DataVector> kPDv,
                  NiFpga_Session& kFpgaSession,
                  std::vector<std::unique_ptr<Controller>>& kPCtlrsVec,
                  std::vector<std::unique_ptr<Device>>& kPSensorDevs,
                  std::vector<std::unique_ptr<Device>>& kPActuatorDevs)
{
    return E_INVALID_ELEM;
}
static DeviceNode::fInitializeCtrlsAndDevs_t gFInitCtrlsDevsF = 
    (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevsFail;

/************************ CONTROL NODE SIMULATION *****************************/

/**
 * Global to store Control Node Data Vector.
 */
static std::shared_ptr<DataVector> gPCnDv = nullptr;

/**
 * Number of loops executed by sim thread. Used to determine how many tx/rx
 * msgs to expect in final telemetry snapshot.
 */
static uint32_t gNumSimLoops = 0;

/**
 * Function args for node simulation thread. If enterLoop is true, thread will 
 * simulate Control Node loop. 
 */
typedef struct FuncArgs
{
    bool clockSync;
    bool enterLoop;
    Node_t deviceNode;
    DataVector::Config_t dvConfig;
    DataVectorRegion_t recvReg;
    DataVectorRegion_t sendReg;
} FuncArgs_t;

/**
 * Node simulation thread. Simulates clock synchronization and Control Node 
 * tx'ing/rx'ing data with Device Node.
 */
static void* fControlNodeSim (void* _args)
{
    // Cast args struct.
    FuncArgs_t args = *((FuncArgs_t*) _args);

    // Add CN region.
    DataVector::Config_t dvConfig = args.dvConfig;
    DataVector::RegionConfig_t regConfig =
    {
        DV_REG_CN,
        {DV_ADD_UINT32 ( DV_ELEM_CN_MSG_TX_COUNT, 0),
         DV_ADD_UINT32 ( DV_ELEM_CN_MSG_RX_COUNT, 0)}
    };
    dvConfig.push_back (regConfig);

    // Init DV. Use the same for all simulated Device Nodes.
    CHECK_SUCCESS (DataVector::createNew (dvConfig, gPCnDv));

    // Create NM.
    NetworkManager::Config_t cnConfig = 
    {
        gNodeToIp,
        gChannels,
        NODE_CONTROL,
        DV_ELEM_CN_MSG_TX_COUNT,
        DV_ELEM_CN_MSG_RX_COUNT,
    };
    std::shared_ptr<NetworkManager> pCnNm = nullptr;
    CHECK_SUCCESS (NetworkManager::createNew (cnConfig, gPCnDv, pCnNm));

    // Sleep to allow Device Node processes to start before sending any msgs.
    usleep (100 * Time::US_IN_MS);

    if (args.clockSync == true)
    {
        // Send clock sync SERVER_READY messages.
        std::vector<uint8_t> msg = {ClockSync::Msg_t::SERVER_READY};
        CHECK_SUCCESS (pCnNm->send (args.deviceNode, msg));
    }

    // Initialize tx/rx buffers.
    uint32_t recvBufSize = 0;                                                 
    uint32_t sendBufSize = 0;                                                 
    CHECK_SUCCESS (gPCnDv->getRegionSizeBytes (args.recvReg, recvBufSize));
    CHECK_SUCCESS (gPCnDv->getRegionSizeBytes (args.sendReg, sendBufSize));
    std::vector<uint8_t> recvBuf (recvBufSize);
    std::vector<uint8_t> sendBuf (sendBufSize);

    // Simulate Control Node loop.
    if (args.enterLoop == true)
    {
        while (1)
        {
            // Send region to DN.
            CHECK_SUCCESS (gPCnDv->readRegion (args.sendReg, sendBuf));
            CHECK_SUCCESS (pCnNm->send (args.deviceNode, sendBuf));

            // Receive data from DN and store in gPCnDv.
            CHECK_SUCCESS (pCnNm->recvBlock (args.deviceNode, recvBuf));
            CHECK_SUCCESS (gPCnDv->writeRegion (args.recvReg, recvBuf));

            // Enable DeviceNodeController.
            CHECK_SUCCESS (gPCnDv->write (DV_ELEM_DEVICE_NODE_CTRL_MODE,
                                          (uint8_t) MODE_ENABLED));

            // If DV_ELEM_TEST0 is true, set DV_ELEM_TEST1.
            bool flag = false;
            CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST0, flag));
            CHECK_SUCCESS (gPCnDv->write (DV_ELEM_TEST1, flag));

            // Increment sim loop counter.
            gNumSimLoops++;

            // Break once receive DV_ELEM_TEST2 flag.
            bool exitFlag = false;
            CHECK_SUCCESS (gPCnDv->read (DV_ELEM_TEST2, exitFlag));
            if (exitFlag == true)
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
ThreadManager::ThreadFunc_t gFControlNodeSim = 
    (ThreadManager::ThreadFunc_t) fControlNodeSim;

/********************************* TESTS **************************************/

TEST_GROUP (DeviceNode)
{
    /** 
     * Clear global memory and kill ntpd.
     */
    void teardown ()
    {
        std::system ("/etc/init.d/ntpd stop > /dev/null 2>&1");
        gPCnDv.reset ();
        gNumSimLoops = 0;
    }
};

/* Test entry with a bad DV config that does not contain a required region. */
TEST (DeviceNode, BadDvConfigMissingRequiredRegion)
{
    // Remove a required region.
    DataVector::Config_t dv0Config = gDv0Config;
    DataVector::Config_t dv1Config = gDv1Config;
    DataVector::Config_t dv2Config = gDv2Config;
    dv0Config.erase (dv0Config.begin ());
    dv1Config.erase (dv1Config.begin ());
    dv2Config.erase (dv2Config.begin ());

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNm0Config, dv0Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (gNm1Config, dv1Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (gNm2Config, dv2Config, gFInitCtrlsDevsS, false);
};

/* Test entry with a bad DV config that does not contain a required elem. */ 
TEST (DeviceNode, BadDvConfigMissingRequiredElem)
{
    // Remove a required element.
    DataVector::Config_t dv0Config = gDv0Config;
    DataVector::Config_t dv1Config = gDv1Config;
    DataVector::Config_t dv2Config = gDv2Config;
    dv0Config[0].elems.erase (dv0Config[0].elems.begin ());
    dv1Config[0].elems.erase (dv1Config[0].elems.begin ());
    dv2Config[0].elems.erase (dv2Config[0].elems.begin ());

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNm0Config, dv0Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (gNm1Config, dv1Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (gNm2Config, dv2Config, gFInitCtrlsDevsS, false);
};

/* Test entry with a bad DV config. */
TEST (DeviceNode, BadDvConfig)
{
    // Keep required regions and elements to pass entry () checks, but add a 
    // dupe region to fail DV checks.
    DataVector::Config_t dv0Config = gDv0Config;
    DataVector::Config_t dv1Config = gDv1Config;
    DataVector::Config_t dv2Config = gDv2Config;
    DataVector::RegionConfig_t dupeRegion = {DV_REG_CN, {}};
    dv0Config.push_back (dupeRegion);
    dv0Config.push_back (dupeRegion);
    dv1Config.push_back (dupeRegion);
    dv1Config.push_back (dupeRegion);
    dv2Config.push_back (dupeRegion);
    dv2Config.push_back (dupeRegion);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNm0Config, dv0Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (gNm1Config, dv1Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (gNm2Config, dv2Config, gFInitCtrlsDevsS, false);
};

/* Test entry with a NM config with a me node that is not a Device Node. */
TEST (DeviceNode, BadNmConfigNotDn)
{
    // Remove a required node.
    NetworkManager::Config_t nmConfig = gNm0Config;
    nmConfig.me = NODE_CONTROL;

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nmConfig, gDv0Config, gFInitCtrlsDevsS, false);
};

/* Test entry with a bad NM config that does not contain all required nodes. No 
   need to init clock sync thread since NM initialized pre clock sync. */
TEST (DeviceNode, BadNmConfigMissingNode)
{
    // Remove a required node (me).
    NetworkManager::Config_t nm0Config = gNm0Config;
    NetworkManager::Config_t nm1Config = gNm1Config;
    NetworkManager::Config_t nm2Config = gNm2Config;
    nm0Config.nodeToIp.erase (NODE_DEVICE0);
    nm1Config.nodeToIp.erase (NODE_DEVICE1);
    nm2Config.nodeToIp.erase (NODE_DEVICE2);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nm0Config, gDv0Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (nm1Config, gDv1Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (nm2Config, gDv2Config, gFInitCtrlsDevsS, false);
};

/* Test entry with a bad NM config that does not contain all required 
   channels. */
TEST (DeviceNode, BadNmConfigMissingChannel)
{
    // Remove a required channel (me <--> control node).
    NetworkManager::Config_t nm0Config = gNm0Config;
    NetworkManager::Config_t nm1Config = gNm1Config;
    NetworkManager::Config_t nm2Config = gNm2Config;
    nm0Config.channels.erase (nm0Config.channels.begin ());
    nm1Config.channels.erase (nm1Config.channels.begin () + 1);
    nm2Config.channels.erase (nm2Config.channels.begin () + 2);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nm0Config, gDv0Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (nm1Config, gDv1Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (nm2Config, gDv2Config, gFInitCtrlsDevsS, false);
};

/* Test entry with a bad NM config. */
TEST (DeviceNode, BadNmConfig)
{
    // Keep required nodes and channels to pass entry () checks, but use a dupe 
    // IP to fail NM checks.
    // Remove a required channel (me <--> control node).
    NetworkManager::Config_t nm0Config = gNm0Config;
    NetworkManager::Config_t nm1Config = gNm1Config;
    NetworkManager::Config_t nm2Config = gNm2Config;
    nm0Config.nodeToIp[NODE_DEVICE0] = "127.0.0.1";
    nm1Config.nodeToIp[NODE_DEVICE1] = "127.0.0.1";
    nm2Config.nodeToIp[NODE_DEVICE2] = "127.0.0.1";

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (nm0Config, gDv0Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (nm1Config, gDv1Config, gFInitCtrlsDevsS, false);
    TEST_ENTRY_EXIT_ON_ERROR (nm2Config, gDv2Config, gFInitCtrlsDevsS, false);
};

/* Test entry with failed clock sync. */
TEST (DeviceNode, ClockSyncFail0)
{
    // Create threads to simulate Control Node during clock sync.
    CREATE_SIM_THREAD (true, false, NODE_DEVICE0, gDv0Config, DV_REG_DN0_TO_CN, 
                       DV_REG_CN_TO_DN0);

    // Create DN processes. Expect them to run and block in Clock Sync step 
    // when main thread sleeps.
    TEST_ENTRY_EXIT_ON_ERROR (gNm0Config, gDv0Config, gFInitCtrlsDevsS, false);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with failed clock sync. */
TEST (DeviceNode, ClockSyncFail1)
{
    // Create threads to simulate Control Node during clock sync.
    CREATE_SIM_THREAD (true, false, NODE_DEVICE1, gDv1Config, DV_REG_DN1_TO_CN, 
                       DV_REG_CN_TO_DN1);

    // Create DN processes. Expect them to run and block in Clock Sync step 
    // when main thread sleeps.
    TEST_ENTRY_EXIT_ON_ERROR (gNm1Config, gDv1Config, gFInitCtrlsDevsS, false);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with failed clock sync. */
TEST (DeviceNode, ClockSyncFail2)
{
    // Create threads to simulate Control Node during clock sync.
    CREATE_SIM_THREAD (true, false, NODE_DEVICE2, gDv2Config, DV_REG_DN2_TO_CN, 
                       DV_REG_CN_TO_DN2);

    // Create DN processes. Expect them to run and block in Clock Sync step 
    // when main thread sleeps.
    TEST_ENTRY_EXIT_ON_ERROR (gNm2Config, gDv2Config, gFInitCtrlsDevsS, false);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with a error on controller initializations.. */
TEST (DeviceNode, BadControllerInit0)
{
    // Create thread to simulate Control Node during clock sync. 
    CREATE_SIM_THREAD (false, false, NODE_DEVICE0, gDv0Config, DV_REG_DN0_TO_CN, 
                       DV_REG_CN_TO_DN0);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNm0Config, gDv0Config, gFInitCtrlsDevsF, true);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with a error on controller initializations.. */
TEST (DeviceNode, BadControllerInit1)
{
    // Create thread to simulate Control Node during clock sync. 
    CREATE_SIM_THREAD (false, false, NODE_DEVICE1, gDv1Config, DV_REG_DN1_TO_CN, 
                       DV_REG_CN_TO_DN1);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNm1Config, gDv1Config, gFInitCtrlsDevsF, true);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test entry with a error on controller initializations.. */
TEST (DeviceNode, BadControllerInit2)
{
    // Create thread to simulate Control Node during clock sync. 
    CREATE_SIM_THREAD (false, false, NODE_DEVICE2, gDv2Config, DV_REG_DN2_TO_CN, 
                       DV_REG_CN_TO_DN2);

    // Create process that calls entry. Expect this process to exit due to a bad
    // config.
    TEST_ENTRY_EXIT_ON_ERROR (gNm2Config, gDv2Config, gFInitCtrlsDevsF, true);

    // Wait for node sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);
};

/* Test running through Device Node loops successfuly. */
TEST (DeviceNode, Success0)
{
    // Create thread to simulate Control Node during clock sync. 
    CREATE_SIM_THREAD (false, true, NODE_DEVICE0, gDv0Config, DV_REG_DN0_TO_CN, 
                       DV_REG_CN_TO_DN0);

    // Create process that calls entry. Expect this process to exit once the
    // ActuatorADevice sends an ack to the "Control Node".
    TEST_ENTRY_EXIT_ON_ERROR (gNm0Config, gDv0Config, gFInitCtrlsDevsS, true);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);

    CHECK_DV (DV_ELEM_DN0_ERROR_COUNT, DV_ELEM_DN0_MSG_TX_COUNT, 
              DV_ELEM_DN0_MSG_RX_COUNT);
};

/* Test running through Device Node loops successfuly. */
TEST (DeviceNode, Success1)
{
    // Create thread to simulate Control Node during clock sync. 
    CREATE_SIM_THREAD (false, true, NODE_DEVICE1, gDv1Config, DV_REG_DN1_TO_CN, 
                       DV_REG_CN_TO_DN1);

    // Create process that calls entry. Expect this process to exit once the
    // ActuatorADevice sends an ack to the "Control Node".
    TEST_ENTRY_EXIT_ON_ERROR (gNm1Config, gDv1Config, gFInitCtrlsDevsS, true);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);

    CHECK_DV (DV_ELEM_DN1_ERROR_COUNT, DV_ELEM_DN1_MSG_TX_COUNT, 
              DV_ELEM_DN1_MSG_RX_COUNT);
};

/* Test running through Device Node loops successfuly. */
TEST (DeviceNode, Success2)
{
    // Create thread to simulate Control Node during clock sync. 
    CREATE_SIM_THREAD (false, true, NODE_DEVICE2, gDv2Config, DV_REG_DN2_TO_CN, 
                       DV_REG_CN_TO_DN2);

    // Create process that calls entry. Expect this process to exit once the
    // ActuatorADevice sends an ack to the "Control Node".
    TEST_ENTRY_EXIT_ON_ERROR (gNm2Config, gDv2Config, gFInitCtrlsDevsS, true);

    // Wait for sim thread.
    Error_t ret;
    WAIT_FOR_THREAD (thread, pTm);

    CHECK_DV (DV_ELEM_DN2_ERROR_COUNT, DV_ELEM_DN2_MSG_TX_COUNT, 
              DV_ELEM_DN2_MSG_RX_COUNT);
};
