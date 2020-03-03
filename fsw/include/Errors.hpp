
/**
 ************************ Error Handling Framework *****************************
 *
 * In order to surface and handle errors in a standard way, all flight software
 * must follow the protocol established by the Error Handling Framework:
 *
 * 1. All functions must return a type Error_t.
 * 2. All callsite must check the return value of the function called.
 *
 *******************************************************************************
 */

# ifndef ERRORS_HPP
# define ERRORS_HPP

#include <cstdint>
#include <string>

enum Error_t : uint32_t
{

    /* General */
    E_SUCCESS = 0,
    E_NONFINITE_VALUE,
    E_OUT_OF_BOUNDS,
    E_OVERFLOW,
    E_INVALID_ENUM,
    E_DATA_VECTOR_NULL,
    E_DATA_VECTOR_READ,
    E_DATA_VECTOR_WRITE,
    E_INVALID_ELEM,
    E_FAILED_TO_OPEN_FILE,

    /* Testing */
    E_TEST_ERROR,

    /* Thread Manager */
    E_INVALID_PRIORITY = 25,
    E_INVALID_POINTER,
    E_INVALID_AFFINITY,
    E_INVALID_ARGS_LENGTH,
    E_FAILED_TO_INIT_KERNEL_ENV,
    E_FAILED_TO_VERIFY_PROCESS,
    E_FAILED_TO_READ_FILE,
    E_FAILED_TO_CLOSE_FILE,
    E_FAILED_TO_ALLOCATE_ARGS,
    E_FAILED_TO_ALLOCATE_THREAD,
    E_FAILED_TO_INIT_THREAD_ATR,
    E_FAILED_TO_SET_SCHED_POL,
    E_FAILED_TO_SET_PRIORITY,
    E_FAILED_TO_SET_AFFINITY,
    E_FAILED_TO_SET_THREAD_PRI,
    E_FAILED_TO_SET_SCHED_INH,
    E_FAILED_TO_CREATE_THREAD,
    E_FAILED_TO_DESTROY_THREAD_ATTR,
    E_FAILED_TO_WAIT_ON_THREAD,
    E_THREAD_NOT_FOUND,
    E_FAILED_TO_CREATE_TIMERFD,
    E_FAILED_TO_ARM_TIMERFD,
    E_FAILED_TO_GET_TIMER_FLAGS,
    E_FAILED_TO_SET_TIMER_FLAGS,
    E_FAILED_TO_READ_TIMERFD,
    E_MISSED_SCHEDULER_DEADLINE,

    /* Network Manager */
    E_EMPTY_NODE_CONFIG = 50,
    E_EMPTY_CHANNEL_CONFIG,
    E_NON_NUMERIC_IP,
    E_INVALID_IP_REGION,
    E_INVALID_IP_SIZE,
    E_INVALID_PORT,
    E_INVALID_NODE,
    E_UNDEFINED_NODE_IN_CHANNEL,
    E_UNDEFINED_ME_NODE,
    E_DUPLICATE_IP,
    E_DUPLICATE_CHANNEL,
    E_FAILED_TO_CREATE_SOCKET,
    E_FAILED_TO_SET_SOCKET_OPTIONS,
    E_FAILED_TO_BIND_TO_SOCKET,
    E_EMPTY_BUFFER,
    E_FAILED_TO_SEND_MSG,
    E_FAILED_TO_RECV_MSG,
    E_UNEXPECTED_SEND_SIZE,
    E_UNEXPECTED_RECV_SIZE,
    E_VECTORS_DIFF_SIZES,
    E_TIMEOUT_TOO_LARGE,
    E_SELECT_FAILED,

    /* State Machine */
    E_INVALID_TRANSITION = 100,
    E_DUPLICATE_NAME,
    E_NAME_NOTFOUND,
    E_NO_STATES,

    /* Data Vector */
    E_EMPTY_CONFIG = 125,
    E_EMPTY_ELEMS,
    E_DUPLICATE_REGION,
    E_DUPLICATE_ELEM,
    E_INVALID_REGION,
    E_INVALID_TYPE,
    E_INCORRECT_TYPE,
    E_INCORRECT_SIZE,
    E_FAILED_TO_INIT_LOCK,
    E_FAILED_TO_LOCK,
    E_FAILED_TO_UNLOCK,
    E_FAILED_TO_READ_AND_UNLOCK,
    E_FAILED_TO_WRITE_AND_UNLOCK,
    E_ENUM_STRING_UNDEFINED,

    /* Data Vector Logger */
    E_FAILED_TO_WRITE_FILE,
    E_FAILED_TO_SEEK,

    /* Devices */
    E_FPGA_INIT = 150,
    E_FPGA_READ,
    E_FPGA_WRITE,
	E_FPGA_NO_SESSION,
    E_PIN_NOT_CONFIGURED,

    /* Time */
    E_FAILED_TO_GET_TIME = 175,
    E_FAILED_TO_INIT_TIME,
    E_OVERFLOW_IMMINENT,

    /* Scripts */
    E_WRONG_ARGC,
    E_INVALID_ARGUMENT,
    E_FAILED_TO_CANCEL_ABORT,

    E_LAST
};

namespace Errors
{

    /**
     * Exit process if kError is anything but E_SUCCESS. This function does not
     * return an Error_t as it is intended to handle other errors in the system
     * and has no error state of its own.
     *
     * @param  kError  Error_t variable to check.
     * @param  kMsg    Message to print on error.
     */
    void exitOnError (Error_t kError, std::string kMsg);
}

#endif
