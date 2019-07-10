#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::fromDefault (std::unique_ptr<StateMachine> &rSM)
{
    // reset the smart pointer, using corresponding default case
    rSM.reset (new StateMachine (1, 2));
    return E_SUCCESS;

}

Error_t StateMachine::fromArr (std::unique_ptr<StateMachine> &rSM, int32_t c[])
{
    // Arbitrary calculations to obtain A, B data from array
    int32_t a = c[0];
    int32_t b = 0;
    for (int32_t i = 0; i < 4; i++)
    {
        b += c[i];
    }
    // reset the smart pointer, using case generated from array
    rSM.reset (new StateMachine (a, b));
    return E_SUCCESS;
}

Error_t StateMachine::fromStates (std::unique_ptr<StateMachine> &rSM,
                                  std::vector<State> stateList)
{
    rSM.reset (new StateMachine (0, 0));
    for (State state : stateList)
    {
        Error_t retState = rSM->addState (state);
        if (retState != E_SUCCESS)
        {
            return E_DUPLICATE_NAME;
        }
    }
    return E_SUCCESS;
    
}

Error_t StateMachine::addState (State newState)
{
    // Check if pointer to current state is null; if so, then set as current
    if (pStateCurrent == nullptr)
    {
        // Allocate necessary memory for a State, then overwrite it 
        pStateCurrent = new State("", {});
        *pStateCurrent = newState;
    }
    // Add the state to unordered_map using State object and State name
    std::string stateName;
    newState.getName (stateName);
    // Insert returns pair containing bool; true if inserted, false if not.
    // Will not insert if there exists a duplicate key, aka duplicate name
    auto resultPair = (this->pStateMap)->insert (std::make_pair (stateName,
                                                                newState));
    bool resultBool = std::get<1> (resultPair);
    if (resultBool)
    {
        return E_SUCCESS;
    }
    else
    {
        return E_DUPLICATE_NAME;
    }
}

Error_t StateMachine::findState (State &stateResult, std::string stateName)
{
    // search the unordered map
    auto search = pStateMap->find (stateName);
    // if element is not found, will point to end of the map
    if (search == pStateMap->end ())
    {
        return E_NAME_NOTFOUND;
    }
    else
    {
        stateResult = search->second;
        return E_SUCCESS;
    }
}

Error_t StateMachine::getStateName (std::string &result)
{
    Error_t ret = pStateCurrent->getName (result);
    return ret;
}

Error_t StateMachine::getStateTransitions (std::vector<std::string> &result)
{
    Error_t ret = pStateCurrent->getTransitions (result);
    return ret;
}

Error_t StateMachine::switchState(std::string targetState)
{
    // Get the valid transitions (using intermediate function)
    std::vector<std::string> validTrans;
    getStateTransitions(validTrans);

    // Check if valid transitions contains the target state
    if (std::find (validTrans.begin (), validTrans.end (), targetState) !=
        validTrans.end())
    {
        // if not equal to end, transition is valid; switch the current state
        // create a temporary empty state
        State stateResult ("", {});
        // get the target state from the state map and check if found
        Error_t ret = findState (stateResult, targetState);
        if (ret != E_SUCCESS)
        {
            return E_NAME_NOTFOUND;
        }
        // overwrite current state with the target state
        *pStateCurrent = stateResult;
        return E_SUCCESS;
    }
    else
    {
        // otherwise, transition is invalid; cannot switch to the new state
        return E_INVALID_TRANSITION;
    }
}

Error_t StateMachine::getA (int32_t &result)
{
    result = this->a;
    return E_SUCCESS;
}

Error_t StateMachine::getB (int32_t &result)
{
    result = this->b;
    return E_SUCCESS;
}

Error_t StateMachine::deleteMap ()
{
    // Manually delete State Map to pass memory leak tests
    delete pStateMap;
    return E_SUCCESS;
}

Error_t StateMachine::deleteState ()
{
    // Manually delete State to pass memory leak tests
    delete pStateCurrent;
    return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine (int32_t a, int32_t b)
{
    this->a = a;
    this->b = b;
    pStateMap = new std::unordered_map<std::string, State>();
    pStateCurrent = nullptr;
}
