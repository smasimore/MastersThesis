#include "State.hpp"

/******************** PUBLIC FUNCTIONS **************************/

State::State (std::shared_ptr<DataVector> kPDataVector,
              StateId_t kStateId,
              const Transitions::Config_t &kTransitionsConfig,
              const Actions::Config_t &kActionsConfig,
              DataVectorElement_t kStateElem,
              Error_t& kRet)
{
    kRet = E_SUCCESS;
    this->mStateId = kStateId;

    // Create Transitions from config.
    kRet = Transitions::createNew (kTransitionsConfig, kPDataVector,
                                   mTransitions);
    if (kRet != E_SUCCESS)
    {
        return;
    }

    // Create Actions from config.
    kRet = Actions::createNew (kActionsConfig, kPDataVector, kStateElem, 
                               mActions);
    if (kRet != E_SUCCESS)
    {
        return;
    }
}

Error_t State::getId (StateId_t& kIdRet)
{
    kIdRet = mStateId;
    return E_SUCCESS;
}

Error_t State::getTransitions (std::shared_ptr<Transitions> &kPTransitionsRet)
{
    kPTransitionsRet = mTransitions;
    return E_SUCCESS;
}

Error_t State::getActions (std::shared_ptr<Actions>& kPActionsRet)
{
    kPActionsRet = mActions;
    return E_SUCCESS;
}
