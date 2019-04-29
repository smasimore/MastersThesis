
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
    E_FAILED_TO_UNLOCK
} Error_t;

#endif
