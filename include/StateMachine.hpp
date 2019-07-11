/**
 * StateMachine struct to manage states aboard the flight computer.
 * Prove basic skeleton functionality of data and initialization.
 */

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include "State.hpp"
#include "Errors.h"

class StateMachine
{
public:

    /**
     * Create a statemachine from a default hardcoded case. This is important
     * in case of parser, config, or other external failures.
     *
     * @param   ppStateMachine      pointer to a pointer to StateMachine object
     *
     * @ret     StateMachine        struct from private constructor
     */
    static Error_t fromDefault (StateMachine **ppStateMachine);

    /**
     * Create a statemachine from data in an array. This is a placeholder
     * function to demonstrate creation from user-defined data.
     *
     * @param   ppStateMachine  pointer to a pointer to StateMachine object
     *          c[]             array of int32_t, an arbitrary data type
     *
     * @ret     StateMachine    struct from private constructor and parameter 
                                data
     */
    static Error_t fromArr (StateMachine **ppStateMachine, int32_t c[]);

    /**
     * Create a statemachine from a defined list of states.
     * @param   ppStateMachine  pointer to a pointer to StateMachine object
     *          stateList       vector of type State
     *
     * @ret     
     */
    static Error_t fromStates (StateMachine **ppStateMachine,
        std::vector<State> stateList);

    /**
     * Intermediate function to add and map State to the State Map/
     *
     * @param   newState            State object of state to add
     *
     * @ret     E_SUCCESS           State object successfully added and mapped
     *          E_DUPLICATE_NAME    State with same name already exists
     */
    Error_t addState (State newState);

    /**
     * Intermediate function to find and store a State in the map by State name
     *
     * @param   stateResult     Reference to State object to store the result
     * @param   stateName       Name of the state to find
     *
     * @ret     E_SUCCESS       Successfully found State with name
     *          E_NAME_NOTFOUND Could not find a state with this name
     */
    Error_t findState (State &stateResult, std::string stateName);

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

    /**
     * Deletes the state map to avoid memory leaks
     *
     * ONLY USE FOR TESTING PURPOSES TO PREVENT MEMORY LEAKS.
     *
     * @ret     E_SUCCESS   successfully cleared the state map
     */
    Error_t deleteMap ();

private:

    /**
     * Private constructor to ensure named constructor idoims are used
     */
    StateMachine (int32_t a, int32_t b);

    /**
     * Private copy constructor to enforce singleton
     */
    StateMachine (StateMachine const &);

    /*
     * Private assignment operator to enforce singleton
     */
    StateMachine& operator=(StateMachine const &);

    /**
     * Placeholder skeleton data for temporary testing purposes only
     */
    int32_t a;
    int32_t b;

    /**
     * Pointer to Unordered map to create the map of the states using
     * key type String and value type State
     */
    std::unordered_map<std::string, State> *stateMap;
 
};
#endif
