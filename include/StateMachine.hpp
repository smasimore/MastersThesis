/**
 * StateMachine struct to manage states aboard the flight computer.
 * Prove basic skeleton functionality of data and initialization.
 */

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include "Errors.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

class StateMachine
{
public:

    /**
     * Create a statemachine from a default hardcoded case. This is important
     * in case of parser, config, or other external failures.
     *
     * @ret StateMachine struct from private constructor
     */
    static Error_t fromDefault (StateMachine **ppStateMachine);

    /**
     * Create a statemachine from data in an array. This is a placeholder
     * function to demonstrate creation from user-defined data.
     *
     * @param   c[] array of int32_t, an arbitrary data type
     *
     * @ret     StateMachine struct from private constructor and parameter data
     */
    static Error_t fromArr (StateMachine **ppStateMachine, int32_t c[]);

    /**
     * Print the data in the StateMachine (skeleton data in this case)
     *
     * @ret     E_SUCCESS   Printing successful
     */
    Error_t printData ();

    /**
     * Returns the value of temporary StateMachine data A
     *
     * @param   result      Reference to int32_t to store value of A
     *
     * @ret     E_SUCCESS   value of A succesfully stored in result
     */
    Error_t getA (int32_t &result);

    /**
     * Returns the value of temporary StateMachine data B
     *
     * @param   result      Reference to int32_t to store value of B
     *
     * @ret     E_SUCCESS   value of B succesfully stored in result
     */
    Error_t getB (int32_t &result);

private:

    /**
     * Private constructor to ensure named constructor idoims are used
     */
    StateMachine (int32_t a, int32_t b);

    /**
     * Placeholder skeleton data for temporary testing purposes only
     */
    int32_t a;
    int32_t b;
};
#endif
