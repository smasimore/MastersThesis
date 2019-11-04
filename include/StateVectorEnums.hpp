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
    SV_REG_TEST2,

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
    SV_ELEM_TEST3,
    SV_ELEM_TEST4,
    SV_ELEM_TEST5,
    SV_ELEM_TEST6,
    SV_ELEM_TEST7,
    SV_ELEM_TEST8,
    SV_ELEM_TEST9,
    SV_ELEM_TEST10,
    SV_ELEM_TEST11,
    SV_ELEM_TEST12,
    SV_ELEM_TEST13,
    SV_ELEM_TEST14,
    SV_ELEM_TEST15,
    SV_ELEM_TEST16,
    SV_ELEM_TEST17,
    SV_ELEM_TEST18,
    SV_ELEM_TEST19,
    SV_ELEM_TEST20,
    SV_ELEM_TEST21,
    SV_ELEM_TEST22,
    SV_ELEM_TEST23,
    SV_ELEM_TEST24,
    SV_ELEM_TEST25,
    SV_ELEM_TEST26,
    SV_ELEM_TEST27,
    SV_ELEM_TEST28,
    SV_ELEM_TEST29,
    SV_ELEM_TEST30,
    SV_ELEM_TEST31,
    SV_ELEM_TEST32,
    SV_ELEM_TEST33,
    SV_ELEM_TEST34,
    SV_ELEM_TEST35,
    SV_ELEM_TEST36,
    SV_ELEM_TEST37,
    SV_ELEM_TEST38,
    SV_ELEM_TEST39,
    SV_ELEM_TEST40,
    SV_ELEM_TEST41,
    SV_ELEM_TEST42,
    SV_ELEM_TEST43,
    SV_ELEM_TEST44,
    SV_ELEM_TEST45,

    SV_ELEM_LAST
};

#endif
