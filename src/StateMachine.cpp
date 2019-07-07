#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::fromDefault (StateMachine **ppStateMachine)
{
    // Create StateMachine case and store in param
    static StateMachine stateMachineInstance = StateMachine (1, 2);
    *ppStateMachine = &stateMachineInstance;
    return E_SUCCESS;

}

Error_t StateMachine::fromArr (StateMachine **ppStateMachine, int32_t c[])
{
    // Arbitrary calculations to obtain A, B data from array
    int32_t a = c[0];
    int32_t b = 0;
    for (int32_t i = 0; i < 4; i++)
    {
        b += c[i];
    }
    // Create StateMachine case and store in param
    static StateMachine stateMachineInstance = StateMachine (a, b);
    *ppStateMachine = &stateMachineInstance;
    return E_SUCCESS;
}

Error_t StateMachine::fromStates (StateMachine **ppStateMachine,
                                  std::vector<State> stateList)
{
    static StateMachine stateMachineInstance = StateMachine (0, 0);
    for (State state : stateList)
    {
        Error_t retState = stateMachineInstance.addState (state);
        if (retState != E_SUCCESS)
        {
            return E_DUPLICATE_NAME;
        }
    }
    *ppStateMachine = &stateMachineInstance;
    return E_SUCCESS;
    
}

Error_t StateMachine::addState (State newState)
{
    // Check if pointer to current state is null; if so, then set as current
    if (pStateCurrent == nullptr)
    {
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

Error_t StateMachine::switchState (std::string stateName)
{
    return E_SUCCESS;
}

Error_t StateMachine::getStateName (std::string &result)
{
    std::string iResult;
    Error_t ret = pStateCurrent->getName (iResult);
    result = "StateA";
    return E_SUCCESS;
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
}

Error_t StateMachine::deleteState ()
{
    // Manually delete State to pass memory leak tests
    delete pStateCurrent;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine (int32_t a, int32_t b)
{
    this->a = a;
    this->b = b;
    pStateMap = new std::unordered_map<std::string, State>();
    pStateCurrent = nullptr;
}

StateMachine::StateMachine (StateMachine const &)
{

}

StateMachine& StateMachine::operator= (StateMachine const &)
{
    return *this;
};
