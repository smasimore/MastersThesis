/**
 * State object that encapsulates the data used to defined a state:
 *   
 *   1) State ID
 *   2) Actions to execute when in the State
 *   3) Transitions to check and execute when in the State.
 *
 */

#ifndef STATE_HPP
#define STATE_HPP

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <vector>
#include <map>

#include "Errors.hpp"
#include "StateMachineEnums.hpp"
#include "Transitions.hpp"
#include "Actions.hpp"
#include "DataVector.hpp"

class State final
{
public:

    /**
     * Config for a State in the State Machine.
     */
    typedef struct Config
    {
        StateId_t id;
        Actions::Config_t actions;
        Transitions::Config_t transitions;
    } Config_t;

    /**
     * Constructor for a State containing name, transitions, and actions.
     *
     * @param   kStateId            Enum containing the state ID.
     * @param   kTransitionsConfig  Transitions config.
     * @param   kActionList         Vector of Action_t containing timestamp,
     *                              pointer to function, and function param.
     * @param   kStateElem          Data Vector elem storing current state.
     * @param   kRet                Param to store return value in.
     *
     * @ret     E_SUCCESS           Successfully created state.
     *          [other]             Transitions or Actions initialization error.
     */
    State (std::shared_ptr<DataVector> kPDataVector,
           StateId_t kStateId,
           const Transitions::Config_t& kTransitionsConfig,
           const Actions::Config_t& kActionsConfig,
           DataVectorElement_t kStateElem,
           Error_t& kRet);

    /**
     * Get the State ID.
     *
     * @param   kIdRet      Param to store state's ID in.
     *
     * @ret     E_SUCCESS   Successfully stored state ID.
     */
    Error_t getId (StateId_t& kIdRet);

    /**
     * Get the State's valid transitions
     *
     * @param   kPTransitionsRet  Pointer to state's transitions object.
     *
     * @ret     E_SUCCESS         Successfully stored Transitions.
     */
    Error_t getTransitions (std::shared_ptr<Transitions>& kPTransitionsRet);

    /**
     * Get the State's Actions.
     *
     * @param   result      Reference to map of timestamps and corresponding
     *                      pointers to functions and params, to store sequence
     *
     * @ret     E_SUCCESS   Successfully stored State action sequence in result
     */
    Error_t getActions (std::shared_ptr<Actions>& kPActionsRet);

private:

    /**
     * State ID.
     */
    StateId_t mStateId;

    /**
     * State's actions to execute.
     */
    std::shared_ptr<Actions> mActions;

    /**
     * State's transitions.
     */
    std::shared_ptr<Transitions> mTransitions;
};
#endif
