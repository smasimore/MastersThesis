/**
 * The Data Vector stores a vector of elements and their 
 * corresponding values. This collection of values represents the current 
 * state of the system from the perspective of the compute node the Data
 * Vector is running on. 
 *
 * The Data Vector functions as the shared memory abstraction for the avionics 
 * software system. It facilitates sharing memory between software modules 
 * (e.g. between the State Machine, Controllers, and Drivers), between threads
 * (e.g. the main RIO thread and the RIO comms thread), and between compute
 * nodes (e.g. tx'ing a region from a RIO to the FC using the Network
 * Interface). 
 *
 * A Data Vector is split into Regions. Regions encapsulate a group of related
 * data within the Data Vector that is either sent to or received from another 
 * flight computer.
 *
 * A lock is used for thread synchronization to ensure only 1 thread is 
 * accessing the Data Vector at once. The underlying lock semantics are as 
 * follows:
 *         
 *     acquireLock: Thread blocks if lock not available and is added to the
 *                  lock's wait list in priority and then FIFO order.
 *     releaseLock: If there is a thread waiting on the lock that has a higher 
 *                  priority than the thread releasing the lock, a context
 *                  switch to the higher priority thread will occur.
 *
 *
 *                   ------ Using the Data Vector --------
 *
 *     1) Define a DataVector::Config_t (see DataVectorTest.cpp for examples). 
 *
 *        WARNING: The initial values passed into the DV_ADD_<type> macros are 
 *                 not validated against <type>. Be careful to avoid mistakes
 *                 such as:
 *
 *                 a) Setting initialVal = 2    for a bool element.
 *                 b) Setting initialVal = 1.23 for an element that is not a
 *                    float or double.
 *                 c) Setting initialVal = 2^33 for an element that only fits
 *                    <= 32 bits.
 *                 d) Setting initialVal = -2   for an unsigned element (e.g.
 *                    DV_T_UINT32).
 *
 *     2) Call DataVector::createNew (yourConfig, pDv).
 *     3) Use the read and write methods to interact with elements in the Data
 *        Vector. Note, elements cannot be added to the Data Vector after it 
 *        has been constructed.
 *
 *    
 * Assumptions: 
 *   #1  Little endian architecture.
 *   #2  Only 1 Data Vector is created per compute node. This object is not a 
 *       singleton in order to facilitate testing.
 *
 * Notes:
 *   #1  Due to networking constraints, the maximum Region size is capped at 
 *       1024 bytes. This is the maximum size of a message that can be received
 *       by a flight computer. There is currently no maximum on overall Data
 *       Vector size.
 *
 */

#ifndef DATA_VECTOR_HPP
#define DATA_VECTOR_HPP

#include <stdint.h>
#include <pthread.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "Errors.hpp"
#include "DataVectorEnums.hpp"
#include "EnumClassHash.hpp"

/*********************** HELPER MACROS FOR DV CONFIG **************************/
/***************** See DataVectorTest.cpp for example usage *******************/


/**
 * Defines an ElementConfig_t of type DV_T_UINT8.
 */
#define DV_ADD_UINT8(elem, initialVal)                                         \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_UINT8,                                                           \
         DataVector::toUInt64<uint8_t> (initialVal)                            \
    })

/**
 * Defines an ElementConfig_t of type DV_T_UINT16.
 */
#define DV_ADD_UINT16(elem, initialVal)                                        \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_UINT16,                                                          \
         DataVector::toUInt64<uint16_t> (initialVal)                           \
    })

/**
 * Defines an ElementConfig_t of type DV_T_UINT32.
 */
#define DV_ADD_UINT32(elem, initialVal)                                        \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_UINT32,                                                          \
         DataVector::toUInt64<uint32_t> (initialVal)                           \
    })

/**
 * Defines an ElementConfig_t of type DV_T_UINT64.
 */
#define DV_ADD_UINT64(elem, initialVal)                                        \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_UINT64,                                                          \
         DataVector::toUInt64<uint64_t> (initialVal)                           \
    })

/**
 * Defines an ElementConfig_t of type DV_T_INT8.
 */
#define DV_ADD_INT8(elem, initialVal)                                          \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_INT8,                                                            \
         DataVector::toUInt64<int8_t> (initialVal)                             \
    })

/**
 * Defines an ElementConfig_t of type DV_T_INT16.
 */
#define DV_ADD_INT16(elem, initialVal)                                         \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_INT16,                                                           \
         DataVector::toUInt64<int16_t> (initialVal)                            \
    })

/**
 * Defines an ElementConfig_t of type DV_T_INT32.
 */
#define DV_ADD_INT32(elem, initialVal)                                         \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_INT32,                                                           \
         DataVector::toUInt64<int32_t> (initialVal)                            \
    })

/**
 * Defines an ElementConfig_t of type DV_T_INT64.
 */
#define DV_ADD_INT64(elem, initialVal)                                         \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_INT64,                                                           \
         DataVector::toUInt64<int64_t> (initialVal)                            \
    })

/**
 * Defines an ElementConfig_t of type DV_T_FLOAT.
 */
#define DV_ADD_FLOAT(elem, initialVal)                                         \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_FLOAT,                                                           \
         DataVector::toUInt64<float> (initialVal)                              \
    })

/**
 * Defines an ElementConfig_t of type DV_T_DOUBLE.
 */
#define DV_ADD_DOUBLE(elem, initialVal)                                        \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_DOUBLE,                                                          \
         DataVector::toUInt64<double> (initialVal)                             \
    })

/**
 * Defines an ElementConfig_t of type DV_T_BOOL.
 */
#define DV_ADD_BOOL(elem, initialVal)                                          \
    ((DataVector::ElementConfig_t) {                                           \
         elem,                                                                 \
         DV_T_BOOL,                                                            \
         DataVector::toUInt64<bool> (initialVal)                               \
    })

/**************************** DATA VECTOR CLASS *****************************/

class DataVector final 
{
public:

    /**
     * Config for a single element in the Data Vector.
     */
    typedef struct ElementConfig
    {
        DataVectorElement_t     elem;
        DataVectorElementType_t type;
        uint64_t                initialVal;
    } ElementConfig_t;

    /**
     * Config for a group of elements called a region. Elements should be
     * grouped s.t. all elements that would be transmitted or received in one
     * message to/from another node are in the same region. This makes tx/rx'ing
     * Data Vector data more efficient, since it allows us to pass a pointer to 
     * the region and length of data to the Network Interface.
     */
    typedef struct RegionConfig
    {
        DataVectorRegion_t          region;
        std::vector<ElementConfig_t> elems;
    } RegionConfig_t;

    /**
     * Config for a group of regions used by a compute node. Used by the
     * constructor to initialize the Data Vector.
     */
    typedef std::vector<RegionConfig_t> Config_t;

    /**
     *  Copy of passed in Data Vector config. Used by DataVectorLogger.
     */
    const Config_t mConfig;

    /**
     * Function to cast various types to a uint64_t bitwise. This is used in 
     * defining the initial values in the Data Vector config. In order to 
     * support various types of initial values, these values need to be casted
     * bitwise to the largest possible element (uint64_t). It is declared as
     * an inline function to reduce function call overhead since it is called
     * for every element in the Data Vector on initialization.
     *
     * Note: This function returns the uint64_t value instead of an Error_t so 
     *       that it can be used directly in the config, which significantly
     *       simplifies the Data Vector config definitions. Additionally,
     *       reinterpret_cast has no failure mode and this function will never
     *       dereference a null pointer as defined.
     */
    template<class T>
    static uint64_t toUInt64 (T val)
    {
        return *(reinterpret_cast<uint64_t*> ((T *) &val));
    }

    /**
     * Entry point for creating a new Data Vector. Validates the
     * passed in config. This should only be called once per compute node, 
     * although this is not enforced to facilitate testing.
     *
     * @param   kConfig               Data Vector's config data.
     * @param   kPDataVectorRet       Pointer to return Data Vector.
     *
     * @ret     E_SUCCESS             Data Vector successfully created.
     *          E_EMPTY_CONFIG        Config empty.
     *          E_EMPTY_ELEMS         Region's element list empty.
     *          E_DUPLICATE_REGION    Duplicate region.
     *          E_DUPLICATE_ELEM      Duplicate element.
     *          E_INVALID_ENUM        Invalid enumeration.
     *          E_INVALID_TYPE        Element type in config not supported by 
     *                                getSizeBytesFromType.
     *          E_REGION_TOO_LARGE    Region too large.
     *          E_FAILED_TO_INIT_LOCK Failed to initialize lock.
     */
    static Error_t createNew (Config_t& kConfig, 
                              std::shared_ptr<DataVector>& kPDataVectorRet);

    /**
     * Given a Data Vector element type, stores the size of that type (bytes)
     * in the sizeBytesRet parameter.
     *
     * @param   kType               Type to get size of.
     * @param   kSizeBytesRet       Param to store element's size in.
     *
     * @ret     E_SUCCESS           Size stored in kSizeBytesRet successfully.
     *          E_INVALID_ENUM      Type not supported.
     */
    static Error_t getSizeBytesFromType (DataVectorElementType_t kType,
                                         uint8_t& kSizeBytesRet);

    /**
     * Returns number of bytes in the region's underlying byte buffer.
     *
     * @param   kRegion             Region to get size of.
     * @param   kSizeBytesRet       Param to store region's size in (bytes).
     *
     * @ret     E_SUCCESS           Size stored in kSizeBytesRet successfully.
     *          E_INVALID_REGION    Region enum invalid or not in Data Vector.
     */
    Error_t getRegionSizeBytes (DataVectorRegion_t kRegion, 
                                uint32_t& kSizeBytesRet);

    /**
     * Returns number of bytes in the underlying Data Vector buffer.
     *
     * @param   kSizeBytesRet       Param to store Data Vector's size in (bytes).
     *
     * @ret     E_SUCCESS           Size stored in kSizeBytesRet successfully.
     */
    Error_t getDataVectorSizeBytes (uint32_t& kSizeBytesRet);

    /**
     * Get the element's type.
     *
     * @param  kElem          Element to check.
     * @param  kTypeRet       Parameter to store type of element.
     *
     * @ret    E_SUCCESS      Element in DV.
     *         E_INVALID_ELEM Element not in DV.
     */
    Error_t getElementType (DataVectorElement_t kElem, 
                            DataVectorElementType_t& kTypeRet);

    /**
     * Check if element exists in Data Vector.
     *
     * @param  kElem          Element to check.
     *
     * @ret    E_SUCCESS      Element in DV.
     *         E_INVALID_ELEM Element not in DV.
     */
    Error_t elementExists (DataVectorElement_t kElem);

    /**
     * Read an element from the Data Vector. Defined in the header so that the
     * templatized functions do not need to each be instantiated explicitly.
     * 
     * NOTE: Calling this method can result in the current thread blocking.
     * 
     * @param   kElem                         Element to read.
     * @param   kValueRet                     Variable to store element's value.
     *
     * @ret     E_SUCCESS                     Element read successfully.
     *          E_INVALID_ELEM                Element not in Data Vector.
     *          E_INVALID_TYPE                Elem_t not supported by Data
     *                                        Vector.
     *          E_INCORRECT_TYPE              Elem_t does not match expected 
     *                                        element type.
     *          E_FAILED_TO_LOCK              Failed to lock.
     *          E_FAILED_TO_READ_AND_UNLOCK   Error on read and failed to 
     *                                        unlock.
     *          E_FAILED_TO_UNLOCK            Read succeeded but failed to 
     *                                        unlock.
     */
    template<class Elem_T>
    Error_t read (DataVectorElement_t kElem, Elem_T& kValueRet)
    {
        // Acquire lock.
        Error_t ret = this->acquireLock ();
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Attempt to read the element.
        ret = this->readImpl (kElem, kValueRet);
        if (ret != E_SUCCESS)
        {
            // If read fails, attempt to release lock.
            Error_t unlockRet = this->releaseLock ();

            // If release fails, return updated error.
            if (unlockRet != E_SUCCESS)
            {
                return E_FAILED_TO_READ_AND_UNLOCK;
            } 

            // Otherwise, return error from read.
            else 
            {
                return ret;
            }
        }

        // Release lock. 
        return this->releaseLock ();
    }

    /**
     * Write an element value to the Data Vector.  Defined in the header so 
     * that the templatized functions do not need to each be instantiated 
     * explicitly.
     *
     * Implementation of Write.  Defined in the header so that the templatized 
     * functions do not need to each be instantiated explicitly.
     *
     * @param   kElem                         Element to write to.
     * @param   kValue                        Value to write.
     *
     * @ret     E_SUCCESS                     Element written to successfully.
     *          E_INVALID_ELEM                Element not in Data Vector.
     *          E_INVALID_TYPE                Elem_t not supported by Data 
     *                                        Vector.
     *          E_INCORRECT_TYPE              Elem_t does not match expected 
     *                                        element type.
     *          E_FAILED_TO_LOCK              Failed to lock.
     *          E_FAILED_TO_WRITE_AND_UNLOCK  Error on write and failed to 
     *                                        unlock.
     *          E_FAILED_TO_UNLOCK            Write succeeded but failed to 
     *                                        unlock.
    */
    template<class Elem_T>
    Error_t write (DataVectorElement_t kElem, Elem_T kValue)
    {
        // Acquire lock.
        Error_t ret = this->acquireLock ();
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Attempt to write element.
        ret = this->writeImpl (kElem, kValue);
        if (ret != E_SUCCESS)
        {
            // If write fails, attempt to release lock.
            Error_t unlockRet = this->releaseLock ();

            // If release fails, return updated error.
            if (unlockRet != E_SUCCESS)
            {
                return E_FAILED_TO_WRITE_AND_UNLOCK;
            } 

            // Otherwise, return error from write.
            else 
            {
                return ret;
            }
        }

        // Release lock.
        return this->releaseLock ();
    }

    /**
     * Increment an element's value by 1. Float, double, and bool cannot be
     * incremented. If element's value is already max value, element will not
     * be incremented and E_ALREADY_MAX will be returned.
     *
     * @param   kElem                         Element to increment.
     *
     * @ret     E_SUCCESS                     Element incremented successfully.
     *          E_ALREADY_MAX                 Element already at max value.
     *          E_INVALID_ELEM                Element not in Data Vector.
     *          E_INVALID_TYPE                Element type cannot be 
     *                                        incremented or not supported by 
     *                                        Data Vector.
     *          E_FAILED_TO_LOCK              Failed to lock.
     *          E_FAILED_TO_UNLOCK            Failed to unlock.
     *          E_FAILED_TO_WRITE_AND_UNLOCK  Error on write and failed to 
     *                                        unlock.
     *          E_FAILED_TO_READ_AND_UNLOCK   Error on read and failed to 
     *                                        unlock.
    */
    Error_t increment (DataVectorElement_t kElem);

    /**
     * Returns a copy the specified region's underlying byte buffer. The vector
     * passed in to copy the underlying buffer to must already have a size
     * equal to the size of the region for copy efficiency purposes.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @param    kRegion                       Region to get underlying 
     *                                         byte buffer of.
     * @param    kRegionBufRet                 Vector to store region's 
     *                                         underlying byte buffer in.
     *
     * @ret      E_SUCCESS                     Buffer copied successfully.
     *           E_INVALID_REGION              Region enum not in Data Vector.
     *           E_INCORRECT_SIZE              Vector provided does not have
     *                                         same size as region.
     *           E_FAILED_TO_LOCK              Failed to lock.
     *           E_FAILED_TO_UNLOCK            Read succeeded but failed to 
     *                                         unlock.
     */
    Error_t readRegion (DataVectorRegion_t kRegion, 
                        std::vector<uint8_t>& kRegionBufRet);

    /**
     * Write the provided buffer to the specified region's underlying byte 
     * buffer. 
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @param    kRegion                       Region to write byte buffer to.
     * @param    kRegionBuf                    Vector containing byte buffer
     *                                         to write to region.
     *
     * @ret      E_SUCCESS                     Buffer copied successfully.
     *           E_INVALID_REGION              Region enum not in Data Vector.
     *           E_INCORRECT_SIZE              Vector provided does not have
     *                                         same size as region.
     *           E_FAILED_TO_LOCK              Failed to lock.
     *           E_FAILED_TO_UNLOCK            Read succeeded but failed to 
     *                                         unlock.
     */
    Error_t writeRegion (DataVectorRegion_t kRegion, 
                         std::vector<uint8_t>& kRegionBuf);

    /**
     * Returns a copy the Data Vector's underlying byte buffer. The vector
     * passed in to copy the underlying buffer to must already have a size
     * equal to the size of the Data Vector for copy efficiency purposes.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @param    kDataVectorBufRet             Vector to store copy of Data 
     *                                         Vector's underlying buffer in.
     *
     * @ret      E_SUCCESS                     Buffer copied successfully.
     *           E_INCORRECT_SIZE              Vector provided does not have
     *                                         same size as mBuffer.
     *           E_FAILED_TO_LOCK              Failed to lock.
     *           E_FAILED_TO_UNLOCK            Read succeeded but failed to 
     *                                         unlock.
     */
    Error_t readDataVector (std::vector<uint8_t>& kDataVectorBufRet);

    /**
     * Overwrite the Data Vector with the provided buffer.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @param    kDvBuf                        Vector containing byte buffer
     *                                         to write to Data Vector.
     *
     * @ret      E_SUCCESS                     Buffer copied successfully.
     *           E_INCORRECT_SIZE              Vector provided does not have
     *                                         same size as Data Vector.
     *           E_FAILED_TO_LOCK              Failed to lock.
     *           E_FAILED_TO_UNLOCK            Write succeeded but failed to 
     *                                         unlock.
     */
    Error_t writeDataVector (std::vector<uint8_t>& kDvBuf);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF DATA VECTOR
     *
     * Acquire the Data Vector lock. This method is public so that the lock
     * can be held while tx/rx'ing a region of or the entire Data Vector using
     * the Network Interface. 
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @ret     E_SUCCESS        Element written to successfully.
     *          E_FAILED_TO_LOCK Failed to lock.
    */
    Error_t acquireLock ();

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF DATA VECTOR
     *
     * Release the Data Vector lock. This method is public so that the lock
     * can be held while tx/rx'ing a region of or the entire Data Vector using
     * the Network Interface.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @ret     E_SUCCESS           Element written to successfully.
     *          E_FAILED_TO_UNLOCK  Failed to unlock.           
    */
    Error_t releaseLock ();

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF DATA VECTOR
     *
     * Implementation of read. Defined in the header so that the templatized 
     * functions do not need to each be instantiated explicitly.
     *
     * @param   kElem             Element to read.
     * @param   kValueRet         Variable to store element's value.
     *
     * @ret     E_SUCCESS         Element read successfully.
     *          E_INVALID_ELEM    Element not in Data Vector.
     *          E_INVALID_TYPE    Elem_t not supported by Data Vector.
     *          E_INCORRECT_TYPE  Elem_t does not match expected element type.
     */
    template<class Elem_T>
    Error_t readImpl (DataVectorElement_t kElem, Elem_T& kValueRet)
    {
        Error_t ret = this->verifyElement (kElem, kValueRet); 
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Store element's value in kValueRet.
        ElementInfo_t* pElementInfo = &mElementToElementInfo[kElem];
        std::memcpy (&kValueRet, &mBuffer[pElementInfo->startIdx], 
                     sizeof (kValueRet));

        return E_SUCCESS;
    }

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF DATA VECTOR
     *
     * Implementation of Write.  Defined in the header so that the templatized 
     * functions do not need to each be instantiated explicitly.
     *
     * @param   kElem             Element to write to.
     * @param   kValue            Value to write.
     *
     * @ret     E_SUCCESS         Element written to successfully.
     *          E_INVALID_ELEM    Element not in Data Vector.
     *          E_INVALID_TYPE    Elem_t not supported by Data Vector.
     *          E_INCORRECT_TYPE  Elem_t does not match expected element type.
    */
    template<class Elem_T>
    Error_t writeImpl (DataVectorElement_t kElem, Elem_T kValue)
    {
        Error_t ret = this->verifyElement (kElem, kValue); 
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Store value in mBuffer.
        ElementInfo_t* pElementInfo = &mElementToElementInfo[kElem];
        std::memcpy (&mBuffer[pElementInfo->startIdx], &kValue, 
                     sizeof (kValue));

        return E_SUCCESS;
    }

private:

    /** 
     * Struct containing an element's start index in mBuffer and type.
     */
    typedef struct ElementInfo
    {   
        uint32_t startIdx;
        DataVectorElementType_t type;
    } ElementInfo_t;

    /** 
     * Struct containing a region's start index into mBuffer and size in bytes.
     */
    typedef struct RegionInfo
    {   
        uint32_t startIdx;
        uint32_t sizeBytes;
    } RegionInfo_t;

    /**
     * Buffer containing Data Vector element data.
     */
    std::vector<uint8_t> mBuffer;

    /**
     * Map from region to region's info, which contains the region's starting
     * address and size in bytes.
     */
    std::unordered_map<DataVectorRegion_t, 
                       RegionInfo_t, 
                       EnumClassHash> mRegionToRegionInfo;

    /**
     * Map from element to element's info, which contains the element's starting
     * address and type.
     */
    std::unordered_map<DataVectorElement_t, 
                       ElementInfo_t, 
                       EnumClassHash> mElementToElementInfo;

    /**
     * Lock for synchronizing access to the Data Vector in a multi-threaded 
     * environment.
     */
    pthread_mutex_t mLock;

    /**
     * Constructor. Given a config, builds mBuffer, mRegionToRegionInfo, and
     * mElementToElementInfo. Because the construuctor can fail, an error code 
     * is returned in the ret parameter.
     *
     * @param    kConfig      Data Vector config.
     * @param    kRet         E_SUCCESS                Successfully created 
     *                                                 Data Vector.
     *                        E_INVALID_ENUM           Element type in config 
     *                                                 not supported by 
     *                                                 getSizeBytesFromType.
     *                        E_FAILED_TO_INIT_LOCK    Failed to initialize 
     *                                                 lock.
     */        
    DataVector (Config_t& kConfig, Error_t& kRet);

    /**
     * Verifies provided config.
     *
     * @param   kConfig             Config to check.
     *
     * @ret     E_SUCCESS           Config valid.
     *          E_EMPTY_CONFIG      Config empty.
     *          E_EMPTY_ELEMS       Region's element list empty.
     *          E_DUPLICATE_REGION  Duplicate region.
     *          E_DUPLICATE_ELEM    Duplicate element.
     *          E_INVALID_ENUM      Invalid enumeration.
     */
    static Error_t verifyConfig (Config_t& kConfig);

    /**
     * Verify element is in Data Vector and kValue's type matches element's
     * type. Defined in the header so that the templatized functions do not 
     * need to each be instantiated explicitly.
     *
     * @param   elem              Config to check.
     *
     * @ret     E_SUCCESS         Element and kValue type valid.
     *          E_INVALID_ELEM    Element not in Data Vector.
     *          E_INVALID_TYPE    Elem_t not supported by Data Vector.
     *          E_INCORRECT_TYPE  Elem_t does not match expected element type.
     */
    template<class Elem_T>
    Error_t verifyElement (DataVectorElement_t kElem, Elem_T& kValue)
    {
        // Check if element in Data Vector.
        Error_t ret = this->elementExists (kElem);
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        ElementInfo_t* pElementInfo = &mElementToElementInfo[kElem];

        // Check if element's type matches type expected by caller.
        bool typeMatches = false;
        switch (pElementInfo->type)
        {   
            case DV_T_UINT8:
                typeMatches = typeid (kValue) == typeid (uint8_t);
                break;
            case DV_T_UINT16:
                typeMatches = typeid (kValue) == typeid (uint16_t);
                break;
            case DV_T_UINT32:
                typeMatches = typeid (kValue) == typeid (uint32_t);
                break;
            case DV_T_UINT64:
                typeMatches = typeid (kValue) == typeid (uint64_t);
                break;
            case DV_T_INT8:
                typeMatches = typeid (kValue) == typeid (int8_t);
                break;
            case DV_T_INT16:
                typeMatches = typeid (kValue) == typeid (int16_t);
                break;
            case DV_T_INT32:
                typeMatches = typeid (kValue) == typeid (int32_t);
                break;
            case DV_T_INT64:
                typeMatches = typeid (kValue) == typeid (int64_t);
                break;
            case DV_T_FLOAT:
                typeMatches = typeid (kValue) == typeid (float);
                break;
            case DV_T_DOUBLE:
                typeMatches = typeid (kValue) == typeid (double);
                break;
            case DV_T_BOOL:
                typeMatches = typeid (kValue) == typeid (bool);
                break;
            default:
                return E_INVALID_TYPE;
        }   

        if (typeMatches == false)
        {   
            return E_INCORRECT_TYPE;
        }   

        return E_SUCCESS;
    }

    /**
     * Initialize the lock as PTHREAD_MUTEX_ERRORCHECK type. This ensures that 
     * if a thread tries to lock a mutex twice, it does not deadlock and instead
     * returns an error.
     *
     * @ret     E_SUCCESS                Lock successfully initialized.
     *          E_FAILED_TO_INIT_LOCK    Lock initialization failed.
     */
    Error_t initLock ();

};
#endif
