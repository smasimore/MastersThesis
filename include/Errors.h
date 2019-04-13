
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
    E_INVALID_PARAM
} Error_t;

#endif
