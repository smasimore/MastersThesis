#include "State.hpp"

/******************** PUBLIC FUNCTIONS **************************/

State::State ()
{
}

State::State (std::vector<int32_t> intData)
{
    State::stateData = intData;
}

State::State (std::string stateName, std::vector<std::string> targetTransitions)
{
    State::stateName = stateName;
    State::targetTransitions = targetTransitions;
}

Error_t State::printData ()
{
    for (int32_t i : StateData) {
        printf ("data@%" PRId32 ": %" PRId32 "\n", i, stateData[i]);
    }
    return E_SUCCESS;
}

Error_t State::getData (std::vector<int32_t> &result)
{
    result = stateData;
    return E_SUCCESS;
}