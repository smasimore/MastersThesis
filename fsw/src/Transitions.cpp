#include "Transitions.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t Transitions::createNew (const Transitions::Config_t& kConfig,
                                std::shared_ptr<DataVector> kPDataVector,
                                std::shared_ptr<Transitions>& kPTransitionsRet)
{
    kPTransitionsRet.reset (new Transitions (kConfig, kPDataVector));
    
    // Verify config.
    Error_t ret = kPTransitionsRet->verifyConfig ();
    if (ret != E_SUCCESS)
    {
        kPTransitionsRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t Transitions::verifyConfig ()
{
    // Verify DV not null.
    if (mPDataVector == nullptr)
    {
        return E_DATA_VECTOR_NULL;
    }
    
    // Loop over transitions.
    for (std::shared_ptr<TransitionBase> transition : mTransitionsList)
    {
        // Verify comparison and state.
        if (transition->mComparison >= CMP_LAST || 
            transition->mTargetState >= STATE_LAST)
        {
            return E_INVALID_ENUM;
        }

        // Verify element exists in Data Vector.
        if (mPDataVector->elementExists (transition->mElem) != E_SUCCESS)
        {
            return E_INVALID_ELEM;
        }

        // Verify type matches elem type.
        DataVectorElementType_t type;
        if (mPDataVector->getElementType (transition->mElem, type) != E_SUCCESS)
        {
            return E_DATA_VECTOR_READ;
        }
        if (transition->mType != type)
        {
            return E_INCORRECT_TYPE;
        }
    }

    return E_SUCCESS;
}

Error_t Transitions::checkTransitions (bool& kShouldTransitionRet, 
                                       StateId_t& kTargetStateRet)
{
    for (std::shared_ptr<Transitions::TransitionBase> transition :
         mTransitionsList)
    {
        // Check for transition using member function.
        bool shouldTransition = false;
        StateId_t targetState;
        
        Error_t ret = transition->checkTransition (mPDataVector, 
                                                   shouldTransition,
                                                   targetState);
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Return after first transition condition met.
        if (shouldTransition == true)
        {
            kShouldTransitionRet = true;
            kTargetStateRet = targetState;
            return E_SUCCESS;
        }
    }

    // No transition condition met.
    kShouldTransitionRet = false;
    return E_SUCCESS;
}

bool Transitions::operator== (Transitions& rhs)
{
    // Verify both have same number of transitions.
    if (mTransitionsList.size () != rhs.mTransitionsList.size ())
    {
        return false;
    }

    // Verify TransitionBase attributes.
    for (uint32_t i = 0; i < mTransitionsList.size (); i++)
    {
        std::shared_ptr<TransitionBase> lhsTransition = mTransitionsList[i];
        std::shared_ptr<TransitionBase> rhsTransition = rhs.mTransitionsList[i];

        if (lhsTransition->mElem != rhsTransition->mElem ||
            lhsTransition->mComparison != rhsTransition->mComparison ||
            lhsTransition->mTargetState != rhsTransition->mTargetState ||
            lhsTransition->mType != rhsTransition->mType)
        {
            return false;
        }
    }

    return true;
}

/******************** PRIVATE FUNCTIONS *************************/

Transitions::Transitions (const Transitions::Config_t &kConfig,
                          std::shared_ptr<DataVector> kPDataVector) :
    mTransitionsList (kConfig),
    mPDataVector (kPDataVector)
{ }
