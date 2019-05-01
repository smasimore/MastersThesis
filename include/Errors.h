
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
    E_SUCCESS,
    E_INVALID_ENUM,
    E_FAILED_TO_INIT_LOCK,
    E_FAILED_TO_LOCK,
    E_FAILED_TO_UNLOCK,
    E_FAILED_TO_INIT_KERNEL_ENV,
    E_FAILED_TO_VERIFY_PROCESS,
    E_FAILED_TO_OPEN_FILE,
    E_FAILED_TO_READ_FILE,
    E_FAILED_TO_CLOSE_FILE,
    E_FAILED_TO_GET_THREAD_NAME,
    E_NAME_OF_THREAD_DNE_EXPECTED
} Error_t;

#endif
