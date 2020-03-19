/**
 * State Machine to manage the rocket states. A State Machine is defined by a
 * set of States and an initial state. The initial state is set by the value in
 * the Data Vector's state element at State Machine initialization time. This
 * element must be a uint32_t. The set of States is defined by the State Machine 
 * config.
 *
 * A State is comprised of 3 things:
 *     1) State ID    - This must be unique to the set of states provided in the
 *                      config.
 *     2) Actions     - This is a map of "time elapsed in state" to a list of 
 *                      actions to be executed at that time. A single action 
 *                      writes a value to an element in the Data Vector (e.g. a 
 *                      controller's mode element, setting an igniter control 
 *                      signal high). If a state is entered more than once, the
 *                      actions will be repeated. The only prohibited action is 
 *                      to write to the Data Vector's state element. The only 
 *                      way to change state is through a transition, so this is 
 *                      explicitly prohibited to ensure this.
 *     3) Transitions - Transitions are the only way to change the state of the
 *                      State Machine. Each transition consists of a Data
 *                      Vector element, comparison type (e.g. ==, >, <), value
 *                      to compare the element's value to, and state to 
 *                      transition to if the comparison is true. Order for a
 *                      list of transitions matters. The first transition
 *                      condition to be met will determine the state 
 *                      transitioned to.
 *
 * An example config is in StateMachineTest.cpp.
 *
 */

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include "StateMachineEnums.hpp"
#include "State.hpp"
#include "Time.hpp"
#include "EnumClassHash.hpp"
#include "Errors.hpp"

class StateMachine final
{
public:

    /**
     * Config for the State Machine.
     */
    typedef std::vector<State::Config_t> Config_t;

    /**
     * Create a new State Machine.
     *
     * @param   kConfig              State Machine config.
     * @param   kPDataVector         Pointer to Data Vector.
     * @param   kTimeNs              Current time in nanoseconds.
     * @param   kDvStateElem         DV elem to read initial state from and 
     *                               store current state.
     * @param   kPSmRet              Pointer to return StateMachine.
     *
     * @ret     E_SUCCESS            Successfully initialized State Machine.
     *          E_DATA_VECTOR_NULL   Data Vector ptr null.
     *          E_DATA_VECTOR_WRITE  Failed to write to DV.
     *          E_DATA_VECTOR_READ   Failed to read from DV.
     *          E_INVALID_ELEM       State DV elem does not exist.
     *          E_INCORRECT_TYPE     State DV elem type must be DV_T_UINT32.
     *          E_NO_STATES          Config empty.
     *          E_INVALID_ENUM       Invalid StateId_t. 
     *          E_DUPLICATE_STATE    Duplicate state ID in config.
     *          E_INVALID_TRANSITION Transition target state not a valid state.
     *          E_STATE_NOTFOUND     Invalid initial state in DV.
     *          [other]              Invalid Transitions or Actions config.
     */
    static Error_t createNew (Config_t& kConfig,
                              std::shared_ptr<DataVector> kPDataVector,
                              Time::TimeNs_t kTimeNs,
                              DataVectorElement_t kDvStateElem,
                              std::unique_ptr<StateMachine> &kPSmRet);

    /**
     * Step the State Machine forward. Logic is as follows:
     *
     *     1) Check if any state transition conditions are true. If yes, 
     *        transition to new state.
     *     2) Check if any state actions should occur based on time elapsed in
     *        state. If yes, execute actions.
     *
     *  @param  kTimeNs             Current time in nanoseconds.
     *  
     *  @ret    E_SUCCESS           Successfully stepped State Machine.
     *          E_INVALID_TIME      kTimeNs less than current state's start 
     *                              time.
     *          E_STATE_NOTFOUND    Could not find transition target state.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     *          [other]             Error during transitions or actions check or
     *                              execution.
     *
     */
    Error_t step (Time::TimeNs_t kTimeNs);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF STATE MACHINE
     *
     * Switch state. Updates class variables, resets new state's action 
     * iterator, and updated Data Vector state elem.
     *
     * @param   kTargetState            Enum containing ID of target state.
     * @param   kTimeNs                 Current time in nanoseconds.
     *
     * @ret     E_SUCCESS               Successfully transitioned to state
     *          E_STATE_NOTFOUND        Could not find kTargetState.
     *          E_DATA_VECTOR_WRITE     Failed to write to DV.
     */
    Error_t switchState (StateId_t kTargetState, Time::TimeNs_t kTimeNs);

private:

    /**
     * Pointer to Data Vector.
     */
    std::shared_ptr<DataVector> mPDataVector;

    /**
     * Map of State ID's to pointer to State object.
     */
    std::unordered_map<StateId_t, std::shared_ptr<State>, EnumClassHash>
        mStateIdToStateMap;

    /**
     * Time when transitioned to current state.
     */
    Time::TimeNs_t mStateStartTimeNs;

    /**
     * Pointer to the current state.
     */
    std::shared_ptr<State> mPStateCurrent;

    /**
     * Data Vector element storing the current State in StateMachine
     */
    DataVectorElement_t mDvStateElem;

    /**
     * Private constructor used by createNew.
     *
     * @param   kConfig              State Machine config.
     * @param   kPDataVector         Pointer to Data Vector.
     * @param   kTimeNs              Current time in nanoseconds.
     * @param   kDvStateElem         DV elem to read initial state from and 
     *                               store current state.
     * @param   kRet                 Object to store return status in.
     *
     * @ret     E_SUCCESS            Successfully initialized State Machine.
     *          E_DATA_VECTOR_NULL   Data Vector ptr null.
     *          E_DATA_VECTOR_WRITE  Failed to write to DV.
     *          E_DATA_VECTOR_READ   Failed to read from DV.
     *          E_INVALID_ELEM       State DV elem does not exist.
     *          E_INCORRECT_TYPE     State DV elem type must be DV_T_UINT32.
     *          E_NO_STATES          Config empty.
     *          E_INVALID_ENUM       Invalid StateId_t. 
     *          E_DUPLICATE_STATE    Duplicate state ID in config.
     *          E_INVALID_TRANSITION Transition target state not a valid state.
     *          E_STATE_NOTFOUND     Invalid initial state in DV.
     *          [other]              Invalid Transitions or Actions config.
     */
    StateMachine (Config_t& kConfig, std::shared_ptr<DataVector> kPDataVector,
                  Time::TimeNs_t kTimeNs, DataVectorElement_t kDvStateElem,
                  Error_t& kRet);

    /**
     * Function used by constructor to add, allocate, and map State to the State 
     * Map. 
     *
     * @param   kPDataVector        Pointer to Data Vector.
     * @param   kStateConfig        State config.
     *
     * @ret     E_SUCCESS           State object successfully added and mapped
     *          E_INVALID_ENUM      Invalid StateId_t. 
     *          E_DUPLICATE_STATE   State with same ID already exists
     *          [other]             Invalid Transitions or Actions.
     */
    Error_t addState (std::shared_ptr<DataVector> kPDataVector,
                      State::Config_t kStateConfig);

    /**
     * Find State in map.
     *
     * @param   kStateId         ID of the state to find.
     * @param   kPStateRet       Pointer to store state object.
     *
     * @ret     E_SUCCESS        Successfully found State and stored in param
     *          E_STATE_NOTFOUND Could not find a state with this ID
     */
    Error_t findState (StateId_t kStateId, std::shared_ptr<State>& kPStateRet);
};

#endif
