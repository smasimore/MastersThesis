#include "State.hpp"

/******************** PUBLIC FUNCTIONS **************************/

State::State (std::vector<int32_t> int_data)
{
    StateData = int_data;
}

Error_t State::printData ()
{
    for (int32_t i : StateData) {
        printf ("data@%" PRId32 ": %" PRId32 "\n", i, StateData[i]);
    }
    return E_SUCCESS;
}

Error_t State::getData (std::vector<int32_t> &result)
{
    result = StateData;
    return E_SUCCESS;
}