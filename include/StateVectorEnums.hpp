/**
 * Enumerations for StateVector. Defined in the global namespace so the enums 
 * names can be shorter (i.e. w/o namespace) throughout configs and flight 
 * software.
 */

# ifndef STATE_VECTOR_ENUMS_HPP
# define STATE_VECTOR_ENUMS_HPP

#include <stdint.h>

/**
 * State Vector element type enumeration.
 */
enum StateVectorElementType_t : uint8_t
{
    T_UINT8,
    T_UINT16,
    T_UINT32,
    T_UINT64,
    T_INT8,
    T_INT16,
    T_INT32,
    T_INT64,
    T_FLOAT,
    T_DOUBLE,
    T_BOOL,

    T_LAST
};

/**
 * Possible regions in State Vector.
 */
enum StateVectorRegion_t : uint32_t
{
    /* Test Regions */
    SV_REG_TEST0,
    SV_REG_TEST1,

    SV_REG_LAST
};

/**
 * Possible elements in State Vector.
 */
enum StateVectorElement_t : uint32_t
{
    /* Test Elements */
    SV_ELEM_TEST0,
    SV_ELEM_TEST1,
    SV_ELEM_TEST2,

    SV_ELEM_LAST
};

#endif
