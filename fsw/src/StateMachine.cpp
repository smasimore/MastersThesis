#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::createNew (Config_t& kConfig,
                                 std::shared_ptr<DataVector> kPDataVector,
                                 Time::TimeNs_t kTimeNs,
                                 DataVectorElement_t kDvStateElem,
                                 std::unique_ptr<StateMachine> &kPSmRet)
{

    // Initialize SM.
    Error_t ret = E_SUCCESS;
    kPSmRet.reset (new StateMachine (kConfig, kPDataVector, kTimeNs, 
                                     kDvStateElem, ret));

    // Free object if failed.
    if (ret != E_SUCCESS)
    {
        kPSmRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t StateMachine::step (Time::TimeNs_t kTimeNs)
{
    // 1) Verify current time not less than current state's start time.
    if (kTimeNs < mStateStartTimeNs)
    {
        return E_INVALID_TIME;
    }

    // 2) Get current state's transitions.
    std::shared_ptr<Transitions> pTransitions = nullptr; 
    Error_t ret = mPStateCurrent->getTransitions (pTransitions);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 3) Check if a transition should occur.
    bool shouldTransition = false;
    StateId_t targetState;
    ret = pTransitions->checkTransitions (shouldTransition, targetState);    
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 4) Transition if condition met.
    if (shouldTransition == true)
    {
        ret = this->switchState (targetState, kTimeNs);
        if (ret != E_SUCCESS)
        {
            return ret;
        }
    }

    // 5) Calculate elapsed time in State.
    Time::TimeNs_t timeElapsedNs = kTimeNs - mStateStartTimeNs;

    // 6) Get current state's actions.
    std::shared_ptr<Actions> pActions;
    ret = mPStateCurrent->getActions (pActions);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 7) Get actions that should be executed.
    std::vector<std::shared_ptr<Actions::ActionBase>> actionsToExecute;
    ret = pActions->checkActions (timeElapsedNs, actionsToExecute);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 8) Execute actions.
    for (std::shared_ptr<Actions::ActionBase> action : actionsToExecute)
    {
        ret = action->execute (mPDataVector);
        if (ret != E_SUCCESS)
        {
            return ret;
        }
    }

    return E_SUCCESS;
}

Error_t StateMachine::switchState (StateId_t kTargetState, 
                                   Time::TimeNs_t kTimeNs)
{
    Error_t ret = E_SUCCESS;

    // 1) Get target state object from map.
    std::shared_ptr<State> stateResult;
    ret = this->findState (kTargetState, stateResult);
    if (ret != E_SUCCESS)
    {
        return E_STATE_NOTFOUND;
    }

    // 2) Overwrite current state and start time with new state and time.
    mPStateCurrent = stateResult;
    mStateStartTimeNs = kTimeNs;

    // 3) Reset the iterator for the new state's action sequence.
    std::shared_ptr<Actions> pActions = nullptr;
    ret = mPStateCurrent->getActions (pActions);
    if (ret != E_SUCCESS)
    {
        return ret;
    }
    ret = pActions->resetActionIterator ();
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 4) Write new state to Data Vector.
    if (mPDataVector->write (mDvStateElem, (uint32_t) kTargetState) !=
            E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine (Config_t& kConfig, 
                            std::shared_ptr<DataVector> kPDataVector,
                            Time::TimeNs_t kTimeNs,
                            DataVectorElement_t kDvStateElem, 
                            Error_t& kRet) :
    mPDataVector (kPDataVector),
    mStateStartTimeNs (0),
    mPStateCurrent (nullptr),
    mDvStateElem (kDvStateElem)
{
    // 1) Verify kPDataVector is not null.
    if (kPDataVector == nullptr)
    {
        kRet = E_DATA_VECTOR_NULL;
        return;
    }
    
    // 2) Verify kDvStateElem exists in DV.
    if (kPDataVector->elementExists (kDvStateElem) != E_SUCCESS)
    {
        kRet = E_INVALID_ELEM;
        return;
    }
    
    // 3) Verify kDvStateElem type is a uint32_t.
    DataVectorElementType_t type = DV_T_LAST;
    if (kPDataVector->getElementType (kDvStateElem, type) != E_SUCCESS)
    {
        kRet = E_DATA_VECTOR_READ;
        return;
    }
    if (type != DV_T_UINT32)
    {
        kRet = E_INCORRECT_TYPE;
        return;
    }

    // 4) Verify states list is not empty.
    if (kConfig.empty ())
    {
        kRet = E_NO_STATES;
        return;
    }

    // 5) Add States.
    for (State::Config_t stateConfig : kConfig)
    {
        kRet = this->addState (kPDataVector, stateConfig);
        if (kRet != E_SUCCESS)
        {
            return;
        }
    }

    // 6) Verify each transition target state is valid. In a separate loop to 
    //    use mStateIdToStateMap, which is built by previous step.
    for (State::Config_t stateConfig : kConfig)
    {
        for (std::shared_ptr<Transitions::TransitionBase> transition : 
                stateConfig.transitions)
        {
            if (mStateIdToStateMap.find (transition->mTargetState) == 
                    mStateIdToStateMap.end ())
            {
                kRet = E_INVALID_TRANSITION;
                return;
            }
        }
    }
    
    // 7) Set up initial State.
    uint32_t initialState;
    if (kPDataVector->read (mDvStateElem, initialState) != E_SUCCESS)
    {
        kRet = E_DATA_VECTOR_READ;
        return;
    }
    kRet = this->switchState ((StateId_t) initialState, kTimeNs);
    if (kRet != E_SUCCESS)
    {
        return;
    }
}

Error_t StateMachine::addState (std::shared_ptr<DataVector> kPDataVector,
                                State::Config_t kStateConfig)
{
    // Verify state ID is valid.
    if (kStateConfig.id >= STATE_LAST)
    {
        return E_INVALID_ENUM;
    }

    // Create State object.
    Error_t ret = E_SUCCESS;
    std::shared_ptr<State> pNewState (new State (kPDataVector,
                                                 kStateConfig.id, 
                                                 kStateConfig.transitions,
                                                 kStateConfig.actions, 
                                                 mDvStateElem,
                                                 ret));
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Insert state into map. Insert call returns pair containing bool; true if 
    // inserted, false if not. Will return false if key already exists.
    bool resultBool = mStateIdToStateMap.insert (
                                         std::make_pair (kStateConfig.id, 
                                                         pNewState)).second;
    if (resultBool == false)
    {
        return E_DUPLICATE_STATE;
    }
        
    return E_SUCCESS;
}

Error_t StateMachine::findState (StateId_t kStateId, 
                                 std::shared_ptr<State> &kPStateRet)
{
    // If element is not found, will point to end of the map.
    if (mStateIdToStateMap.find (kStateId) == mStateIdToStateMap.end ())
    {
        return E_STATE_NOTFOUND;
    }
    else
    {
        kPStateRet = mStateIdToStateMap[kStateId];
        return E_SUCCESS;
    }
}
