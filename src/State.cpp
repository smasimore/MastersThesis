#include "State.hpp"

/******************** PUBLIC FUNCTIONS **************************/

State::State (std::vector<int32_t> intData)
{
    this->stateData = intData;
}

State::State (std::string stateName,
              std::vector<std::string> targetTransitions)
{
    this->stateName = stateName;
    this->targetTransitions = targetTransitions;
}

Error_t State::getData (std::vector<int32_t> &result)
{
    result = this->stateData;
    return E_SUCCESS;
}

Error_t State::getName (std::string &result)
{
    result = this->stateName;
    return E_SUCCESS;
}

Error_t State::getTransitions (std::vector<std::string> &result)
{
    result = this->targetTransitions;
    return E_SUCCESS;
}
