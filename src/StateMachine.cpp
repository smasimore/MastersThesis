#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/


Error_t StateMachine::createNew (std::unique_ptr<StateMachine> &rSM,
                                 const std::vector<State_t> &stateList)
{
    rSM.reset (new StateMachine ());
    for (State_t state : stateList)
    {
        Error_t retState = rSM->addState (state);
        if (retState != E_SUCCESS)
        {
            // on failure, free the StateMachine
            rSM.reset ();
            return E_DUPLICATE_NAME;
        }
    }
    return E_SUCCESS;
}

Error_t StateMachine::addState (State_t stateIn)
{
    // Allocate the state from parameter data, and create shared pointer
    std::shared_ptr<State> pNewState (new State (stateIn.name, 
                                                 stateIn.transitions,
                                                 stateIn.actions));

    // Check if pointer to current state is null; if so, then set as current
    if (mPStateCurrent == nullptr)
    {
        // overwrite memory of current state
        mPStateCurrent = pNewState;
        // Reset the iterator from the State's action sequence
        std::map<int32_t, std::vector<State::Action_t> > *pTempMap;
        Error_t ret = mPStateCurrent->getActionSequence (&pTempMap);
        if (ret != E_SUCCESS)
        {
            return ret;
        }
        mActionIter = pTempMap->begin ();
        mActionEnd = pTempMap->end ();
    }
    // Insert returns pair containing bool; true if inserted, false if not.
    // Will not insert if there exists a duplicate key, aka duplicate name
    bool resultBool = (this->mStateMap).
        insert (std::make_pair (stateIn.name, pNewState)).second;
    if (resultBool)
    {
        return E_SUCCESS;
    }
    else
    {
        return E_DUPLICATE_NAME;
    }
}

Error_t StateMachine::findState (std::shared_ptr<State> &rState,
                                 std::string stateName)
{
    // search the unordered map
    std::unordered_map <std::string, std::shared_ptr<State> > ::const_iterator 
        search = mStateMap.find (stateName);
    // if element is not found, will point to end of the map
    if (search == mStateMap.end ())
    {
        return E_NAME_NOTFOUND;
    }
    else
    {
        rState = search->second;
        return E_SUCCESS;
    }
}

Error_t StateMachine::getCurrentStateName (std::string &result)
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }
    std::string *pName;
    Error_t ret = mPStateCurrent->getName (&pName);
    result = *pName;
    return ret;    
}

Error_t StateMachine::getCurrentStateTransitions (std::vector<std::string> 
                                                  &result)
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }
    std::vector<std::string> *pTrans;
    Error_t ret = mPStateCurrent->getTransitions (&pTrans);
    result = *pTrans;
    return ret;
}

Error_t StateMachine::getCurrentActionSequence (std::map<int32_t,
                                                std::vector<State::Action_t>> 
                                                &result)
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }
    std::map<int32_t, std::vector<State::Action_t>>
        *pSequence;
    Error_t ret = mPStateCurrent->getActionSequence (&pSequence);
    result = *pSequence;
    return ret;
}

Error_t StateMachine::executeCurrentSequence ()
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }

    // Create a temp pointer to map, then point to address of action sequence
    std::map<int32_t, std::vector<State::Action_t>> *pTempMap;
    Error_t ret = mPStateCurrent->getActionSequence (&pTempMap);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Iterate through map; guaranteed ordered by timestamp
    for (std::pair<int32_t, std::vector<State::Action_t>> actionPair : 
         *pTempMap)
    {
        for (State::Action_t action : actionPair.second)
        {
            // call function in pointer with implicit dereference
            ret = (action.func) (action.param);
            // catch failure points in action sequence
            if (ret != E_SUCCESS)
            {
                return ret;
            }
        }
    }
    return E_SUCCESS;
}

Error_t StateMachine::switchState(std::string transitionState)
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }

    // Get the valid transitions (using intermediate function)
    std::vector<std::string> *pValidTrans;
    Error_t ret = mPStateCurrent->getTransitions (&pValidTrans);

    // getter function only returns E_SUCCESS for now, if block for future use
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Check if valid transitions contains the target state
    if (std::find (pValidTrans->begin (), pValidTrans->end (), transitionState) 
        != pValidTrans->end())
    {   
        // if not equal to end, transition is valid; switch the current state
        // create a temporary empty state
        std::shared_ptr<State> stateResult;
        // get the target state from the state map and check if found
        Error_t ret = findState (stateResult, transitionState);
        if (ret != E_SUCCESS)
        {
            return E_NAME_NOTFOUND;
        }

        // overwrite current state with the target state
        mPStateCurrent = stateResult;
        // Reset the iterator from the new State's action sequence
        std::map<int32_t, std::vector<State::Action_t>> *pTempMap;
        ret = mPStateCurrent->getActionSequence (&pTempMap);
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        mActionIter = pTempMap->begin ();
        mActionEnd = pTempMap->end ();
        return E_SUCCESS;
    }

    else
    {
        // otherwise, transition is invalid; cannot switch to the new state
        return E_INVALID_TRANSITION;
    }
}

Error_t StateMachine::periodic ()
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }

    // Begin a loop, to ensure all actions are being completed. There might be
    // more than one timestamp that must be executed given a current time. As
    // long as there are still actions to complete and timestamps are met, keep
    // executing the actions necessary.
    while (mActionIter != mActionEnd && mActionIter->first <= timeElapsed)
    {
        // Timestamp met, execute all functions in the vector
        for (State::Action_t action : mActionIter->second)
        {
            // call each function in pointer with implicit dereference
            Error_t ret = (action.func) (action.param);
            // catch failure points in action sequence, and break out
            if (ret != E_SUCCESS)
            {
                return ret;
            }
        }

        // Increment the iterator to the next vector of actions
        ++mActionIter;     
    }
    return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine ()
{
    mPStateCurrent = nullptr;
}
