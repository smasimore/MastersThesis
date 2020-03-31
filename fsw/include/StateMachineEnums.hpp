#ifndef STATE_MACHINE_ENUMS_HPP
#define STATE_MACHINE_ENUMS_HPP

#include <cstdint>

enum StateId_t : uint32_t
{
    /* Test States */
    STATE_A,
    STATE_B,
    STATE_C,
    STATE_D,
    STATE_E,

    /* Platform LED Test */
    STATE_START,
    STATE_LED_ON,
    STATE_LED_FLASH,
    STATE_END,
    STATE_ERROR,

    STATE_LAST,
};

enum TransitionComparison_t : uint8_t
{
    CMP_EQUALS,
    CMP_GREATER_THAN,
    CMP_GREATER_EQUALS_THAN,
    CMP_LESS_THAN,
    CMP_LESS_EQUALS_THAN,
    
    CMP_LAST
};

# endif
