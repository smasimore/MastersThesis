#include "RecoveryIgniterController.hpp"

/***************************** PUBLIC FUNCTIONS *******************************/

Error_t RecoveryIgniterController::runEnabled ()
{
    // Get the current time.
    Time::TimeNs_t currTimeNs = 0;
    Error_t err = mPDataVector->read (mCONFIG.missionTimeElem, currTimeNs);
    if (err != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Disable igniter and return early if the recovery system is disarmed.
    bool armed = false;
    err = mPDataVector->read (mCONFIG.recArmedElem, armed);
    if (err != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }
    if (!armed)
    {
        return mPDataVector->write (mCONFIG.igniterControlElem, false);
    }

    // Check if a deployment time has been set.
    Time::TimeNs_t depTimeNs = 0;
    err = mPDataVector->read (mCONFIG.tDepTimeElem, depTimeNs);
    if (err != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // If yes, see if enough time has passed to turn off the igniter before
    // returning early.
    if (depTimeNs > 0)
    {
        // Disable igniter if sufficient time has elapsed since deployment.
        Time::TimeNs_t sinceDepNs = currTimeNs - depTimeNs;
        if (sinceDepNs >= IGNITION_DURATION_NS)
        {
            return mPDataVector->write (mCONFIG.igniterControlElem, false);
        }

        return E_SUCCESS;
    }

    // Return early if mission time is below deployment window lower bound.
    if (currTimeNs < mCONFIG.tDepBoundLowNs)
    {
        return E_SUCCESS;
    }

    // If the mission time is above deployment time window, it's time to deploy.
    bool deploy = false;
    if (currTimeNs > mCONFIG.tDepBoundHighNs)
    {
        deploy = true;
    }

    // Otherwise, see if the DV currently commands deployment.
    if (!deploy)
    {
        Error_t err = mPDataVector->read (mCONFIG.depCommandElem, deploy);
        if (err != E_SUCCESS)
        {
            return E_DATA_VECTOR_READ;
        }
    }

    // Make it so.
    if (deploy)
    {
        // Mark the time we're deploying at.
        err = mPDataVector->write (mCONFIG.tDepTimeElem, currTimeNs);
        if (err != E_SUCCESS)
        {
            return E_DATA_VECTOR_WRITE;
        }
        // Enable igniter.
        return mPDataVector->write (mCONFIG.igniterControlElem, true);
    }

    return E_SUCCESS;
}

Error_t RecoveryIgniterController::runSafed ()
{
    // Lower igniter device.
    if (mPDataVector->write (mCONFIG.igniterControlElem, false) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

/***************************** PRIVATE FUNCTIONS ******************************/

RecoveryIgniterController::RecoveryIgniterController (
        RecoveryIgniterController::Config_t& kConfig,
        std::shared_ptr<DataVector> kPDataVector,
        DataVectorElement_t kDvModeElem) :
    Controller (kPDataVector, kDvModeElem), mCONFIG (kConfig)
{
}

Error_t RecoveryIgniterController::verifyConfig ()
{
    // All configuration elements must exist in DV.
    if (mPDataVector->elementExists (mCONFIG.depCommandElem)     != E_SUCCESS ||
        mPDataVector->elementExists (mCONFIG.tDepTimeElem)       != E_SUCCESS ||
        mPDataVector->elementExists (mCONFIG.missionTimeElem)    != E_SUCCESS ||
        mPDataVector->elementExists (mCONFIG.igniterControlElem) != E_SUCCESS ||
        mPDataVector->elementExists (mCONFIG.recArmedElem)       != E_SUCCESS)
    {
        return E_INVALID_ELEM;
    }

    // Bounds must be in the correct order and lower bound must be nonzero.
    if (mCONFIG.tDepBoundLowNs == 0 ||
        mCONFIG.tDepBoundLowNs >= mCONFIG.tDepBoundHighNs)
    {
        return E_OUT_OF_BOUNDS;
    }

    return E_SUCCESS;
}