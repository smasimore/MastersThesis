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
#include "Errors.hpp"

class StateMachine
{
public:

    /**
     * Struct containing the necessary elements of a State
     */
    typedef struct StateInput
    {
        std::string name;
        std::vector<std::string> transitions;
        std::vector<State::Action_t> actions;
    } State_t;

    /**
     * Create a statemachine from a list of state names and transitions.
     *
     * @param   rSM                 reference to smart pointer of type 
     *                              StateMachine
     * @param   stateList           vector of State_t structs containing the
     *                              input data for the States to be created
     *
     * @ret     E_SUCCESS           successfully passed a StateMachine object 
     *                              into rSM using States in stateList
     *          E_DUPLICATE_NAME    a duplicate state name found in stateList
     */
    static Error_t createNew (std::unique_ptr<StateMachine> &rSM,
                              const std::vector<State_t> &stateList);

    /**
     * Intermediate function to add, allocate, and map State to the State Map.
     * When called the first time, will set the first state as the
     * current state of the StateMachine.
     *
     * @param   stateIn             State struct containing the State's data
     *
     * @ret     E_SUCCESS           State object successfully added and mapped
     *          E_DUPLICATE_NAME    State with same name already exists
     */
    Error_t addState (State_t stateIn);

    /**
     * Intermediate function to find and store a State in the map by State name
     *
     * @param   rState          Reference to shared pointer of state
     * @param   stateName       Name of the state to find
     *
     * @ret     E_SUCCESS       Successfully found State and stored in param
     *          E_NAME_NOTFOUND Could not find a state with this name
     */
    Error_t findState (std::shared_ptr<State> &rState, std::string stateName);

    /**
     * Intermediate function to force a State Transition. For now, just update
     * stored String representing the current State.
     *
     * @param   targetState             String containing name of target state
     *
     * @ret     E_SUCCESS               Successfully transitioned to state
     *          E_INVALID_NAME          Target state name does not exist
     *          E_INVALID_TRANSITION    Target state not a valid transition
     */
    Error_t switchState (std::string transitionState);

    /**
     * Function to return a copy of the name of current State
     * 
     * @param   result          Reference to string to store name of State
     *
     * @ret     E_SUCCESS       Name of current State stored in result
     *          E_NO_STATES     No states have been added to StateMachine
     */
    Error_t getCurrentStateName (std::string &result);

    /**
     * Function to return a copy of the valid transitions of current State
     *
     * @param   result      Reference to vector of strings to store valid
     *                      transitions of the state
     *
     * @ret     E_SUCCESS   Valid transitions of current State stored in result
     *          E_NO_STATES No states have been added to StateMachine
     */
    Error_t getCurrentStateTransitions (std::vector<std::string> &result);

    /**
     * Function to return a copy of the action sequence of current State
     *
     * @param   result      Reference to map of timestamps and corresponding
     *                      functions and params to store action sequence
     *
     * @ret     E_SUCCESS   Action sequence of current state stored in result
     * @ret     E_NO_STATES No states have been added to StateMachine
     */
    Error_t getCurrentActionSequence (std::map<int32_t, std::vector<
                                      State::Action_t>> &result);

    /**
     * Function to execute all actions in the current state's action sequence.
     *  NOTE: This function does not yet consider real time timestamps. It will
     *  only iterate entirely through the action sequence to test outputs.
     *
     * @ret     E_SUCCESS   Action sequence executed successfully
     *          E_NO_STATES No states have been added to StateMachine
     *          [other]     Other failure result from functions in sequence
     */
    Error_t executeCurrentSequence ();

    /**
     * Function to handle real time execution of the actions in the action 
     * sequence. Based on the StateMachine's understanding of real time, this
     * function will attempt to call each function when its timestamp is equal
     * or less than the measurement of the current time elapsed since State
     * transition. 
     
     * This function should be called from the main periodic thread. Currently
     * this periodic function does not yet account for checking whether or not
     * transitions between states should occur. Depending on the complexity
     * of transition logic, it might make more sense to implement to separate
     * periodic functions based on the priorities of executing actions versus
     * checking for a state transition.
     *
     * @ret     E_SUCCESS   Successfully checked the time and executed actions
     * @ret     E_NO_STATES No states have been added to StateMachine
     * @ret     [other]     Other failure result from functions in sequence
     */
    Error_t periodic ();

    /**
     * Placeholder public variable to test time logic in periodic function
     */
    int32_t timeElapsed;

private:

    /**
     * Private constructor to ensure named constructor idioms are used
     */
    StateMachine ();

    /**
     * Unordered map to create the map of the states using key type String and 
     * value type pointer to State
     */
    std::unordered_map<std::string, std::shared_ptr<State>> mStateMap;

    /**
     * Shared pointer to the current state
     */
    std::shared_ptr<State> mPStateCurrent;

    /**
     * Iterator to step through the action sequence
     */
    std::map<int32_t, std::vector<State::Action_t>>::iterator mActionIter;

    /**
    * End iterator to catch the end of the action sequence
    */
    std::map<int32_t, std::vector<State::Action_t>>::iterator mActionEnd;
};
#endif
