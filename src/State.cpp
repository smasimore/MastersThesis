#include "State.hpp"

/******************** PUBLIC FUNCTIONS **************************/

State::State (std::string stateName,
              const std::vector<std::string> &validTransitions)
{
    this->mStateName = stateName;
    this->mValidTransitions = validTransitions;
}

State::State (std::string stateName,
              const std::vector<std::string> &validTransitions,
              const std::vector<State::Action_t> &actionList)
{
    this->mStateName = stateName;
    this->mValidTransitions = validTransitions;
    // Parser would likely process each of the action sequence line by line.
    // Hence, there should be some intermediate logic to group the timestamps
    // such as how the action sequence is stored in a state.
    for (State::Action action : actionList)
    {
        // access the vector corresponding to the timestamp, then insert tuple
        this->mActionSequence[action.timestamp].push_back (action);
    }
}

Error_t State::getName (std::string **ppResult)
{
    *ppResult = &mStateName;
    return E_SUCCESS;
}

Error_t State::getTransitions (std::vector<std::string> **ppResult)
{
    *ppResult = &mValidTransitions;
    return E_SUCCESS;
}

Error_t State::getActionSequence (std::map<int32_t, std::vector<Action_t>>
                                  **ppResult)
{
    *ppResult = &mActionSequence;
    return E_SUCCESS;
}