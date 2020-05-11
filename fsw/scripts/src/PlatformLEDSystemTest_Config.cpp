/**
 * See PlatformLEDSystemTest_Config.hpp for instructions on running test.
 */

#include "PlatformLEDSystemTest_Config.hpp"

/*************************** DATA VECTOR CONFIGS ******************************/

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegCnToDn0 =
{
    DV_REG_CN_TO_DN0,
    // TYPE           ELEM                                  INITIAL_VAL  
    {DV_ADD_BOOL    ( DV_ELEM_STATE_A_LED_CONTROL_VAL,      false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_B_LED_CONTROL_VAL,      false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_C_LED_CONTROL_VAL,      false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_D_LED_CONTROL_VAL,      false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_E_LED_CONTROL_VAL,      false        )}
};

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegCnToDn1 = 
{
    DV_REG_CN_TO_DN1,
    // TYPE          ELEM                                   INITIAL_VAL   
    {DV_ADD_BOOL    ( DV_ELEM_CN_LED0_CONTROL_VAL,          MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED1_CONTROL_VAL,          MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED2_CONTROL_VAL,          MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED3_CONTROL_VAL,          MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED4_CONTROL_VAL,          MODE_SAFED   )}
};

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegCnToDn2 = 
{
    DV_REG_CN_TO_DN2,
    // TYPE          ELEM                                   INITIAL_VAL   
    {DV_ADD_UINT8   ( DV_ELEM_DN_FLASH_LED_CTRL_MODE,       MODE_SAFED   )}
};

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegDn0ToCn = 
{
    DV_REG_DN0_TO_CN,
    // TYPE          ELEM                                   INITIAL_VAL   
    {DV_ADD_UINT32  ( DV_ELEM_DN0_MSG_TX_COUNT,             0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN0_MSG_RX_COUNT,             0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN0_LOOP_COUNT,               0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN0_ERROR_COUNT,              0            ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_A_LED_FEEDBACK_VAL,     false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_B_LED_FEEDBACK_VAL,     false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_C_LED_FEEDBACK_VAL,     false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_D_LED_FEEDBACK_VAL,     false        ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_E_LED_FEEDBACK_VAL,     false        )}
};

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegDn1ToCn = 
{
    DV_REG_DN1_TO_CN,
    // TYPE          ELEM                                   INITIAL_VAL   
    {DV_ADD_UINT32  ( DV_ELEM_DN1_MSG_TX_COUNT,             0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN1_MSG_RX_COUNT,             0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN1_LOOP_COUNT,               0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN1_ERROR_COUNT,              0            ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED0_FEEDBACK_VAL,         MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED1_FEEDBACK_VAL,         MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED2_FEEDBACK_VAL,         MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED3_FEEDBACK_VAL,         MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_CN_LED4_FEEDBACK_VAL,         MODE_SAFED   )}
};

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegDn2ToCn = 
{
    DV_REG_DN2_TO_CN,
    // TYPE          ELEM                                   INITIAL_VAL   
    {DV_ADD_UINT32  ( DV_ELEM_DN2_MSG_TX_COUNT,             0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN2_MSG_RX_COUNT,             0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN2_LOOP_COUNT,               0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN2_ERROR_COUNT,              0            ),
     DV_ADD_BOOL    ( DV_ELEM_DN_FLASH_LED_CONTROL_VAL,     0            ),
     DV_ADD_BOOL    ( DV_ELEM_DN_FLASH_LED_FEEDBACK_VAL,    0            )}
};

DataVector::RegionConfig_t PlatformLEDSystemTest_Config::mDvRegGndToCn =
{
    DV_REG_GROUND_TO_CN,
    // TYPE           ELEM                                  INITIAL_VAL  
    {DV_ADD_UINT8   ( DV_ELEM_CMD_REQ,                      CMD_NONE     ),
     DV_ADD_UINT32  ( DV_ELEM_CMD_REQ_NUM,                  0            ),
     DV_ADD_UINT32  ( DV_ELEM_CMD_WRITE_ELEM,               DV_ELEM_LAST ),
     DV_ADD_UINT64  ( DV_ELEM_CMD_WRITE_VAL,                0            )}
};

DataVector::Config_t PlatformLEDSystemTest_Config::mCnDvConfig =
{
    {DV_REG_CN,
    // TYPE          ELEM                                   INITIAL_VAL
    {DV_ADD_UINT32  ( DV_ELEM_CN_LOOP_COUNT,                0            ),
     DV_ADD_UINT32  ( DV_ELEM_CN_ERROR_COUNT,               0            ),
     DV_ADD_UINT32  ( DV_ELEM_CN_MSG_TX_COUNT,              0            ),
     DV_ADD_UINT32  ( DV_ELEM_CN_MSG_RX_COUNT,              0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN0_RX_MISS_COUNT,            0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN1_RX_MISS_COUNT,            0            ),
     DV_ADD_UINT32  ( DV_ELEM_DN2_RX_MISS_COUNT,            0            ),
     DV_ADD_UINT32  ( DV_ELEM_CN_LOOP_DEADLINE_MISS_COUNT,  0            ),
     DV_ADD_UINT32  ( DV_ELEM_CN_COMMS_DEADLINE_MISS_COUNT, 0            ),
     DV_ADD_UINT8   ( DV_ELEM_CMD,                          CMD_NONE     ),
     DV_ADD_UINT32  ( DV_ELEM_LAST_CMD_PROC_NUM,            0            ),
     DV_ADD_UINT64  ( DV_ELEM_CN_TIME_NS,                   0            ),
     DV_ADD_UINT32  ( DV_ELEM_STATE,                        STATE_A      ),
     DV_ADD_UINT8   ( DV_ELEM_STATE_LED_CTRL_MODE,          MODE_SAFED   ),
     DV_ADD_UINT8   ( DV_ELEM_CN_LED0_CTRL_MODE,            MODE_SAFED   ),
     DV_ADD_UINT8   ( DV_ELEM_CN_LED1_CTRL_MODE,            MODE_SAFED   ),
     DV_ADD_UINT8   ( DV_ELEM_CN_LED2_CTRL_MODE,            MODE_SAFED   ),
     DV_ADD_UINT8   ( DV_ELEM_CN_LED3_CTRL_MODE,            MODE_SAFED   ),
     DV_ADD_UINT8   ( DV_ELEM_CN_LED4_CTRL_MODE,            MODE_SAFED   ),
     DV_ADD_UINT8   ( DV_ELEM_THREAD_KILL_CTRL_MODE,        MODE_SAFED   ),
     DV_ADD_BOOL    ( DV_ELEM_STATE_B_TRANS_FLAG,           false        )}},

    PlatformLEDSystemTest_Config::mDvRegCnToDn0,
    PlatformLEDSystemTest_Config::mDvRegCnToDn1,
    PlatformLEDSystemTest_Config::mDvRegCnToDn2,
    PlatformLEDSystemTest_Config::mDvRegDn0ToCn,
    PlatformLEDSystemTest_Config::mDvRegDn1ToCn,
    PlatformLEDSystemTest_Config::mDvRegDn2ToCn,
    PlatformLEDSystemTest_Config::mDvRegGndToCn,
};

DataVector::Config_t PlatformLEDSystemTest_Config::mDnDvConfig =
{
#if DEVICE_NODE_TO_COMPILE == DEVICE_NODE0
    PlatformLEDSystemTest_Config::mDvRegCnToDn0,
    PlatformLEDSystemTest_Config::mDvRegDn0ToCn,
#elif DEVICE_NODE_TO_COMPILE == DEVICE_NODE1
    PlatformLEDSystemTest_Config::mDvRegCnToDn1,
    PlatformLEDSystemTest_Config::mDvRegDn1ToCn,
#else
    PlatformLEDSystemTest_Config::mDvRegCnToDn2,
    PlatformLEDSystemTest_Config::mDvRegDn2ToCn,
#endif
};

DataVector::Config_t PlatformLEDSystemTest_Config::mGndDvConfig =
{
    {DV_REG_GROUND,
    // TYPE          ELEM                                   INITIAL_VAL
    {DV_ADD_UINT32  ( DV_ELEM_GROUND_MSG_TX_COUNT,          0            ),
     DV_ADD_UINT32  ( DV_ELEM_GROUND_MSG_RX_COUNT,          0            )}},

	PlatformLEDSystemTest_Config::mDvRegGndToCn,
};

/************************* NETWORK MANAGER CONFIGS ****************************/

std::unordered_map<Node_t, 
                   NetworkManager::IP_t, 
                   EnumClassHash> PlatformLEDSystemTest_Config::mNodes = 
{
    {NODE_DEVICE0, DEVICE_NODE0_IP},
    {NODE_DEVICE1, DEVICE_NODE1_IP},
    {NODE_DEVICE2, DEVICE_NODE2_IP},
    {NODE_CONTROL, CONTROL_NODE_IP},
    {NODE_GROUND,  GROUND_NODE_IP}
};

std::vector<NetworkManager::ChannelConfig_t> 
    PlatformLEDSystemTest_Config::mChannels =
{
    {NODE_CONTROL, 
     NODE_DEVICE0, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT)},
    {NODE_CONTROL,
     NODE_DEVICE1,
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 1)},
    {NODE_CONTROL,
     NODE_DEVICE2,
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 2)},
    {NODE_CONTROL,
     NODE_GROUND, 
     static_cast<uint16_t> (NetworkManager::MIN_PORT + 3)},
};

NetworkManager::Config_t PlatformLEDSystemTest_Config::mDnNmConfig =
{
    // Nodes
    PlatformLEDSystemTest_Config::mNodes,
    // Channels
    PlatformLEDSystemTest_Config::mChannels,
    // Me
    (Node_t) DEVICE_NODE_TO_COMPILE,
    // Msg Tx and Rx Counters.
#if DEVICE_NODE_TO_COMPILE == DEVICE_NODE0
    DV_ELEM_DN0_MSG_TX_COUNT,
    DV_ELEM_DN0_MSG_RX_COUNT,
#elif DEVICE_NODE_TO_COMPILE == DEVICE_NODE1
    DV_ELEM_DN1_MSG_TX_COUNT,
    DV_ELEM_DN1_MSG_RX_COUNT,
#else
    DV_ELEM_DN2_MSG_TX_COUNT,
    DV_ELEM_DN2_MSG_RX_COUNT,
#endif
};

NetworkManager::Config_t PlatformLEDSystemTest_Config::mCnNmConfig =
{
    // Nodes
    PlatformLEDSystemTest_Config::mNodes,
    // Channels
    PlatformLEDSystemTest_Config::mChannels,
    // Me
    NODE_CONTROL,
    // Msg Tx and Rx Counters.
    DV_ELEM_CN_MSG_TX_COUNT,
    DV_ELEM_CN_MSG_RX_COUNT,
};

NetworkManager::Config_t PlatformLEDSystemTest_Config::mGndNmConfig =
{
    // Nodes
    PlatformLEDSystemTest_Config::mNodes,
    // Channels
    PlatformLEDSystemTest_Config::mChannels,
    // Me
    NODE_GROUND,
    // Msg Tx and Rx Counters.
    DV_ELEM_GROUND_MSG_TX_COUNT,
    DV_ELEM_GROUND_MSG_RX_COUNT,
};

/************************** COMMAND HANDLER CONFIG ****************************/

CommandHandler::Config_t PlatformLEDSystemTest_Config::mChConfig =
{
    DV_ELEM_CMD,
    DV_ELEM_CMD_REQ,
    DV_ELEM_CMD_WRITE_ELEM,
    DV_ELEM_CMD_WRITE_VAL,
    DV_ELEM_CMD_REQ_NUM,    
    DV_ELEM_LAST_CMD_PROC_NUM,
};
