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

State::State (std::string stateName,
              std::vector<std::string> targetTransitions,
              std::vector<std::tuple<int32_t, Error_t (*) (int32_t),
              int32_t >> actionList)
{
    this->stateName = stateName;
    this->targetTransitions = targetTransitions;
    // Parser would likely process each of the action sequence line by line.
    // Hence, there should be some intermediate logic to group the timestamps
    // such as how the action sequence is stored in a state.
    for (std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup : 
         actionList)
    {
        // access the vector corresponding to the timestamp, then insert tuple
        int32_t timestamp = std::get<0> (tup);
        this->actionSequence[timestamp].push_back (std::make_tuple (
            std::get<1> (tup), std::get<2> (tup)));
    }
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

Error_t State::getActionSequence (std::map<int32_t, std::vector<std::tuple<
                                  Error_t (*) (int32_t), int32_t> > > &result)
{
    result = this->actionSequence;
    return E_SUCCESS;
}