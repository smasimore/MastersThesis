/**
 * The purpose of the State Vector is to store a vector of elements and their 
 * corresponding values, representing the current state of the system from the
 * perspective of the compute node it is running on. 
 *
 *                   ------ Using the State Vector --------
 *
 *         1) Define a StateVector::StateVectorConfig_t (see StateVectorTest.cpp 
 *            for examples). 
 *
 *            WARNING: The initial values passed into the SV_ADD_<type> macros are 
 *                     not validated against <type>. Be careful to avoid mistakes
 *                     such as:
 *
 *                     a) Setting initialVal = 2    for a bool element.
 *                     b) Setting initialVal = 1.23 for an element that is not a 
 *                        float or double.
 *                     c) Setting initialVal = 2^33 for an element that only fits
 *                        <= 32 bits.
 *                     d) Setting initialVal = -2   for an unsigned element (e.g.
 *                        SV_T_UINT32).
 *
 *         2) Call StateVector::createNew (yourConfig).
 *         3) Use the read and write methods to interact with elements in the
 *            State Vector.
 *    
 * NOTE: 
 *   #1  This object is currently not threadsafe and should only be called from
 *       one thread.
 *   #2  While each compute node should only create 1 state vector, this object
 *       is not a singleton in order to facilitate testing.
 *   #3  Elements of type array are currently unsupported.
 *
 */

# ifndef STATE_VECTOR_HPP
# define STATE_VECTOR_HPP

#include <stdint.h>
#include <memory>
#include <vector>

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
     * @param   config              State Vector's config data.
     * @param   pStateVectorRet     Pointer to return State Vector.
     *
     * @ret     E_SUCCESS           State Vector successfully created.
     *          E_EMPTY_CONFIG      Config empty.
    */
    static Error_t createNew (StateVectorConfig_t& config,
                              std::shared_ptr<StateVector>& pStateVectorRet);

    /**
     * Read an element from the State Vector.  Defined in the header so that the
     * templatized functions do not need to each be instantiated explicitly.
     *
     * @param   elem         Element to read.
     * @param   valueRet     Variable to store element's value.
     *
     * @ret     E_SUCCESS    Element read successfully.
    */
    template<class Elem_T>
    Error_t read (StateVectorElement_t elem, Elem_T& valueRet)
    {
        // TODO(smasimore): Implement.
        return E_SUCCESS;
    }

    /**
     * Write a value to the State Vector.  Defined in the header so that the
     * templatized functions do not need to each be instantiated explicitly.
     *
     * @param   elem         Element to write to.
     * @param   value        Value to write.
     *
     * @ret     E_SUCCESS    Element written to successfully.
    */
    template<class Elem_T>
    Error_t write (StateVectorElement_t elem, Elem_T value)
    {
        // TODO(smasimore): Implement.
        return E_SUCCESS;
    }

private:

    /**
     * Constructor.
     */        
    StateVector (StateVectorConfig_t& config);

    /**
     * Verifies provided config.
     *
     * @ret     E_SUCCESS           Config valid.
     *          E_EMPTY_CONFIG      Config empty.
     *          E_EMPTY_ELEMS       Region's element list empty.
     *          E_DUPLICATE_REGION  Duplicate region.
     *          E_DUPLICATE_ELEM    Duplicate element.
     *          E_INVALID_ENUM      Invalid enumeration.
     */
    static Error_t verifyConfig (StateVectorConfig_t& config);
};

#endif
