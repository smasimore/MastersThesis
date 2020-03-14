#include <set>
#include <cstring>
#include <algorithm>

#include "DataVectorLogger.hpp"

const uint32_t DataVectorLogger::WATCH_ELEM_VALUE_START_POS = 33;

const std::unordered_map<DataVectorRegion_t, 
                         std::string, 
                         EnumClassHash> DataVectorLogger::mRegionToStr =
{
    {DV_REG_TEST0, "DV_REG_TEST0"},
    {DV_REG_TEST1, "DV_REG_TEST1"},
    {DV_REG_TEST2, "DV_REG_TEST2"},
};

const std::unordered_map<DataVectorElement_t, 
                         std::string, 
                         EnumClassHash> DataVectorLogger::mElemToStr =
{
    {DV_ELEM_TEST0,                  "DV_ELEM_TEST0"                  },
    {DV_ELEM_TEST1,                  "DV_ELEM_TEST1"                  },
    {DV_ELEM_TEST2,                  "DV_ELEM_TEST2"                  },
    {DV_ELEM_TEST3,                  "DV_ELEM_TEST3"                  },
    {DV_ELEM_TEST4,                  "DV_ELEM_TEST4"                  },
    {DV_ELEM_TEST5,                  "DV_ELEM_TEST5"                  },
    {DV_ELEM_TEST6,                  "DV_ELEM_TEST6"                  },
    {DV_ELEM_TEST7,                  "DV_ELEM_TEST7"                  },
    {DV_ELEM_TEST8,                  "DV_ELEM_TEST8"                  },
    {DV_ELEM_TEST9,                  "DV_ELEM_TEST9"                  },
    {DV_ELEM_TEST10,                 "DV_ELEM_TEST10"                 },
    {DV_ELEM_TEST11,                 "DV_ELEM_TEST11"                 },
    {DV_ELEM_TEST12,                 "DV_ELEM_TEST12"                 },
    {DV_ELEM_TEST13,                 "DV_ELEM_TEST13"                 },
    {DV_ELEM_TEST14,                 "DV_ELEM_TEST14"                 },
    {DV_ELEM_TEST15,                 "DV_ELEM_TEST15"                 },
    {DV_ELEM_TEST16,                 "DV_ELEM_TEST16"                 },
    {DV_ELEM_TEST17,                 "DV_ELEM_TEST17"                 },
    {DV_ELEM_TEST18,                 "DV_ELEM_TEST18"                 },
    {DV_ELEM_TEST19,                 "DV_ELEM_TEST19"                 },
    {DV_ELEM_TEST20,                 "DV_ELEM_TEST20"                 },
    {DV_ELEM_TEST21,                 "DV_ELEM_TEST21"                 },
    {DV_ELEM_TEST22,                 "DV_ELEM_TEST22"                 },
    {DV_ELEM_TEST23,                 "DV_ELEM_TEST23"                 },
    {DV_ELEM_TEST24,                 "DV_ELEM_TEST24"                 },
    {DV_ELEM_TEST25,                 "DV_ELEM_TEST25"                 },
    {DV_ELEM_TEST26,                 "DV_ELEM_TEST26"                 },
    {DV_ELEM_TEST27,                 "DV_ELEM_TEST27"                 },
    {DV_ELEM_TEST28,                 "DV_ELEM_TEST28"                 },
    {DV_ELEM_TEST29,                 "DV_ELEM_TEST29"                 },
    {DV_ELEM_TEST30,                 "DV_ELEM_TEST30"                 },
    {DV_ELEM_TEST31,                 "DV_ELEM_TEST31"                 },
    {DV_ELEM_TEST32,                 "DV_ELEM_TEST32"                 },
    {DV_ELEM_TEST33,                 "DV_ELEM_TEST33"                 },
    {DV_ELEM_TEST34,                 "DV_ELEM_TEST34"                 },
    {DV_ELEM_TEST35,                 "DV_ELEM_TEST35"                 },
    {DV_ELEM_TEST36,                 "DV_ELEM_TEST36"                 },
    {DV_ELEM_TEST37,                 "DV_ELEM_TEST37"                 },
    {DV_ELEM_TEST38,                 "DV_ELEM_TEST38"                 },
    {DV_ELEM_TEST39,                 "DV_ELEM_TEST39"                 },
    {DV_ELEM_TEST40,                 "DV_ELEM_TEST40"                 },
    {DV_ELEM_TEST41,                 "DV_ELEM_TEST41"                 },
    {DV_ELEM_TEST42,                 "DV_ELEM_TEST42"                 },
    {DV_ELEM_TEST43,                 "DV_ELEM_TEST43"                 },
    {DV_ELEM_TEST44,                 "DV_ELEM_TEST44"                 },
    {DV_ELEM_TEST45,                 "DV_ELEM_TEST45"                 },
    {DV_ELEM_TEST46,                 "DV_ELEM_TEST46"                 },
    {DV_ELEM_RCS_CONTROLLER_MODE,    "DV_ELEM_RCS_CONTROLLER_MODE"    },
    {DV_ELEM_LED_CONTROLLER_MODE,    "DV_ELEM_LED_CONTROLLER_MODE"    },
    {DV_ELEM_LED_CONTROL_VAL,        "DV_ELEM_LED_CONTROL_VAL"        },
    {DV_ELEM_LED_FEEDBACK_VAL,       "DV_ELEM_LED_FEEDBACK_VAL"       },
    {DV_ELEM_RECIGNTEST_CONTROL_VAL, "DV_ELEM_RECIGNTEST_CONTROL_VAL" },
    {DV_ELEM_RECIGNTEST_FEEDBACK_VAL,"DV_ELEM_RECIGNTEST_FEEDBACK_VAL"},
};

/**************************** PUBLIC FUNCTIONS ********************************/

DataVectorLogger::~DataVectorLogger ()
{
    mOutputStream.close ();
}

Error_t DataVectorLogger::createNew (
                             Mode_t kMode, std::shared_ptr<DataVector>& kPDv,
                             std::string kFileName,
                             std::shared_ptr<DataVectorLogger>& kPLoggerRet)
{
    Error_t ret = E_SUCCESS;

    // Verify Data Vector ptr is not null.
    if (kPDv == nullptr)
    {
        return E_DATA_VECTOR_NULL;
    }

    // Verify valid mode.
    if (kMode >= Mode_t::LAST)
    {
        return E_INVALID_ENUM;
    }

    // Create Logger.
    kPLoggerRet.reset (new DataVectorLogger (kMode, kPDv, kFileName, ret));

    // Check for error on construct and free memory if it failed.
    if (ret != E_SUCCESS)
    {
        kPLoggerRet.reset ();
        return ret;
    }

    return E_SUCCESS;
}

Error_t DataVectorLogger::log ()
{
    // 1) Copy the Data Vector to the copy buffer.
    Error_t ret = mPDataVector->readDataVector (mCopyBuffer);
    if (ret != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // 2) Write the copy to the Snapshot Data Vector.
    ret = mPDataVectorSnapshot->writeDataVector (mCopyBuffer);
    if (ret != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    // 3) Log based on mode.
    switch (mMode)
    {
        case CSV:
            ret = this->writeCsvRow ();
            break;
        case WATCH:
            ret = this->writeWatch ();
            break;
        default:
            return E_INVALID_ENUM;
    }

    return ret;
}

/*************************** PRIVATE FUNCTIONS ********************************/

DataVectorLogger::DataVectorLogger (Mode_t kMode, 
                                    std::shared_ptr<DataVector>& kPDv,
                                    std::string kFileName, Error_t& kRet) :
    mMode (kMode),
    mPDataVector (kPDv)
{
    // 1) Create a new file or clear existing file.
    mOutputStream.open (kFileName, std::ofstream::out | std::ofstream::trunc);

    // 2) Check for failure to open file.
    if (mOutputStream.good () == false)
    {
        kRet = E_FAILED_TO_OPEN_FILE;
        return;
    }

    // 3) If the mode is CSV, write the CSV header.
    if (kMode == Mode_t::CSV)
    {
        kRet = this->writeCsvHeader ();
        if (kRet != E_SUCCESS)
        {
            return;
        }
    }

    // 4) Initialize the copy buffer.
    uint32_t dbSizeBytes = 0;
    kRet = kPDv->getDataVectorSizeBytes (dbSizeBytes);
    if (kRet != E_SUCCESS)
    {
        kRet = E_DATA_VECTOR_READ;
        return;
    }
    mCopyBuffer.resize (dbSizeBytes);

    // 5) Make a copy of the Data Vector for use during log () method.
    mPDataVectorSnapshot.reset (new DataVector (*mPDataVector));

    kRet = E_SUCCESS;
}

Error_t DataVectorLogger::writeCsvHeader ()
{
    // 1) Loop over regions.
    std::string header;
    const DataVector::Config_t& config = mPDataVector->mConfig;
    for (DataVector::RegionConfig_t regionConfig : config)
    {
        // 1a) Add region name.
        DataVectorRegion_t region = regionConfig.region;
        header += mRegionToStr.find (region) == mRegionToStr.end ()
                      ? std::to_string (region) + ","
                      : mRegionToStr.at (region) + ",";

        // 1b) Loop over elements.
        for (DataVector::ElementConfig_t elemConfig : regionConfig.elems)
        {
            // 1b i) Add element name. 
            DataVectorElement_t elem = elemConfig.elem;
            header += mElemToStr.find (elem) == mElemToStr.end ()
                          ? std::to_string (elem) + ","
                          : mElemToStr.at (elem) + ",";
        }
    }

    // 2) Write header. Endl adds new line and flushes to disk.
    mOutputStream << header << std::endl;

    // 3) Verify output stream state is still good.
    if (mOutputStream.good () == false)
    {
        return E_FAILED_TO_WRITE_FILE;
    }

    return E_SUCCESS;
}

Error_t DataVectorLogger::writeCsvRow ()
{
    // Loop over regions.
    std::string row;
    const DataVector::Config_t& config = mPDataVector->mConfig;
    for (DataVector::RegionConfig_t regionConfig : config)
    {
        // Add empty cell for region.
        row += ",";

        // Loop over elements.
        for (DataVector::ElementConfig_t elemConfig : regionConfig.elems)
        {
            // Add element value. 
            DataVectorElement_t elem = elemConfig.elem;
            std::string value;
            Error_t ret = this->getElementValueStr (elem, value);
            if (ret != E_SUCCESS)
            {
                return ret;
            }

            row += value + ",";
        }
    }

    // Write row. Endl adds new line and flushes to disk.
    mOutputStream << row << std::endl;

    // Verify output stream state is still good.
    if (mOutputStream.good () == false)
    {
        return E_FAILED_TO_WRITE_FILE;
    }

    return E_SUCCESS;
}

Error_t DataVectorLogger::writeWatch ()
{
    Error_t ret = E_SUCCESS;
    std::string formattedStr;

    // 1) Add header.
    formattedStr = "\n\n---------------------------------------------\n";
    formattedStr += "---------------- Data Vector ----------------\n";
    formattedStr += "---------------------------------------------\n\n";

    // 2) Loop over regions.
    const DataVector::Config_t& config = mPDataVector->mConfig;
    for (DataVector::RegionConfig_t regionConfig : config)
    {
        // 2a) Add region name.
        formattedStr += "\n\nRegion: ";
        DataVectorRegion_t region = regionConfig.region;
        formattedStr += mRegionToStr.find (region) == mRegionToStr.end ()
                       ? std::to_string (region)
                       : mRegionToStr.at (region);
        formattedStr += "\n---------------------------------------------\n";

        // 2b) Loop over elements
        for (DataVector::ElementConfig_t elemConfig : regionConfig.elems)
        {
            // 2b i) Add element name. 
            DataVectorElement_t elem = elemConfig.elem;
            std::string elementStr = 
                mElemToStr.find (elem) == mElemToStr.end ()
                    ? std::to_string (elem) + ":"
                    : mElemToStr.at (elem) + ":";
            formattedStr += elementStr;

            // 2b ii) Add spaces to align element value.
            if (WATCH_ELEM_VALUE_START_POS > elementStr.size ())
            {
                formattedStr += std::string (
                               WATCH_ELEM_VALUE_START_POS - elementStr.size (), 
                               ' ');
            }

            // 2b iii) Add element value.
            std::string elemValStr;
            ret = this->getElementValueStr (elem, elemValStr);
            if (ret != E_SUCCESS)
            {
                return ret;
            }
            formattedStr += elemValStr + "\n";
        }
    }

    // 3) Seek to beginning of file to overwrite.
    mOutputStream.seekp (0);
    if (mOutputStream.good () == false)
    {
        return E_FAILED_TO_SEEK;
    }

    // 4) Write to file. Endl adds new line and flushes to disk.
    mOutputStream << formattedStr << std::endl;
    if (mOutputStream.good () == false)
    {
        return E_FAILED_TO_WRITE_FILE;
    }

    return E_SUCCESS;
}


Error_t DataVectorLogger::getElementValueStr (DataVectorElement_t kElem,
                                              std::string& kValueStrRet)
{
    // Get element's type.
    DataVectorElementType_t type;
    Error_t ret = mPDataVector->getElementType (kElem, type);
    if (ret != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Read value based on type and convert to string.
    uint8_t  valUInt8  = 0;
    uint16_t valUInt16 = 0;
    uint32_t valUInt32 = 0;
    uint64_t valUInt64 = 0;
    int8_t   valInt8   = 0;
    int16_t  valInt16  = 0;
    int32_t  valInt32  = 0;
    int64_t  valInt64  = 0;
    float    valFloat  = 0;
    double   valDouble = 0;
    bool     valBool   = false;

    switch (type)
    {
        case DV_T_UINT8:
            ret = mPDataVectorSnapshot->read (kElem, valUInt8);
            kValueStrRet = std::to_string (valUInt8);
            break;
        case DV_T_UINT16:
            ret = mPDataVectorSnapshot->read (kElem, valUInt16);
            kValueStrRet = std::to_string (valUInt16);
            break;
        case DV_T_UINT32:
            ret = mPDataVectorSnapshot->read (kElem, valUInt32);
            kValueStrRet = std::to_string (valUInt32);
            break;
        case DV_T_UINT64:
            ret = mPDataVectorSnapshot->read (kElem, valUInt64);
            kValueStrRet = std::to_string (valUInt64);
            break;
        case DV_T_INT8:
            ret = mPDataVectorSnapshot->read (kElem, valInt8);
            kValueStrRet = std::to_string (valInt8);
            break;
        case DV_T_INT16:
            ret = mPDataVectorSnapshot->read (kElem, valInt16);
            kValueStrRet = std::to_string (valInt16);
            break;
        case DV_T_INT32:
            ret = mPDataVectorSnapshot->read (kElem, valInt32);
            kValueStrRet = std::to_string (valInt32);
            break;
        case DV_T_INT64:
            ret = mPDataVectorSnapshot->read (kElem, valInt64);
            kValueStrRet = std::to_string (valInt64);
            break;
        case DV_T_FLOAT:
            ret = mPDataVectorSnapshot->read (kElem, valFloat);
            kValueStrRet = std::to_string (valFloat);
            break;
        case DV_T_DOUBLE:
            ret = mPDataVectorSnapshot->read (kElem, valDouble);
            kValueStrRet = std::to_string (valDouble);
            break;
        case DV_T_BOOL:
            ret = mPDataVectorSnapshot->read (kElem, valBool);
            kValueStrRet = std::to_string (valBool);
            break;
        default:
            return E_INVALID_TYPE;
    }

    if (ret != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    return E_SUCCESS;
}

