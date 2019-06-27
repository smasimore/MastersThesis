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

Error_t State::printData ()
{
    for (int32_t i : this->stateData)
    {
        //printf ("data@%" PRId32 ": %" PRId32 "\n", i, this->stateData[i]);
    }
    return E_SUCCESS;
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
