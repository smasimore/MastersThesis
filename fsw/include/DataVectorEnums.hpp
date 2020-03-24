/**
 * Enumerations for DataVector. Defined in the global namespace so the enums 
 * names can be shorter (i.e. w/o namespace) throughout configs and flight 
 * software.
 */

#ifndef DATA_VECTOR_ENUMS_HPP
#define DATA_VECTOR_ENUMS_HPP

#include <stdint.h>

/**
 * Data Vector element type enumeration.
 */
enum DataVectorElementType_t : uint8_t
{
    DV_T_UINT8,
    DV_T_UINT16,
    DV_T_UINT32,
    DV_T_UINT64,
    DV_T_INT8,
    DV_T_INT16,
    DV_T_INT32,
    DV_T_INT64,
    DV_T_FLOAT,
    DV_T_DOUBLE,
    DV_T_BOOL,

    DV_T_LAST
};

/**
 * Possible regions in Data Vector.
 */
enum DataVectorRegion_t : uint32_t
{

///////////////////////////////// TEST REGIONS /////////////////////////////////

    /* Test Regions */
    DV_REG_TEST0,
    DV_REG_TEST1,
    DV_REG_TEST2,

///////////////////////////////// FSW REGIONS //////////////////////////////////

    DV_REG_CN,
    DV_REG_DN0_TO_CN,
    DV_REG_DN1_TO_CN,
    DV_REG_DN2_TO_CN,
    DV_REG_GROUND_TO_CN,
    DV_REG_CN_TO_DN0,
    DV_REG_CN_TO_DN1,
    DV_REG_CN_TO_DN2,

    DV_REG_LAST
};

/**
 * Possible elements in Data Vector.
 */
enum DataVectorElement_t : uint32_t
{

///////////////////////////////// TEST ELEMENTS ////////////////////////////////

    /* Generic Test Elements */
    DV_ELEM_TEST0,
    DV_ELEM_TEST1,
    DV_ELEM_TEST2,
    DV_ELEM_TEST3,
    DV_ELEM_TEST4,
    DV_ELEM_TEST5,
    DV_ELEM_TEST6,
    DV_ELEM_TEST7,
    DV_ELEM_TEST8,
    DV_ELEM_TEST9,
    DV_ELEM_TEST10,
    DV_ELEM_TEST11,
    DV_ELEM_TEST12,
    DV_ELEM_TEST13,
    DV_ELEM_TEST14,
    DV_ELEM_TEST15,
    DV_ELEM_TEST16,
    DV_ELEM_TEST17,
    DV_ELEM_TEST18,
    DV_ELEM_TEST19,
    DV_ELEM_TEST20,
    DV_ELEM_TEST21,
    DV_ELEM_TEST22,
    DV_ELEM_TEST23,
    DV_ELEM_TEST24,
    DV_ELEM_TEST25,
    DV_ELEM_TEST26,
    DV_ELEM_TEST27,
    DV_ELEM_TEST28,
    DV_ELEM_TEST29,
    DV_ELEM_TEST30,
    DV_ELEM_TEST31,
    DV_ELEM_TEST32,
    DV_ELEM_TEST33,
    DV_ELEM_TEST34,
    DV_ELEM_TEST35,
    DV_ELEM_TEST36,
    DV_ELEM_TEST37,
    DV_ELEM_TEST38,
    DV_ELEM_TEST39,
    DV_ELEM_TEST40,
    DV_ELEM_TEST41,
    DV_ELEM_TEST42,
    DV_ELEM_TEST43,
    DV_ELEM_TEST44,
    DV_ELEM_TEST45,
    DV_ELEM_TEST46,

    /* Test Controllers */
    DV_ELEM_TEST_CONTROLLER_MODE,
    DV_ELEM_DN_RESP_CTRL_MODE,
    DV_ELEM_ERROR_CTRL_MODE,
    DV_ELEM_THREAD_KILL_CTRL_MODE,
    DV_ELEM_DEVICE_NODE_CTRL_MODE,

    /* RCS Controller */
    DV_ELEM_RCS_CONTROLLER_MODE,

    /* LED Controller */
    DV_ELEM_LED_CONTROLLER_MODE,
    DV_ELEM_LED_CONTROL_VAL,
    DV_ELEM_LED_FEEDBACK_VAL,

    /* Recovery Igniter Test Script */
    DV_ELEM_RECIGNTEST_CONTROL_VAL,
    DV_ELEM_RECIGNTEST_FEEDBACK_VAL,

///////////////////////////////// FSW ELEMENTS /////////////////////////////////

    /* Health Counters */
    DV_ELEM_CN_LOOP_COUNT,
    DV_ELEM_CN_ERROR_COUNT,
    DV_ELEM_CN_MSG_TX_COUNT,
    DV_ELEM_CN_MSG_RX_COUNT,
    DV_ELEM_DN0_LOOP_COUNT,
    DV_ELEM_DN0_ERROR_COUNT,
    DV_ELEM_DN0_MSG_TX_COUNT,
    DV_ELEM_DN0_MSG_RX_COUNT,
    DV_ELEM_DN1_LOOP_COUNT,
    DV_ELEM_DN1_ERROR_COUNT,
    DV_ELEM_DN1_MSG_TX_COUNT,
    DV_ELEM_DN1_MSG_RX_COUNT,
    DV_ELEM_DN2_LOOP_COUNT,
    DV_ELEM_DN2_ERROR_COUNT,
    DV_ELEM_DN2_MSG_TX_COUNT,
    DV_ELEM_DN2_MSG_RX_COUNT,
    DV_ELEM_GROUND_MSG_TX_COUNT,
    DV_ELEM_GROUND_MSG_RX_COUNT,
    DV_ELEM_DN0_RX_MISS_COUNT,
    DV_ELEM_DN1_RX_MISS_COUNT,
    DV_ELEM_DN2_RX_MISS_COUNT,

    /* Time */
    DV_ELEM_CN_TIME_NS,

    /* State Machine */
    DV_ELEM_STATE,

    /* Command Handler */
    DV_ELEM_CMD_REQ,
    DV_ELEM_CMD,
    DV_ELEM_CMD_WRITE_ELEM,
    DV_ELEM_CMD_WRITE_VAL,
    DV_ELEM_LAST_CMD_REQ_NUM,
    DV_ELEM_LAST_CMD_PROC_NUM,

    DV_ELEM_LAST
};

#endif
