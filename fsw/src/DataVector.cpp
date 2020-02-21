#include <set>
#include <cstring>
#include <algorithm>
#include <iostream>

#include "DataVector.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t DataVector::createNew (DataVector::Config_t& kConfig,
                               std::shared_ptr<DataVector>& kPDataVectorRet)
{
    Error_t ret = E_SUCCESS;

    // Verify config.
    ret = DataVector::verifyConfig (kConfig);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Create Data Vector.
    kPDataVectorRet.reset (new DataVector (kConfig, ret));

    // Check for error on construct and free memory if it failed.
    if (ret != E_SUCCESS)
    {
        kPDataVectorRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t DataVector::getSizeBytesFromType (DataVectorElementType_t kType,
                                          uint8_t& kSizeBytesRet)
{
    switch (kType)
    {
        case DV_T_UINT8:
            kSizeBytesRet = sizeof (uint8_t);
            break;
        case DV_T_UINT16:
            kSizeBytesRet = sizeof (uint16_t);
            break;
        case DV_T_UINT32:
            kSizeBytesRet = sizeof (uint32_t);
            break;
        case DV_T_UINT64:
            kSizeBytesRet = sizeof (uint64_t);
            break;
        case DV_T_INT8:
            kSizeBytesRet = sizeof (int8_t);
            break;
        case DV_T_INT16:
            kSizeBytesRet = sizeof (int16_t);
            break;
        case DV_T_INT32:
            kSizeBytesRet = sizeof (int32_t);
            break;
        case DV_T_INT64:
            kSizeBytesRet = sizeof (int64_t);
            break;
        case DV_T_FLOAT:
            kSizeBytesRet = sizeof (float);
            break;
        case DV_T_DOUBLE:
            kSizeBytesRet = sizeof (double);
            break;
        case DV_T_BOOL:
            kSizeBytesRet = sizeof (bool);
            break;
        default:
            return E_INVALID_ENUM;
    }

    return E_SUCCESS;
}

Error_t DataVector::getDataVectorSizeBytes (uint32_t& kSizeBytesRet)
{
    kSizeBytesRet = mBuffer.size ();
    return E_SUCCESS;
}

Error_t DataVector::getRegionSizeBytes (DataVectorRegion_t kRegion,
                                        uint32_t& kSizeBytesRet)
{
    // Get region's info. If region not in Data Vector, return error.
    if (mRegionToRegionInfo.find (kRegion) == mRegionToRegionInfo.end ())
    {
        return E_INVALID_REGION;
    }

    // Store region's size in return param.
    RegionInfo_t* pRegionInfo = &mRegionToRegionInfo[kRegion];
    kSizeBytesRet = pRegionInfo->sizeBytes;

    return E_SUCCESS;
}

Error_t DataVector::getElementType (DataVectorElement_t kElem, 
                                    DataVectorElementType_t& kTypeRet)
{
    // Check if element in Data Vector.
    Error_t ret = this->elementExists (kElem); 
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    kTypeRet = mElementToElementInfo[kElem].type;
   
    return E_SUCCESS;
}

Error_t DataVector::elementExists (DataVectorElement_t kElem)
{
    if (mElementToElementInfo.find (kElem) == mElementToElementInfo.end ())
    {
        return E_INVALID_ELEM;
    }

    return E_SUCCESS;
}

Error_t DataVector::readRegion (DataVectorRegion_t kRegion, 
                                std::vector<uint8_t>& kRegionBufRet)
{
    Error_t ret = E_SUCCESS;

    // Get kRegion info. If kRegion not in Data Vector, return error.
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

Error_t DataVector::writeRegion (DataVectorRegion_t kRegion, 
                                 std::vector<uint8_t>& kRegionBuf)
{
    Error_t ret = E_SUCCESS;

    // Get kRegion info. If kRegion not in Data Vector, return error.
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

Error_t DataVector::readDataVector (std::vector<uint8_t>& kDataVectorBufRet)
{
    Error_t ret = E_SUCCESS;

    // Verify vector is same size as mBuffer.
    if (kDataVectorBufRet.size () != mBuffer.size ())
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
    kDataVectorBufRet = mBuffer;

    // Release lock. 
    return this->releaseLock ();
}


Error_t DataVector::writeDataVector (std::vector<uint8_t>& kDvBuf)
{
    Error_t ret = E_SUCCESS;

    // Verify passed in buffer is same size as Data Vector's underlying buffer.
    if (kDvBuf.size () != mBuffer.size ())
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
    std::vector<uint8_t>::iterator startIter = mBuffer.begin ();
    std::copy_n (kDvBuf.begin (), mBuffer.size (), startIter);

    // Release lock. 
    return this->releaseLock ();
}

Error_t DataVector::acquireLock ()
{
    if (pthread_mutex_lock (&mLock) != 0)
    {
        return E_FAILED_TO_LOCK;
    }

    return E_SUCCESS;
}

Error_t DataVector::releaseLock ()
{
    if (pthread_mutex_unlock (&mLock) != 0)
    {
        return E_FAILED_TO_UNLOCK;
    }

    return E_SUCCESS;
}

/**************************** PRIVATE FUNCTIONS *******************************/

DataVector::DataVector (DataVector::Config_t& kConfig, Error_t& kRet) :
    mConfig (kConfig)
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
        DataVectorRegion_t region = pRegionConfig->region;

        // 3a) Store current size of mBuffer as the starting index of the 
        //     current region.
        uint32_t regionStartIdx = mBuffer.size ();

        // 3b) Loop over the region's elements.
        uint32_t regionSizeBytes = 0;
        std::vector<DataVectorElement_t> elementsInRegion (pElemConfigs->size ());
        for (uint32_t elemIdx = 0; elemIdx < pElemConfigs->size (); elemIdx++)
        {
            // 3b i) Get pointer to element kConfig.
            ElementConfig_t* pElemConfig = &(pElemConfigs->at (elemIdx));

            // 3b ii) Get size of element.
            uint8_t elemSizeBytes;
            kRet = DataVector::getSizeBytesFromType (pElemConfig->type, 
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

            // 3b viii) Add element to elementsInRegion vector.
            elementsInRegion[elemIdx] = pElemConfig->elem;
        }
        
        // 3c) Create the region's info struct and add it to the global map.
        RegionInfo_t regionInfo;
        regionInfo.startIdx = regionStartIdx;
        regionInfo.sizeBytes = regionSizeBytes;
        mRegionToRegionInfo[region] = regionInfo;
    }
}

Error_t DataVector::verifyConfig (DataVector::Config_t& kConfig)
{
    // 1) Verify kConfig not empty.
    if (kConfig.size () == 0)
    {
        return E_EMPTY_CONFIG;
    }

    // 2) Verify elements list is not empty, elements are unique, and enums are 
    //    valid.
    std::set<DataVectorRegion_t>  regSet;
    std::set<DataVectorElement_t> elemSet;
    for (uint32_t i_region = 0; i_region < kConfig.size (); i_region++)
    {
        DataVector::RegionConfig_t               regConfig = kConfig[i_region];
        DataVectorRegion_t                       reg       = regConfig.region;
        std::vector<DataVector::ElementConfig_t> regElems  = regConfig.elems;

        // 2a) Verify region's elems list not empty.
        if (regElems.size () == 0)
        {
            return E_EMPTY_ELEMS;
        }

        // 2b) Verify valid region enum.
        if (reg >= DV_REG_LAST)
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
            DataVector::ElementConfig_t elemConfig = regElems[i_elem];
            DataVectorElement_t         elem       = elemConfig.elem;
            DataVectorElementType_t     elemType   = elemConfig.type;

            // 2d i) Verify valid elem enum.
            if (elem >= DV_ELEM_LAST)
            {
                return E_INVALID_ENUM;
            }

            // 2d ii) Verify valid type enum.
            if (elemType >= DV_T_LAST)
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

Error_t DataVector::initLock ()
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
