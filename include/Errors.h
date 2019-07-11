
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

typedef enum Error {

    /* General */
    E_SUCCESS,

    /* Test Log */
    E_INVALID_ENUM,
    E_FAILED_TO_INIT_LOCK,
    E_FAILED_TO_LOCK,
    E_FAILED_TO_UNLOCK,

    /* Thread Manager */
    E_INVALID_PRIORITY,
    E_INVALID_POINTER,
    E_INVALID_AFFINITY,
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

    /* Network Interface */
    E_SOCKET_NOT_INITIALIZED,
    E_FAILED_TO_CREATE_SOCKET,
    E_FAILED_TO_ALLOCATE_SOCKET,
    E_FAILED_TO_BIND_TO_SOCKET,
    

    /* State Machine */
    E_INVALID_TRANSITION,
    E_DUPLICATE_NAME,
    E_NAME_NOTFOUND,

    LAST

} Error_t;

#endif
