/**
 * State class that will be created and manipulated by the StateMachine
 * Each state must be initialized with important data, such as associated
 * transition and target states.
 */

#ifndef STATE_HPP
#define STATE_HPP

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>

#include "Errors.h"

class State
{
public:

    /**
     * Struct containing the necessary elements of an action 
     */
    typedef struct Action
    {
        int32_t timestamp;
        Error_t (*func) (int32_t);
        int32_t param;
    } Action_t;

    /**
     * Struct containing the necessary elements of a State
     */
    typedef struct StateInput
    {
        std::string name;
        std::vector <std::string> transitions;
        std::vector<Action_t> actions;
    } State_t;

    /**
     * Constructor for a state with more complex State data, tentative
     *
     * @param   stateName           string containing the state name
     * @param   validTransitions    vector of strings representing valid states
     *
     * @ret     State               State class with data from params
     */
    State (std::string stateName, const std::vector<std::string> 
           &targetTransitions);

    /**
     * Constructor for a State containing name, transitions, and actions
     *
     * @param   stateName           string containing the state name
     * @param   validTransitions    vector of strings representing valid states
     * @param   actionList          vector of Action_t containing timestamp,
     *                              pointer to function, and function param.
     *
     * @ret     State               State class with data from params
     */
    State (std::string stateName, const std::vector<std::string>
           &targetTransitions, const std::vector<Action_t> &actionList);

    /**
     * Get the State name
     *
     * @param   result      Reference to String type to store State name in
     *
     * @ret     E_SUCCESS   Successfully stored State name in result
     */
    Error_t getName (std::string **ppResult);

    /**
     * Get the State's valid transitions
     *
     * @param   result      Reference to vector of type string to store data in
     *
     * @ret     E_SUCCESS   Successfully stored State transitions in result
     */
    Error_t getTransitions (std::vector<std::string> **ppResult);

    /**
     * Get the State's action sequence
     *
     * @param   result      Reference to map of timestamps and corresponding
     *                      pointers to functions and params, to store sequence
     *
     * @ret     E_SUCCESS   Successfully stored State action sequence in result
     */
    Error_t getActionSequence (std::map<int32_t, std::vector<Action_t>> 
                               **ppResult);

private:

    /**
     * The name of the State, for identification and mapping purposes
     */
    std::string mStateName;

    /**
     * The valid transitions of the State, vector of type String representing 
     * the name of the valid states for a transition
     */
    std::vector<std::string> mTargetTransitions;

    /**
     * The old iteration of the action sequence of the State. Ordered map
     * containing vector of tuples, using timestamp as the key. Tuples in the
     * vector contain the pointer to function, and the parameter.
     */
    std::map <int32_t, std::vector <Action_t>> mActionSequence;
};
#endif