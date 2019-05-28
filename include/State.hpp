/**
 * State class that will be created and manipulated by the StateMachine, eventually
 * Each state must be initialized with important data, such as associated transition and target states.
 * Each state object should then be stored in a hashmap to easily allow access of important state data.
 */

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <vector>

#include "Errors.h"

#ifndef STATE_HPP
#define STATE_HPP
class State 
{
public:

    /**
     * Default Constructor to resolve mapping issues
     *
     * The unordered_map seems to require a default constructor to correctly map the objects.
     *
     * @ret     State       State class with empty data
     */

    State ();

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
     *
     *          validTransitions    vector of States that the State can transition to
     *
     * @ret     State               State class with data from params
     */
    State (std::string stateName, std::vector<State> validTransitions);

    /**
     * Print the State data
     *
     * @ret     E_SUCCESS   Succesfully printed the State data
     */
	Error_t printData ();

    /**
     * Get the State data
     * 
     * @param   result      Reference to vector of type int32_t to store State dasta in
     *
     * @ret     E_SUCCESS   Successfully stored State data in result
     */
	Error_t getData (std::vector<int32_t> &result);

private:

    /**
     * The name of the State, for identification and mapping purposes
     */
    std::string stateName;

    /**
     * The first iteration skeleton State data, vector of type int32_t to be used temporarily
     */
	std::vector<int32_t> stateData;

    /**
     * The second iteration valid State transitions, vector of type State to use for now
     */
    std::vector<std::string> targetTransitions;
};
#endif

