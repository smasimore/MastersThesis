
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

# ifndef ERRORS_H
# define ERRORS_H

typedef enum Error
{

    /* General */
    E_SUCCESS = 0,
    E_NONFINITE_VALUE,
    E_OUT_OF_BOUNDS,
    E_OVERFLOW,
    E_INVALID_ENUM,

    /* Test Log */
    E_FAILED_TO_INIT_LOCK,
    E_FAILED_TO_LOCK,
    E_FAILED_TO_UNLOCK,

    /* Testing */
    E_TEST_ERROR,

    /* Thread Manager */
    E_INVALID_PRIORITY = 25,
    E_INVALID_POINTER,
    E_INVALID_AFFINITY,
    E_INVALID_ARGS_LENGTH,
    E_FAILED_TO_INIT_KERNEL_ENV,
    E_FAILED_TO_VERIFY_PROCESS,
    E_FAILED_TO_OPEN_FILE,
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

    /* Network Interface */
    E_SOCKET_NOT_INITIALIZED = 50,
    E_FAILED_TO_CREATE_SOCKET,
    E_FAILED_TO_ALLOCATE_SOCKET,
    E_FAILED_TO_BIND_TO_SOCKET,
    E_FAILED_TO_CLOSE_SOCKET,
    E_FAILED_TO_SEND_DATA,
    E_FAILED_TO_RECV_DATA,
    E_PARTIAL_SEND,
    E_PARTIAL_RECV,
    E_RECV_TRUNC,
    E_INVALID_SRC_ADDR,
    E_INVALID_BUF_LEN,
    E_WOULD_BLOCK,

    /* State Machine */
    E_INVALID_TRANSITION = 75,
    E_DUPLICATE_NAME,
    E_NAME_NOTFOUND,
    E_NO_STATES,

    /* State Vector */
    E_EMPTY_CONFIG = 100,
    E_EMPTY_ELEMS,
    E_DUPLICATE_REGION,
    E_DUPLICATE_ELEM,
    E_INVALID_REGION,
    E_INVALID_ELEM,
    E_INVALID_TYPE,
    E_INCORRECT_TYPE,

    E_LAST
} Error_t;

#endif
