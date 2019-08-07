#include "State.hpp"

/******************** PUBLIC FUNCTIONS **************************/

State::State (std::string stateName,
              const std::vector<std::string> &targetTransitions)
{
    this->mStateName = stateName;
    this->mTargetTransitions = targetTransitions;
}

State::State (std::string stateName,
              const std::vector<std::string> &targetTransitions,
              const std::vector<State::ActionLine_t> &actionList)
{
    this->mStateName = stateName;
    this->mTargetTransitions = targetTransitions;
    // Parser would likely process each of the action sequence line by line.
    // Hence, there should be some intermediate logic to group the timestamps
    // such as how the action sequence is stored in a state.
    for (State::ActionLine_t tup : actionList)
    {
        // access the vector corresponding to the timestamp, then insert tuple
        int32_t timestamp = std::get<0> (tup);
        this->mActionSequence[timestamp].push_back (std::make_tuple (
            std::get<1> (tup), std::get<2> (tup)));
    }
}

Error_t State::getName (std::string &result)
{
    result = this->mStateName;
    return E_SUCCESS;
}

Error_t State::getTransitions (std::vector<std::string> &result)
{
    result = this->mTargetTransitions;
    return E_SUCCESS;
}

Error_t State::getActionSequence (std::map<int32_t, std::vector<Action_t>> 
                                  &result)
{
    result = this->mActionSequence;
    return E_SUCCESS;
}

Error_t State::getSequenceP (std::map<int32_t, std::vector<Action_t>> 
                             **ppResult)
{
    *ppResult = &mActionSequence;
    return E_SUCCESS;
}