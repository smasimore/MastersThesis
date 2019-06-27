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

#include "Errors.h"

class State
{
public:

    /**
     * Constructor for the state, public for testing purposes only
     *
     * @param   intData     vector of int32_t to serve as placeholder data
     *
     * @ret     State       State class with data from param
     */
    State (std::vector<int32_t> intData);

    /**
     * Constructor for a state with more complex State data, tentative
     *
     * @param   stateName           string containing the state name
     * @param   validTransitions    vector of States that the State can
     *                              transition to
     *
     * @ret     State               State class with data from params
     */
    State (std::string stateName, std::vector<std::string> targetTransitions);

    /**
     * Print the State data
     *
     * @ret     E_SUCCESS   Succesfully printed the State data
     */
    Error_t printData ();

    /**
     * Get the State data
     *
     * @param   result      Reference to vector of type int32_t to store State
     *                      data in
     *
     * @ret     E_SUCCESS   Successfully stored State data in result
     */
    Error_t getData (std::vector<int32_t> &result);

    /**
     * Get the State name
     *
     * @param   result      Reference to String type to store State name in
     *
     * @ret     #_SUCCESS   Successfully stored State name in result
     */
    Error_t getName (std::string &result);

    Error_t getTransitions (std::vector<std::string> &result);

private:

    /**
     * The name of the State, for identification and mapping purposes
     */
    std::string stateName;

    /**
     * The first iteration skeleton State data, vector of type int32_t to be
     * used temporarily
     */
    std::vector<int32_t> stateData;

    /**
     * The second iteration of valid State transitions, vector of type String
     * representing name of the states to use for now
     */
    std::vector<std::string> targetTransitions;
};
#endif

