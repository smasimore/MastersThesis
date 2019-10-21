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
    SV_T_UINT8,
    SV_T_UINT16,
    SV_T_UINT32,
    SV_T_UINT64,
    SV_T_INT8,
    SV_T_INT16,
    SV_T_INT32,
    SV_T_INT64,
    SV_T_FLOAT,
    SV_T_DOUBLE,
    SV_T_BOOL,

    SV_T_LAST
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
