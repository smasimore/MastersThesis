#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::fromDefault (std::unique_ptr<StateMachine> &rSM)
{
    // reset the smart pointer, using corresponding default case
    rSM.reset (new StateMachine ());
    return E_SUCCESS;

}

Error_t StateMachine::fromStates (std::unique_ptr<StateMachine> &rSM,
                                  const std::vector<std::tuple<std::string, 
                                  std::vector<std::string>>> &stateList)
{
    rSM.reset (new StateMachine ());
    for (std::tuple<std::string, std::vector<std::string>> tup : stateList)
    {
        Error_t retState = rSM->addState (std::get<0> (tup), std::get<1>(tup));
        if (retState != E_SUCCESS)
        {
            return E_DUPLICATE_NAME;
        }
    }
    return E_SUCCESS;    
}

Error_t StateMachine::fromStates (std::unique_ptr<StateMachine> &rSM,
                                  const std::vector<std::tuple<std::string,
                                  std::vector<std::string>, std::vector<
                                  State::ActionLine_t> >> &stateList)
{
    rSM.reset (new StateMachine ());
    for (std::tuple<std::string, std::vector<std::string>, std::vector<
         State::ActionLine_t>> tup : stateList)
    {
        Error_t retState = rSM->addState (std::get<0> (tup), std::get<1> (tup),
                                          std::get<2> (tup));
        if (retState != E_SUCCESS)
        {
            return E_DUPLICATE_NAME;
        }
    }
    return E_SUCCESS;
}

Error_t StateMachine::addState (std::string stateName, 
                                const std::vector<std::string> 
                                &stateTransitions)
{
    // Allocate the state from parameter data, and create shared pointer
    std::shared_ptr<State> pNewState (new State (stateName, stateTransitions));
    // Check if pointer to current state is null; if so, then set as current
    if (mPStateCurrent == nullptr)
    {
        // overwrite memory of current state
        mPStateCurrent = pNewState;
    }
    // Insert returns pair containing bool; true if inserted, false if not.
    // Will not insert if there exists a duplicate key, aka duplicate name
    bool resultBool = (this->mStateMap).
        insert (std::make_pair (stateName, pNewState)).second;
    if (resultBool)
    {
        return E_SUCCESS;
    }
    else
    {
        return E_DUPLICATE_NAME;
    }
}

Error_t StateMachine::addState (std::string stateName,
                                const std::vector<std::string> 
                                &stateTransitions, const std::vector<std::tuple
                                <int32_t, Error_t (*) (int32_t), int32_t>> 
                                &actionList)
{
    // Allocate the state from parameter data, and create shared pointer
    std::shared_ptr<State> pNewState (new State (stateName, stateTransitions,
                                                 actionList));
    // Check if pointer to current state is null; if so, then set as current
    if (mPStateCurrent == nullptr)
    {
        // overwrite memory of current state
        mPStateCurrent = pNewState;
    }
    // Insert returns pair containing bool; true if inserted, false if not.
    // Will not insert if there exists a duplicate key, aka duplicate name
    bool resultBool = (this->mStateMap).
        insert (std::make_pair (stateName, pNewState)).second;
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
    Error_t ret = mPStateCurrent->getName (result);
    return ret;    
}

Error_t StateMachine::getCurrentStateTransitions (std::vector<std::string> 
                                                  &result)
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }
    Error_t ret = mPStateCurrent->getTransitions (result);
    return ret;
}

Error_t StateMachine::getCurrentActionSequence (std::map<int32_t,
                                                std::vector<std::tuple<
                                                Error_t (*) (int32_t), int32_t>
                                                > > &result)
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }
    Error_t ret = mPStateCurrent->getActionSequence (result);
    return ret;
}

Error_t StateMachine::executeCurrentSequence ()
{
    if (mPStateCurrent == nullptr)
    {
        return E_NO_STATES;
    }
    // Create a temp pointer to map, then point to address of action sequence
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        *pTempMap;
    Error_t ret = mPStateCurrent->getSequenceP (&pTempMap);
    if (ret != E_SUCCESS)
    {
        return ret;
    }
    // Iterate through map; guaranteed ordered by timestamp
    for (std::pair<int32_t, std::vector<std::tuple<Error_t (*) (int32_t),
        int32_t>>> vecPair : *pTempMap)
    {
        // Iterate through each vector element found in second in pair
        for (std::tuple<Error_t (*) (int32_t), int32_t> tup : vecPair.second)
        {
            // call function in pointer with implicit dereference
            ret = (std::get<0> (tup)) (std::get<1>(tup));
            // catch failure points in action sequence
            if (ret != E_SUCCESS)
            {
                return ret;
            }
        }
    }
    return E_SUCCESS;
}

Error_t StateMachine::switchState(std::string targetState)
{
    // Get the valid transitions (using intermediate function)
    std::vector<std::string> validTrans;
    Error_t ret = getCurrentStateTransitions(validTrans);
    // getter function only returns E_SUCCESS for now, if block for future use
    if (ret != E_SUCCESS)
    {
        return ret;
    }
    // Check if valid transitions contains the target state
    if (std::find (validTrans.begin (), validTrans.end (), targetState) !=
        validTrans.end())
    {   // if not equal to end, transition is valid; switch the current state
        // create a temporary empty state
        std::shared_ptr<State> stateResult;
        // get the target state from the state map and check if found
        Error_t ret = findState (stateResult, targetState);
        if (ret != E_SUCCESS)
        {
            return E_NAME_NOTFOUND;
        }
        // overwrite current state with the target state
        mPStateCurrent = stateResult;
        return E_SUCCESS;
    }
    else
    {
        // otherwise, transition is invalid; cannot switch to the new state
        return E_INVALID_TRANSITION;
    }
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine ()
{
    mPStateCurrent = nullptr;
}
