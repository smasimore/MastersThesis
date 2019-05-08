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

#ifndef State_HPP
#define State_HPP
class State 
{
public:

    /**
     * Construct the State from skeleton data type.
     * 
     * @param   int_data    vector of int32_t to serve as placeholder data
     *
     * @ret     State       State class with corresponding data as input
     */
	State (std::vector<int32_t> int_data);

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
     * The skeleton State data, vector of type int32_t to be used temporarily
     */
	std::vector<int32_t> StateData;
};
#endif

