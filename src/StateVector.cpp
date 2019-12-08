#include <set>
#include <cstring>
#include <algorithm>

#include "StateVector.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t StateVector::createNew (StateVector::StateVectorConfig_t& kConfig,
                                std::shared_ptr<StateVector>& kPStateVectorRet)
{
    Error_t ret = E_SUCCESS;

    // Verify config.
    ret = StateVector::verifyConfig (kConfig);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Create State Vector.
    kPStateVectorRet.reset (new StateVector (kConfig, ret));

    // Check for error on construct and free memory if it failed.
    if (ret != E_SUCCESS)
    {
        kPStateVectorRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t StateVector::getSizeBytesFromType (StateVectorElementType_t kType,
                                           uint8_t& kSizeBytesRet)
{
    switch (kType)
    {
        case SV_T_UINT8:
            kSizeBytesRet = sizeof (uint8_t);
            break;
        case SV_T_UINT16:
            kSizeBytesRet = sizeof (uint16_t);
            break;
        case SV_T_UINT32:
            kSizeBytesRet = sizeof (uint32_t);
            break;
        case SV_T_UINT64:
            kSizeBytesRet = sizeof (uint64_t);
            break;
        case SV_T_INT8:
            kSizeBytesRet = sizeof (int8_t);
            break;
        case SV_T_INT16:
            kSizeBytesRet = sizeof (int16_t);
            break;
        case SV_T_INT32:
            kSizeBytesRet = sizeof (int32_t);
            break;
        case SV_T_INT64:
            kSizeBytesRet = sizeof (int64_t);
            break;
        case SV_T_FLOAT:
            kSizeBytesRet = sizeof (float);
            break;
        case SV_T_DOUBLE:
            kSizeBytesRet = sizeof (double);
            break;
        case SV_T_BOOL:
            kSizeBytesRet = sizeof (bool);
            break;
        default:
            return E_INVALID_ENUM;
    }

    return E_SUCCESS;
}

Error_t StateVector::getStateVectorSizeBytes (uint32_t& kSizeBytesRet)
{
    kSizeBytesRet = mBuffer.size ();
    return E_SUCCESS;
}

Error_t StateVector::getRegionSizeBytes (StateVectorRegion_t kRegion,
                                         uint32_t& kSizeBytesRet)
{
    // Get region's info. If region not in State Vector, return error.
    if (mRegionToRegionInfo.find (kRegion) == mRegionToRegionInfo.end ())
    {
        return E_INVALID_REGION;
    }

    // Store region's size in return param.
    RegionInfo_t* pRegionInfo = &mRegionToRegionInfo[kRegion];
    kSizeBytesRet = pRegionInfo->sizeBytes;

    return E_SUCCESS;
}

Error_t StateVector::readRegion (StateVectorRegion_t kRegion, 
                                 std::vector<uint8_t>& kRegionBufRet)
{
    Error_t ret = E_SUCCESS;

    // Get kRegion info. If kRegion not in State Vector, return error.
    if (mRegionToRegionInfo.find (kRegion) == mRegionToRegionInfo.end ())
    {
        return E_INVALID_REGION;
    }
    RegionInfo_t* pRegionInfo = &mRegionToRegionInfo[kRegion];

    // Verify vector is same size as region's underlying buffer.
    if (kRegionBufRet.size () != pRegionInfo->sizeBytes)
    {
        return E_INCORRECT_SIZE;
    }

    // Acquire lock.
    ret = this->acquireLock (); 
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Copy buffer.
    std::vector<uint8_t>::iterator startIter = mBuffer.begin () + 
                                                   pRegionInfo->startIdx;
    std::copy_n (startIter, pRegionInfo->sizeBytes, kRegionBufRet.begin ());

    // Release lock. 
    return this->releaseLock ();
}

Error_t StateVector::writeRegion (StateVectorRegion_t kRegion, 
                                  std::vector<uint8_t>& kRegionBuf)
{
    Error_t ret = E_SUCCESS;

    // Get kRegion info. If kRegion not in State Vector, return error.
    if (mRegionToRegionInfo.find (kRegion) == mRegionToRegionInfo.end ())
    {
        return E_INVALID_REGION;
    }
    RegionInfo_t* pRegionInfo = &mRegionToRegionInfo[kRegion];

    // Verify vector is same size as region's underlying buffer.
    if (kRegionBuf.size () != pRegionInfo->sizeBytes)
    {
        return E_INCORRECT_SIZE;
    }

    // Acquire lock.
    ret = this->acquireLock (); 
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Copy buffer.
    std::vector<uint8_t>::iterator startIter = mBuffer.begin () + 
                                                   pRegionInfo->startIdx;
    std::copy_n (kRegionBuf.begin (), pRegionInfo->sizeBytes, startIter);

    // Release lock. 
    return this->releaseLock ();
}

Error_t StateVector::readStateVector (std::vector<uint8_t>& kStateVectorBufRet)
{
    Error_t ret = E_SUCCESS;

    // Verify vector is same size as mBuffer.
    if (kStateVectorBufRet.size () != mBuffer.size ())
    {
        return E_INCORRECT_SIZE;
    }

    // Acquire lock.
    ret = this->acquireLock (); 
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Copy buffer.
    kStateVectorBufRet = mBuffer;

    // Release lock. 
    return this->releaseLock ();
}

Error_t StateVector::acquireLock ()
{
    if (pthread_mutex_lock (&mLock) != 0)
    {
        return E_FAILED_TO_LOCK;
    }

    return E_SUCCESS;
}

Error_t StateVector::releaseLock ()
{
    if (pthread_mutex_unlock (&mLock) != 0)
    {
        return E_FAILED_TO_UNLOCK;
    }

    return E_SUCCESS;
}

/**************************** PRIVATE FUNCTIONS *******************************/

StateVector::StateVector (StateVector::StateVectorConfig_t& kConfig, 
                          Error_t& kRet)
{
    // 1) Set kReturn value to success.
    kRet = E_SUCCESS;

    // 2) Initialize lock.
    kRet = this->initLock ();
    if (kRet != E_SUCCESS)
    {
        return;
    }

    // 3) Loop over each region, adding the region's element data to mBuffer
    //    and build supporting datastructures mRegionToRegionInfo and
    //    mElementToElementInfo.
    for (uint32_t regionIdx = 0; regionIdx < kConfig.size (); regionIdx++)
    {
        RegionConfig_t* pRegionConfig = &(kConfig[regionIdx]);
        std::vector<ElementConfig_t>* pElemConfigs = &(pRegionConfig->elems);
        StateVectorRegion_t region = pRegionConfig->region;

        // 3a) Store current size of mBuffer as the starting index of the 
        //     current region.
        uint32_t regionStartIdx = mBuffer.size ();

        // 3b) Loop over the region's elements.
        uint32_t regionSizeBytes = 0;
        for (uint32_t elemIdx = 0; elemIdx < pElemConfigs->size (); elemIdx++)
        {
            // 3b i) Get pointer to element kConfig.
            ElementConfig_t* pElemConfig = &(pElemConfigs->at (elemIdx));

            // 3b ii) Get size of element.
            uint8_t elemSizeBytes;
            kRet = StateVector::getSizeBytesFromType (pElemConfig->type, 
                                                     elemSizeBytes);
            if (kRet != E_SUCCESS)
            {
                return;
            }

            // 3b iii) Get element's starting index into mBuffer (always 
            //         append).
            uint32_t elemStartIdx = mBuffer.size ();

            // 3b iv) Increase the size of mBuffer to make room for the new 
            //         element.
            mBuffer.resize (elemStartIdx + elemSizeBytes);

            // 3b v) Copy the element's initial value to mBuffer. This step
            //       assumes the CPU uses little endian byte ordering.
            uint8_t* pElemStart = &mBuffer[elemStartIdx];
            std::memcpy (pElemStart, &pElemConfig->initialVal, elemSizeBytes);

            // 3b vi) Update the running count of the region's size.
            regionSizeBytes += elemSizeBytes;

            // 3b vii) Create the element's info struct and add it to the global
            //         map. 
            ElementInfo_t elementInfo;
            elementInfo.startIdx = elemStartIdx;
            elementInfo.type = pElemConfig->type;
            mElementToElementInfo[pElemConfig->elem] = elementInfo;
        }
        
        // 3c) Create the region's info struct and add it to the global map.
        RegionInfo_t regionInfo;
        regionInfo.startIdx = regionStartIdx;
        regionInfo.sizeBytes = regionSizeBytes;
        mRegionToRegionInfo[region] = regionInfo;
    }
}

Error_t StateVector::verifyConfig (StateVector::StateVectorConfig_t& kConfig)
{
    // 1) Verify kConfig not empty.
    if (kConfig.size () == 0)
    {
        return E_EMPTY_CONFIG;
    }

    // 2) Verify elements list is not empty, elements are unique, and enums are 
    //    valid.
    std::set<StateVectorRegion_t>  regSet;
    std::set<StateVectorElement_t> elemSet;
    for (uint32_t i_region = 0; i_region < kConfig.size (); i_region++)
    {
        StateVector::RegionConfig_t               regConfig = kConfig[i_region];
        StateVectorRegion_t                       reg       = regConfig.region;
        std::vector<StateVector::ElementConfig_t> regElems  = regConfig.elems;

        // 2a) Verify region's elems list not empty.
        if (regElems.size () == 0)
        {
            return E_EMPTY_ELEMS;
        }

        // 2b) Verify valid region enum.
        if (reg >= SV_REG_LAST)
        {
            return E_INVALID_ENUM;
        }

        // 2c) Insert into region set. If region already in set, return error.
        if ((regSet.insert (reg)).second == false)
        {
            return E_DUPLICATE_REGION;
        }

        // 2d) Loop through elements.
        for (uint32_t i_elem = 0; i_elem < regElems.size (); i_elem++)
        {
            StateVector::ElementConfig_t elemConfig = regElems[i_elem];
            StateVectorElement_t         elem       = elemConfig.elem;
            StateVectorElementType_t     elemType   = elemConfig.type;

            // 2d i) Verify valid elem enum.
            if (elem >= SV_ELEM_LAST)
            {
                return E_INVALID_ENUM;
            }

            // 2d ii) Verify valid type enum.
            if (elemType >= SV_T_LAST)
            {
                return E_INVALID_ENUM;
            }

            // 2d iii) Insert into elem set.
            if ((elemSet.insert (elem)).second == false)
            {
                return E_DUPLICATE_ELEM;
            }
        }
    }

    return E_SUCCESS;
}

Error_t StateVector::initLock ()
{
    // Initialize lock as PTHREAD_MUTEX_ERRORCHECK type to prevent deadlock
    // if a thread attempts to lock twice without an unlock in between. 
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init (&attr) != 0 ||
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
        return E_FAILED_TO_INIT_LOCK;
    }

    // Initialize lock.
    if (pthread_mutex_init (&mLock, &attr) != 0)
    {
        pthread_mutexattr_destroy (&attr);
        return E_FAILED_TO_INIT_LOCK;
    }

    // Ignore possible error here, since repurcussion is negligible loss of 
    // memory. 
    pthread_mutexattr_destroy (&attr);

    return E_SUCCESS;
}
