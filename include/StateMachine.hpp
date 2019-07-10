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
#include <algorithm>
#include <memory>
#include "State.hpp"
#include "Errors.h"

class StateMachine
{
public:

    /**
     * Create a statemachine from a default hardcoded case. This is important
     * in case of parser, config, or other external failures.
     *
     * @param   rSM                 reference to smart pointer of StateMachine
     *
     * @ret     StateMachine        struct from private constructor
     */
    static Error_t fromDefault (std::unique_ptr<StateMachine> &rStateMachine);

    /**
     * Create a statemachine from data in an array. This is a placeholder
     * function to demonstrate creation from user-defined data.
     *
     * @param   ppStateMachine  pointer to a pointer to StateMachine object
     * @param   c[]             array of int32_t, an arbitrary data type
     *
     * @ret     StateMachine    struct from private constructor and parameter 
                                data
     */
    static Error_t fromArr (StateMachine **ppStateMachine, int32_t c[]);

    /**
     * Create a statemachine from a defined list of states.
     * @param   ppStateMachine  pointer to a pointer to StateMachine object
     * @param   stateList       vector of type State
     *
     * @ret     
     */
    static Error_t fromStates (StateMachine **ppStateMachine,
        std::vector<State> stateList);

    /**
     * Intermediate function to add and map State to the State Map.
     * When called the first time, will set the first state as the
     * current state of the StateMachine.
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
     * Intermediate function to force a State Transition. For now, just update
     * stored String representing the current State.
     *
     * @param   targetState             String containing name of target state
     *
     * @ret     E_SUCCESS               Successfully transitioned to state
                E_INVALID_NAME          Target state name does not exist
     *          E_INVALID_TRANSITION    Target state not a valid transition
     */
    Error_t switchState (std::string targetState);

    /**
     * Intermediate function to return the name of current State
     * 
     * @param   result          Reference to string to store name of State
     *
     * @ret     E_SUCCESS       Name of current State stored in result
     */
    Error_t getStateName (std::string &result);

    /**
     * Intermediate function to return the valid transitions of current State
     *
     * @param   result      Reference to vector of strings to store valid
     *                      transitions of the state
     *
     * @ret     E_SUCCESS   Valid transitions of current State stored in result
     */
    Error_t getStateTransitions (std::vector<std::string> &result);

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
     * Deletes the allocated map in StateMachine to avoid memory leaks
     *
     * ONLY USE FOR TESTING PURPOSES TO PREVENT MEMORY LEAKS.
     *
     * @ret     E_SUCCESS   successfully deleted the map
     */
    Error_t deleteMap ();

    /**
     * Deletes the allocated state in StateMachine to avoid memory leaks
     *
     * ONLY USE FOR TESTING PURPOSES TO PREVENT MEMORY LEAKS.
     *
     * @ret     E_SUCCESS   successfully deleted the state
     */
    Error_t deleteState ();

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

    /**
     * Pointer to Unordered map to create the map of the states using
     * key type String and value type State
     */
    std::unordered_map<std::string, State> *pStateMap;

    /**
     * Pointer to a copy of the current state
     */
    State* pStateCurrent;
 
};
#endif
