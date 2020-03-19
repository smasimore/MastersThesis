#include "Actions.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t Actions::createNew (const Config_t& kConfig,
                            std::shared_ptr<DataVector> kPDataVector,
                            DataVectorElement_t kStateElem,
                            std::shared_ptr<Actions>& kPActionsRet)
{
    kPActionsRet.reset (new Actions (kConfig, kPDataVector));
    
    // Verify config.
    Error_t ret = kPActionsRet->verifyConfig (kStateElem);
    if (ret != E_SUCCESS)
    {
        kPActionsRet.reset ();
        return ret;
    }
    return E_SUCCESS;
}

Error_t Actions::verifyConfig (DataVectorElement_t kStateElem)
{
    // Verify DV not null.
    if (mPDataVector == nullptr)
    {
        return E_DATA_VECTOR_NULL;
    }
    
    // Loop over time => actions.
    for (std::pair<Time::TimeNs_t, 
                   std::vector<std::shared_ptr<ActionBase>>> mapPair : 
             mTimeToActionsMap)
    {
        // Loop over actions.
        for (std::shared_ptr<ActionBase> action : mapPair.second)
        {
            // Verify element exists in Data Vector.
            if (mPDataVector->elementExists (action->mElem) != E_SUCCESS)
            {
                return E_INVALID_ELEM;
            }

            // Verify type matches elem type.
            DataVectorElementType_t type;
            if (mPDataVector->getElementType (action->mElem, type) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }
            if (action->mType != type)
            {
                return E_INCORRECT_TYPE;
            }

            // Verify action is not attempting to change state.
            if (action->mElem == kStateElem)
            {
                return E_INVALID_ACTION;
            }
        }
    }

    return E_SUCCESS;
}

Error_t Actions::checkActions (Time::TimeNs_t kTimeElapsedNs,
                               std::vector<std::shared_ptr<ActionBase>>&
                               kActionsToExecuteRet)
{
    // Clear actions return.
    kActionsToExecuteRet.clear ();
    
    // Check if any actions should be executed.
    while (mActionIter != mActionEnd && mActionIter->first <= kTimeElapsedNs)
    {
        // Add each action to return vector.
        for (std::shared_ptr<ActionBase> action : mActionIter->second)
        {
            kActionsToExecuteRet.push_back (action);
        }

        mActionIter++;
    }
    return E_SUCCESS;
}

Error_t Actions::resetActionIterator ()
{
    mActionIter = mTimeToActionsMap.begin ();
    return E_SUCCESS;
}

bool Actions::operator== (Actions& rhs)
{
    // Verify both have same number of actions.
    if (mTimeToActionsMap.size () != rhs.mTimeToActionsMap.size ())
    {
        return false;
    }

    // Loop over time => actions elements.
    for (std::pair<Time::TimeNs_t, 
                   std::vector<std::shared_ptr<ActionBase>>> lhsPair : 
             mTimeToActionsMap)
    {
        // Verify same time key exists in rhs.
        if (rhs.mTimeToActionsMap.find (lhsPair.first) == 
            rhs.mTimeToActionsMap.end ())
        {
            return false;
        }

        // Verify same number of actions.
        if (lhsPair.second.size () != 
            rhs.mTimeToActionsMap[lhsPair.first].size ())
        {
            return false;
        }

        // Loop over actions and verify base attributes.
        for (uint32_t i = 0; i < lhsPair.second.size (); i++)
        {
            std::shared_ptr<Actions::ActionBase> lhsAction = lhsPair.second[i];
            std::shared_ptr<Actions::ActionBase> rhsAction = 
                rhs.mTimeToActionsMap[lhsPair.first][i];
            if (lhsAction->mElem != rhsAction->mElem ||
                lhsAction->mType != rhsAction->mType)
            {
                return false;
            }
        }
    }

    return true;
}


/******************** PRIVATE FUNCTIONS *************************/

/*
 * Private constructor to enforce createNew function
 */
Actions::Actions (const Actions::Config_t &kConfig,
                  std::shared_ptr<DataVector> kPDataVector) :
    mTimeToActionsMap (kConfig),
    mPDataVector (kPDataVector)
{
    mActionIter = mTimeToActionsMap.begin ();
    mActionEnd = mTimeToActionsMap.end ();
}
