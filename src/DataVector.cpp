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

Error_t DataVector::elementExists (DataVectorElement_t kElem)
{
    if (mElementToElementInfo.find (kElem) == mElementToElementInfo.end ())
    {
        return E_INVALID_ELEM;
    }

    return E_SUCCESS;
}

Error_t DataVector::printPretty ()
{
    Error_t ret = E_SUCCESS;

    // Add header.
    std::string printStr = "\n\nDATA VECTOR\n\n";

    // Loop over regions.
    for (std::pair<DataVectorRegion_t, RegionInfo_t> region : 
             mRegionToRegionInfo)
    {
        // Add region name.
        printStr += "REGION: ";
        std::string regionStr;
        printStr += regionEnumToString (region.first, regionStr) != E_SUCCESS
                        ? std::to_string (region.first) + "\n"
                        : regionStr + "\n";

        // Loop over elements
        printStr += "ELEMENTS: \n";
        for (DataVectorElement_t element : region.second.elements)
        {
            // Add element name. 
            std::string elementStr;
            printStr += elementEnumToString (element, elementStr) != E_SUCCESS
                            ? std::to_string (element) + "\t"
                            : elementStr + "\t";

            // Add element value.
            ret = this->appendElementValue (element, printStr);
            if (ret != E_SUCCESS)
            {
                return ret;
            }

            printStr += "\n";
        }

        printStr += "\n";
    }

    // Print string.
    std::cout << printStr << std::endl;

    return E_SUCCESS;
}

Error_t DataVector::printCsvHeader ()
{
    // Loop over regions.
    std::string printStr;
    for (std::pair<DataVectorRegion_t, RegionInfo_t> region : 
             mRegionToRegionInfo)
    {
        // Add region name.
        std::string regionStr;
        printStr += regionEnumToString (region.first, regionStr) != E_SUCCESS
                        ? std::to_string (region.first) + ","
                        : regionStr + ",";

        // Loop over elements.
        for (DataVectorElement_t element : region.second.elements)
        {
            // Add element name. 
            std::string elementStr;
            printStr += elementEnumToString (element, elementStr) != E_SUCCESS
                            ? std::to_string (element) + ","
                            : elementStr + ",";
        }
    }

    // Print string.
    std::cout << printStr << std::endl;

    return E_SUCCESS;
}

Error_t DataVector::printCsvRow ()
{
    Error_t ret = E_SUCCESS;

    // Loop over regions.
    std::string printStr;
    for (std::pair<DataVectorRegion_t, RegionInfo_t> region : 
             mRegionToRegionInfo)
    {
        // Add empty cell for region.
        printStr += ",";

        // Loop over elements
        for (DataVectorElement_t element : region.second.elements)
        {
            // Add element value.
            ret = this->appendElementValue (element, printStr);
            if (ret != E_SUCCESS)
            {
                return ret;
            }

            printStr += ",";
        }
    }

    // Print string.
    std::cout << printStr << std::endl;

    return E_SUCCESS;
}


/**************************** PRIVATE FUNCTIONS *******************************/

DataVector::DataVector (DataVector::Config_t& kConfig, Error_t& kRet)
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
        regionInfo.elements = elementsInRegion;
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

Error_t DataVector::regionEnumToString (DataVectorRegion_t kRegionEnum, 
                                        std::string& kRegionStrRet)
{
    switch (kRegionEnum)
    {
        case DV_REG_TEST0:
            kRegionStrRet = "DV_REG_TEST0";
            break;
        case DV_REG_TEST1:
            kRegionStrRet = "DV_REG_TEST1";
            break;
        case DV_REG_TEST2:
            kRegionStrRet = "DV_REG_TEST2";
            break;
        case DV_REG_LAST:
            kRegionStrRet = "DV_REG_LAST";
            break;
        default:
            return E_ENUM_STRING_UNDEFINED;
    }

    return E_SUCCESS;
}

Error_t DataVector::elementEnumToString (DataVectorElement_t kElementEnum,
                                         std::string& kElementStrRet)
{
    switch (kElementEnum)
    {
        case DV_ELEM_TEST0:
            kElementStrRet = "DV_ELEM_TEST0";
            break;
        case DV_ELEM_TEST1:
            kElementStrRet = "DV_ELEM_TEST1";
            break;
        case DV_ELEM_TEST2:
            kElementStrRet = "DV_ELEM_TEST2";
            break;
        case DV_ELEM_TEST3:
            kElementStrRet = "DV_ELEM_TEST3";
            break;
        case DV_ELEM_TEST4:
            kElementStrRet = "DV_ELEM_TEST4";
            break;
        case DV_ELEM_TEST5:
            kElementStrRet = "DV_ELEM_TEST5";
            break;
        case DV_ELEM_TEST6:
            kElementStrRet = "DV_ELEM_TEST6";
            break;
        case DV_ELEM_TEST7:
            kElementStrRet = "DV_ELEM_TEST7";
            break;
        case DV_ELEM_TEST8:
            kElementStrRet = "DV_ELEM_TEST8";
            break;
        case DV_ELEM_TEST9:
            kElementStrRet = "DV_ELEM_TEST9";
            break;
        case DV_ELEM_TEST10:
            kElementStrRet = "DV_ELEM_TEST10";
            break;
        case DV_ELEM_TEST11:
            kElementStrRet = "DV_ELEM_TEST11";
            break;
        case DV_ELEM_TEST12:
            kElementStrRet = "DV_ELEM_TEST12";
            break;
        case DV_ELEM_TEST13:
            kElementStrRet = "DV_ELEM_TEST13";
            break;
        case DV_ELEM_TEST14:
            kElementStrRet = "DV_ELEM_TEST14";
            break;
        case DV_ELEM_TEST15:
            kElementStrRet = "DV_ELEM_TEST15";
            break;
        case DV_ELEM_TEST16:
            kElementStrRet = "DV_ELEM_TEST16";
            break;
        case DV_ELEM_TEST17:
            kElementStrRet = "DV_ELEM_TEST17";
            break;
        case DV_ELEM_TEST18:
            kElementStrRet = "DV_ELEM_TEST18";
            break;
        case DV_ELEM_TEST19:
            kElementStrRet = "DV_ELEM_TEST19";
            break;
        case DV_ELEM_TEST20:
            kElementStrRet = "DV_ELEM_TEST20";
            break;
        case DV_ELEM_TEST21:
            kElementStrRet = "DV_ELEM_TEST21";
            break;
        case DV_ELEM_TEST22:
            kElementStrRet = "DV_ELEM_TEST22";
            break;
        case DV_ELEM_TEST23:
            kElementStrRet = "DV_ELEM_TEST23";
            break;
        case DV_ELEM_TEST24:
            kElementStrRet = "DV_ELEM_TEST24";
            break;
        case DV_ELEM_TEST25:
            kElementStrRet = "DV_ELEM_TEST25";
            break;
        case DV_ELEM_TEST26:
            kElementStrRet = "DV_ELEM_TEST26";
            break;
        case DV_ELEM_TEST27:
            kElementStrRet = "DV_ELEM_TEST27";
            break;
        case DV_ELEM_TEST28:
            kElementStrRet = "DV_ELEM_TEST28";
            break;
        case DV_ELEM_TEST29:
            kElementStrRet = "DV_ELEM_TEST29";
            break;
        case DV_ELEM_TEST30:
            kElementStrRet = "DV_ELEM_TEST30";
            break;
        case DV_ELEM_TEST31:
            kElementStrRet = "DV_ELEM_TEST31";
            break;
        case DV_ELEM_TEST32:
            kElementStrRet = "DV_ELEM_TEST32";
            break;
        case DV_ELEM_TEST33:
            kElementStrRet = "DV_ELEM_TEST33";
            break;
        case DV_ELEM_TEST34:
            kElementStrRet = "DV_ELEM_TEST34";
            break;
        case DV_ELEM_TEST35:
            kElementStrRet = "DV_ELEM_TEST35";
            break;
        case DV_ELEM_TEST36:
            kElementStrRet = "DV_ELEM_TEST36";
            break;
        case DV_ELEM_TEST37:
            kElementStrRet = "DV_ELEM_TEST37";
            break;
        case DV_ELEM_TEST38:
            kElementStrRet = "DV_ELEM_TEST38";
            break;
        case DV_ELEM_TEST39:
            kElementStrRet = "DV_ELEM_TEST39";
            break;
        case DV_ELEM_TEST40:
            kElementStrRet = "DV_ELEM_TEST40";
            break;
        case DV_ELEM_TEST41:
            kElementStrRet = "DV_ELEM_TEST41";
            break;
        case DV_ELEM_TEST42:
            kElementStrRet = "DV_ELEM_TEST42";
            break;
        case DV_ELEM_TEST43:
            kElementStrRet = "DV_ELEM_TEST43";
            break;
        case DV_ELEM_TEST44:
            kElementStrRet = "DV_ELEM_TEST44";
            break;
        case DV_ELEM_TEST45:
            kElementStrRet = "DV_ELEM_TEST45";
            break;
        case DV_ELEM_TEST46:
            kElementStrRet = "DV_ELEM_TEST46";
            break;
        case DV_ELEM_LAST:
            kElementStrRet = "DV_ELEM_LAST";
            break;
        default:
            return E_ENUM_STRING_UNDEFINED;
    }

    return E_SUCCESS;
}

Error_t DataVector::appendElementValue (DataVectorElement_t kElem,
                                        std::string& kStr)
{
    // Check if element in Data Vector.
    Error_t ret = E_SUCCESS;
    if (this->elementExists (kElem) != E_SUCCESS)
    {
        return ret;
    }

    // Since kElems have different types, get the type of this kElem
    // and switch based on type.
    ElementInfo_t kElemInfo = mElementToElementInfo[kElem];
    DataVectorElementType_t type = kElemInfo.type;
    
    uint8_t valUInt8 = 0;
    uint16_t valUInt16 = 0;
    uint32_t valUInt32 = 0;
    uint64_t valUInt64 = 0;
    int8_t valInt8 = 0;
    int16_t valInt16 = 0;
    int32_t valInt32 = 0;
    int64_t valInt64 = 0;
    float valFloat = 0;
    double valDouble = 0;
    bool valBool = false;

    switch (type)
    {
        case DV_T_UINT8:
            ret = this->read (kElem, valUInt8);
            kStr += std::to_string (valUInt8);
            break;
        case DV_T_UINT16:
            ret = this->read (kElem, valUInt16);
            kStr += std::to_string (valUInt16);
            break;
        case DV_T_UINT32:
            ret = this->read (kElem, valUInt32);
            kStr += std::to_string (valUInt32);
            break;
        case DV_T_UINT64:
            ret = this->read (kElem, valUInt64);
            kStr += std::to_string (valUInt64);
            break;
        case DV_T_INT8:
            ret = this->read (kElem, valInt8);
            kStr += std::to_string (valInt8);
            break;
        case DV_T_INT16:
            ret = this->read (kElem, valInt16);
            kStr += std::to_string (valInt16);
            break;
        case DV_T_INT32:
            ret = this->read (kElem, valInt32);
            kStr += std::to_string (valInt32);
            break;
        case DV_T_INT64:
            ret = this->read (kElem, valInt64);
            kStr += std::to_string (valInt64);
            break;
        case DV_T_FLOAT:
            ret = this->read (kElem, valFloat);
            kStr += std::to_string (valFloat);
            break;
        case DV_T_DOUBLE:
            ret = this->read (kElem, valDouble);
            kStr += std::to_string (valDouble);
            break;
        case DV_T_BOOL:
            ret = this->read (kElem, valBool);
            kStr += valBool == true ? "true" : "false";
            break;
        default:
            return E_INVALID_TYPE;
    }

    return ret;
}
