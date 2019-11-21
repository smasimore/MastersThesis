/**
 * The State Vector stores a vector of elements and their 
 * corresponding values. This collection of values represents the current 
 * state of the system from the perspective of the compute node the State
 * Vector is running on. 
 *
 * The State Vector functions as the shared memory abstraction for the avionics 
 * software system. It facilitates sharing memory between software modules 
 * (e.g. between the State Machine, Controllers, and Drivers), between threads
 * (e.g. the main RIO thread and the RIO comms thread), and between compute
 * nodes (e.g. tx'ing a region from a RIO to the FC using the Network
 * Interface). 
 *
 * If using the State Vector in a multi-threaded environment, use the 
 * readWithLock and writeWithLock methods. Otherwise, use the read and write
 * methods. The lock uses the following semantics:
 *         
 *     acquireLock: Thread blocks if lock not available and is added to the
 *                  lock's wait list in priority and then FIFO order.
 *     releaseLock: If there is a thread waiting on the lock that has a higher 
 *                  priority than the thread releasing the lock, a context
 *                  switch to the higher priority thread will occur.
 *
 *
 *                   ------ Using the State Vector --------
 *
 *     1) Define a StateVector::StateVectorConfig_t (see StateVectorTest.cpp 
 *        for examples). 
 *
 *        WARNING: The initial values passed into the SV_ADD_<type> macros are 
 *                 not validated against <type>. Be careful to avoid mistakes
 *                 such as:
 *
 *                 a) Setting initialVal = 2    for a bool element.
 *                 b) Setting initialVal = 1.23 for an element that is not a
 *                    float or double.
 *                 c) Setting initialVal = 2^33 for an element that only fits
 *                    <= 32 bits.
 *                 d) Setting initialVal = -2   for an unsigned element (e.g.
 *                    SV_T_UINT32).
 *
 *     2) Call StateVector::createNew (yourConfig).
 *     3) Use the read and write (or readWithLock and writeWithLock) methods 
 *        to interact with elements in the State Vector. Note, elements cannot 
 *        be added to the State Vector after it has been constructed.
 *
 *    
 * Assumptions: 
 *   #1  Little endian architecture.
 *   #2  Only 1 State Vector is created per compute node. This object is not a 
 *       singleton in order to facilitate testing.
 *
 */

# ifndef STATE_VECTOR_HPP
# define STATE_VECTOR_HPP

#include <stdint.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <pthread.h>

#include "Errors.h"
#include "StateVectorEnums.hpp"

/*********************** HELPER MACROS FOR SV CONFIG *************************/

/**
 * Defines an ElementConfig_t of type SV_T_UINT8.
 */
#define SV_ADD_UINT8(elem, initialVal)                                        \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_UINT8,                                                          \
         StateVector::toUInt64<uint8_t> (initialVal)                          \
    })

/**
 * Defines an ElementConfig_t of type SV_T_UINT16.
 */
#define SV_ADD_UINT16(elem, initialVal)                                       \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_UINT16,                                                         \
         StateVector::toUInt64<uint16_t> (initialVal)                         \
    })

/**
 * Defines an ElementConfig_t of type SV_T_UINT32.
 */
#define SV_ADD_UINT32(elem, initialVal)                                       \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_UINT32,                                                         \
         StateVector::toUInt64<uint32_t> (initialVal)                         \
    })

/**
 * Defines an ElementConfig_t of type SV_T_UINT64.
 */
#define SV_ADD_UINT64(elem, initialVal)                                       \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_UINT64,                                                         \
         StateVector::toUInt64<uint64_t> (initialVal)                         \
    })

/**
 * Defines an ElementConfig_t of type SV_T_INT8.
 */
#define SV_ADD_INT8(elem, initialVal)                                         \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_INT8,                                                           \
         StateVector::toUInt64<int8_t> (initialVal)                           \
    })

/**
 * Defines an ElementConfig_t of type SV_T_INT16.
 */
#define SV_ADD_INT16(elem, initialVal)                                        \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_INT16,                                                          \
         StateVector::toUInt64<int16_t> (initialVal)                          \
    })

/**
 * Defines an ElementConfig_t of type SV_T_INT32.
 */
#define SV_ADD_INT32(elem, initialVal)                                        \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_INT32,                                                          \
         StateVector::toUInt64<int32_t> (initialVal)                          \
    })

/**
 * Defines an ElementConfig_t of type SV_T_INT64.
 */
#define SV_ADD_INT64(elem, initialVal)                                        \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_INT64,                                                          \
         StateVector::toUInt64<int64_t> (initialVal)                          \
    })

/**
 * Defines an ElementConfig_t of type SV_T_FLOAT.
 */
#define SV_ADD_FLOAT(elem, initialVal)                                        \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_FLOAT,                                                          \
         StateVector::toUInt64<float> (initialVal)                            \
    })

/**
 * Defines an ElementConfig_t of type SV_T_DOUBLE.
 */
#define SV_ADD_DOUBLE(elem, initialVal)                                       \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_DOUBLE,                                                         \
         StateVector::toUInt64<double> (initialVal)                           \
    })

/**
 * Defines an ElementConfig_t of type SV_T_BOOL.
 */
#define SV_ADD_BOOL(elem, initialVal)                                         \
    ((StateVector::ElementConfig_t) {                                         \
         elem,                                                                \
         SV_T_BOOL,                                                           \
         StateVector::toUInt64<bool> (initialVal)                             \
    })

/**************************** STATE VECTOR CLASS *****************************/

class StateVector final 
{
public:

    /**
     * Config for a single element in the State Vector.
     */
    typedef struct ElementConfig
    {
        StateVectorElement_t     elem;
        StateVectorElementType_t type;
        uint64_t                 initialVal;
    } ElementConfig_t;

    /**
     * Config for a group of elements called a region. Elements should be
     * grouped s.t. all elements that would be transmitted or received in one
     * message to/from another node are in the same region. This makes tx/rx'ing
     * State Vector data more efficient, since it allows us to pass a pointer to 
     * the region and length of data to the Network Interface.
     */
    typedef struct RegionConfig
    {
        StateVectorRegion_t          region;
        std::vector<ElementConfig_t> elems;
    } RegionConfig_t;

    /**
     * Config for a group of regions used by a compute node. Used by the
     * constructor to initialize the State Vector.
     */
    typedef std::vector<RegionConfig_t> StateVectorConfig_t;

    /** 
     * Struct containing the State Vector's start pointer and size in bytes.
     */
    typedef struct StateVectorInfo
    {   
        uint8_t* pStart;
        uint32_t sizeBytes;
    } StateVectorInfo_t;

    /** 
     * Struct containing a region's start pointer and size in bytes.
     */
    typedef struct RegionInfo
    {   
        uint8_t* pStart;
        uint32_t sizeBytes;
    } RegionInfo_t;

    /**
     * Function to cast various types to a uint64_t bitwise. This is used in 
     * defining the initial values in the State Vector config. In order to 
     * support various types of initial values, these values need to be casted
     * bitwise to the largest possible element (uint64_t). It is declared as
     * an inline function to reduce function call overhead since it is called
     * for every element in the State Vector on initialization.
     *
     * Note: This function returns the uint64_t value instead of an Error_t so 
     *       that it can be used directly in the config, which significantly
     *       simplifies the State Vector config definitions. Additionally,
     *       reinterpret_cast has no failure mode and this function will never
     *       dereference a null pointer as defined.
     */
    template<class T>
    static uint64_t toUInt64 (T val)
    {
        return *(reinterpret_cast<uint64_t*> ((T *) &val));
    }

    /**
     * Entry point for creating a new State Vector. Validates the
     * passed in config. This should only be called once per compute node, although
     * this is not enforced to facilitate testing.
     *
     * @param   kConfig             State Vector's config data.
     * @param   kPStateVectorRet    Pointer to return State Vector.
     *
     * @ret     E_SUCCESS           State Vector successfully created.
     *          E_EMPTY_CONFIG      Config empty.
    */
    static Error_t createNew (StateVectorConfig_t& kConfig,
                              std::shared_ptr<StateVector>& kPStateVectorRet);

    /**
     * Given a State Vector element type, stores the size of that type (bytes)
     * in the sizeBytesRet parameter.
     *
     * @param   kType               Type to get size of.
     * @param   kSizeBytesRet       Param to store element's size in.
     *
     * @ret     E_SUCCESS           Size stored in sizeBytesRet successfully.
     *          E_INVALID_ENUM      Type not supported.
     */
    static Error_t getSizeBytesFromType (StateVectorElementType_t kType,
                                         uint8_t& kSizeBytesRet);

    /**
     * Returns a copy of the State Vector's info struct.
     *
     * @param    kStateVectorInfoRet   Struct to store State Vector info in.
     *
     * @ret      E_SUCCESS             Info returned successfully.
     */
    Error_t getStateVectorInfo (StateVectorInfo_t& kStateVectorInfoRet);

    /**
     * Returns a copy of a region's info struct.
     *
     * @param    kRegion           Region to get info for.
     * @param    kRegionInfoRet    Struct to store region info in.    
     *
     * @ret      E_SUCCESS         Region info returned successfully.
     *           E_INVALID_REGION  Region enum invalid or not in State Vector.
     */
    Error_t getRegionInfo (StateVectorRegion_t kRegion,
                           RegionInfo_t& kRegionInfoRet);

    /**
     * Read an element from the State Vector.  Defined in the header so that the
     * templatized functions do not need to each be instantiated explicitly.
     *
     * @param   kElem             Element to read.
     * @param   kValueRet         Variable to store element's value.
     *
     * @ret     E_SUCCESS         Element read successfully.
     *          E_INVALID_ELEM    Element not in State Vector.
     *          E_INVALID_TYPE    Elem_t not supported by State Vector.
     *          E_INCORRECT_TYPE  Elem_t does not match expected element type.
     */
    template<class Elem_T>
    Error_t read (StateVectorElement_t kElem, Elem_T& kValueRet)
    {
        Error_t ret = this->verifyElement (kElem, kValueRet); 
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Store element's value in kValueRet.
        ElementInfo_t* pElementInfo = &mElementToElementInfo[kElem];
        std::memcpy (&kValueRet, pElementInfo->pStart, sizeof (kValueRet));

        return E_SUCCESS;
    }

    /**
     * Write a value to the State Vector.  Defined in the header so that the
     * templatized functions do not need to each be instantiated explicitly.
     *
     * @param   kElem             Element to write to.
     * @param   kValue            Value to write.
     *
     * @ret     E_SUCCESS         Element written to successfully.
     *          E_INVALID_ELEM    Element not in State Vector.
     *          E_INVALID_TYPE    Elem_t not supported by State Vector.
     *          E_INCORRECT_TYPE  Elem_t does not match expected element type.
    */
    template<class Elem_T>
    Error_t write (StateVectorElement_t kElem, Elem_T kValue)
    {
        Error_t ret = this->verifyElement (kElem, kValue); 
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Store value in mBuffer.
        ElementInfo_t* pElementInfo = &mElementToElementInfo[kElem];
        std::memcpy (pElementInfo->pStart, &kValue, sizeof (kValue));

        return E_SUCCESS;
    }

    /**
     * Acquire the State Vector lock, read an element from the State Vector, 
     * and release the lock. Defined in the header so that the templatized 
     * functions do not need to each be instantiated explicitly.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @param   kElem                         Element to read.
     * @param   kValueRet                     Variable to store element's value.
     *
     * @ret     E_SUCCESS                     Element read successfully.
     *          E_FAILED_TO_LOCK              Failed to lock.
     *          E_INVALID_ELEM                Element not in State Vector.
     *          E_INVALID_TYPE                Elem_t does not match expected 
     *                                        element type or type not 
     *                                        supported.
     *          E_FAILED_TO_READ_AND_UNLOCK   Error on read and failed to 
     *                                        unlock.
     *          E_FAILED_TO_UNLOCK            Read succeeded but failed to 
     *                                        unlock.
     *
     */
    template<class Elem_T>
    Error_t readWithLock (StateVectorElement_t kElem, Elem_T& kValueRet)
    {
        Error_t ret = E_SUCCESS;
        
        // Acquire lock.
        ret = this->acquireLock ();
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Attempt to read the element.
        ret = this->read (kElem, kValueRet);
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
        ret = this->releaseLock ();

        return ret;
    }

    /**
     * Acquire the State Vector lock, write a value to the State Vector, and
     * release the lock. Defined in the header so that the templatized functions 
     * do not need to each be instantiated explicitly.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @param   kElem                         Element to write to.
     * @param   kValue                        Value to write.
     *
     * @ret     E_SUCCESS                     Element written to successfully.
     *          E_FAILED_TO_LOCK              Failed to lock.
     *          E_INVALID_ELEM                Element not in State Vector.
     *          E_INVALID_TYPE                Elem_t does not match expected 
     *                                        element type or type not 
     *                                        supported.
     *          E_FAILED_TO_WRITE_AND_UNLOCK  Error on write and failed to 
     *                                        unlock.
     *          E_FAILED_TO_UNLOCK            Write succeeded but failed to 
     *                                        unlock.
     */
    template<class Elem_T>
    Error_t writeWithLock (StateVectorElement_t kElem, Elem_T kValue)
    {
        Error_t ret = E_SUCCESS;
        
        // Acquire lock.
        ret = this->acquireLock ();
        if (ret != E_SUCCESS)
        {
            return ret;
        }

        // Attempt to write element.
        ret = this->write (kElem, kValue);
        if (ret != E_SUCCESS)
        {
            // If write fails, attempt to release lock.
            Error_t unlockRet = this->releaseLock ();

            // If release fails, return updated error.
            if (unlockRet != E_SUCCESS)
            {
                return E_FAILED_TO_WRITE_AND_UNLOCK;
            } 

            // Otherwise, return error from read.
            else 
            {
                return ret;
            }
        }

        // Release lock.
        ret = this->releaseLock ();

        return ret;
    }

    /**
     * Acquire the State Vector lock. This method is public so that the lock
     * can be held while tx/rx'ing a region of or the entire State Vector using
     * the Network Interface. 
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @ret     E_SUCCESS        Element written to successfully.
     *          E_FAILED_TO_LOCK Failed to lock.
    */
    Error_t acquireLock ();

    /**
     * Release the State Vector lock. This method is public so that the lock
     * can be held while tx/rx'ing a region of or the entire State Vector using
     * the Network Interface.
     *
     * NOTE: Calling this method can result in the current thread blocking.
     *
     * @ret     E_SUCCESS           Element written to successfully.
     *          E_FAILED_TO_UNLOCK  Failed to unlock.           
    */
    Error_t releaseLock ();

private:

    /** 
     * Struct containing an element's start pointer and type.
     */
    typedef struct ElementInfo
    {   
        uint8_t* pStart;
        StateVectorElementType_t type;
    } ElementInfo_t;

    /**
     * In order to use any enum classes we've defined as the key for an 
     * unordered_map, we need to define a hash function that maps the key
     * type to size_t. Hash function copied from:
     * https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
     */
    struct EnumClassHash
    {
        template <typename T>
        std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    /**
     * Buffer containing State Vector element data.
     */
    std::vector<uint8_t> mBuffer;

    /**
     * Struct containing State Vector info (starting address and size in 
     * bytes).
     */
    StateVectorInfo_t mStateVectorInfo;

    /**
     * Map from region to region's info, which contains the region's starting
     * address and size in bytes.
     */
    std::unordered_map<StateVectorRegion_t, 
                       RegionInfo_t, 
                       EnumClassHash> mRegionToRegionInfo;

    /**
     * Map from element to element's info, which contains the element's starting
     * address and type.
     */
    std::unordered_map<StateVectorElement_t, 
                       ElementInfo_t, 
                       EnumClassHash> mElementToElementInfo;

    /**
     * Lock for synchronizing access to the State Vector in a multi-threaded 
     * environment.
     */
    pthread_mutex_t mLock;

    /**
     * Constructor. Given a config, builds mBuffer, mStateVectorInfo, and
     * mRegionToRegionInfo. Because StateVector::getSizeBytesFromType is called
     * while building the State Vector's underlying data structures, the 
     * constructor can fail. If this happens, return an error code in the ret
     * parameter.
     *
     * @param    kConfig    State Vector config.
     * @param    kRet       E_SUCCESS                Successfully created State
     *                                               Vector.
     *                      E_INVALID_ENUM           Element type in config not 
     *                                               supported by 
     *                                               getSizeBytesFromType.
     *                      E_FAILED_TO_INIT_LOCK    Failed to initialize lock.
     */        
    StateVector (StateVectorConfig_t& kConfig, Error_t& kRet);

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
    static Error_t verifyConfig (StateVectorConfig_t& kConfig);

    /**
     * Verify element is in State Vector and kValue's type matches element's
     * type. Defined in the header so that the templatized functions do not 
     * need to each be instantiated explicitly.
     *
     * @param   elem              Config to check.
     *
     * @ret     E_SUCCESS         Element and kValue type valid.
     *          E_INVALID_ELEM    Element not in State Vector.
     *          E_INVALID_TYPE    Elem_t not supported by State Vector.
     *          E_INCORRECT_TYPE  Elem_t does not match expected element type.
     */
    template<class Elem_T>
    Error_t verifyElement (StateVectorElement_t kElem, Elem_T& kValue)
    {
        // Check if element in State Vector.
        if (mElementToElementInfo.find (kElem) == mElementToElementInfo.end ()) 
        {   
            return E_INVALID_ELEM;
        }   

        ElementInfo_t* pElementInfo = &mElementToElementInfo[kElem];

        // Check if element's type matches type expected by caller.
        bool typeMatches = false;
        switch (pElementInfo->type)
        {   
            case SV_T_UINT8:
                typeMatches = typeid (kValue) == typeid (uint8_t);
                break;
            case SV_T_UINT16:
                typeMatches = typeid (kValue) == typeid (uint16_t);
                break;
            case SV_T_UINT32:
                typeMatches = typeid (kValue) == typeid (uint32_t);
                break;
            case SV_T_UINT64:
                typeMatches = typeid (kValue) == typeid (uint64_t);
                break;
            case SV_T_INT8:
                typeMatches = typeid (kValue) == typeid (int8_t);
                break;
            case SV_T_INT16:
                typeMatches = typeid (kValue) == typeid (int16_t);
                break;
            case SV_T_INT32:
                typeMatches = typeid (kValue) == typeid (int32_t);
                break;
            case SV_T_INT64:
                typeMatches = typeid (kValue) == typeid (int64_t);
                break;
            case SV_T_FLOAT:
                typeMatches = typeid (kValue) == typeid (float);
                break;
            case SV_T_DOUBLE:
                typeMatches = typeid (kValue) == typeid (double);
                break;
            case SV_T_BOOL:
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
