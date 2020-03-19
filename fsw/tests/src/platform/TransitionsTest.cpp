/* All #include statements should come before the CppUTest include */
#include "Transitions.hpp"
#include "StateMachine.hpp"

#include "TestHelpers.hpp"

/**
 * Run checkTransitions and verify results.
 *
 * @param  kShouldTransition  True if should transition.
 * @param  kTargetState       Expected value of target state.
 */
#define CHECK_TRANSITION(kShouldTransition, kTargetState)                      \
{                                                                              \
    bool shouldTransition = false;                                             \
    StateId_t targetState = STATE_LAST;                                        \
    CHECK_SUCCESS (pT->checkTransitions (shouldTransition, targetState));      \
    CHECK_EQUAL (kShouldTransition, shouldTransition);                         \
    CHECK_EQUAL (kTargetState, targetState);                                   \
}

/**
 * Transitions config with each elem and comparison type.
 */
static std::vector<std::shared_ptr<Transitions::TransitionBase>> gTransConfig =
{
    TR_CREATE_UINT8  ( DV_ELEM_TEST0,   CMP_EQUALS,                1,     STATE_A ),
    TR_CREATE_UINT16 ( DV_ELEM_TEST1,   CMP_GREATER_THAN,          1,     STATE_B ),
    TR_CREATE_UINT32 ( DV_ELEM_TEST2,   CMP_GREATER_EQUALS_THAN,   2,     STATE_C ),
    TR_CREATE_UINT64 ( DV_ELEM_TEST3,   CMP_LESS_THAN,             1,     STATE_D ),
    TR_CREATE_INT8   ( DV_ELEM_TEST4,   CMP_LESS_EQUALS_THAN,     -2,     STATE_A ),
    TR_CREATE_INT16  ( DV_ELEM_TEST5,   CMP_EQUALS,                1,     STATE_B ),
    TR_CREATE_INT32  ( DV_ELEM_TEST6,   CMP_GREATER_THAN,          1,     STATE_C ),
    TR_CREATE_INT64  ( DV_ELEM_TEST7,   CMP_GREATER_EQUALS_THAN,   1,     STATE_D ),
    TR_CREATE_FLOAT  ( DV_ELEM_TEST8,   CMP_LESS_THAN,            -1.23,  STATE_A ),
    TR_CREATE_DOUBLE ( DV_ELEM_TEST9,   CMP_LESS_EQUALS_THAN,     -1.23,  STATE_B ),
    TR_CREATE_BOOL   ( DV_ELEM_TEST10,  CMP_EQUALS,               true,   STATE_C ),
};

/**
 * Data Vector config to support global Transitions config.
 */
static DataVector::Config_t gDvConfig =
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT8  ( DV_ELEM_TEST0,   0     ),
        DV_ADD_UINT16 ( DV_ELEM_TEST1,   0     ),
        DV_ADD_UINT32 ( DV_ELEM_TEST2,   0     ),
        DV_ADD_UINT64 ( DV_ELEM_TEST3,   2     ),
        DV_ADD_INT8   ( DV_ELEM_TEST4,   0     ),
        DV_ADD_INT16  ( DV_ELEM_TEST5,   0     ),
        DV_ADD_INT32  ( DV_ELEM_TEST6,   0     ),
        DV_ADD_INT64  ( DV_ELEM_TEST7,   0     ),
        DV_ADD_FLOAT  ( DV_ELEM_TEST8,   0     ),
        DV_ADD_DOUBLE ( DV_ELEM_TEST9,   0     ),
        DV_ADD_BOOL   ( DV_ELEM_TEST10,  false ),
    }},
};

/* Tests verifying verifyConfig. */
TEST_GROUP (Transitions_VerifyConfig)
{

};

/* Test creating a Transitions object with a null DV. */
TEST (Transitions_VerifyConfig, DvNull)
{
    std::shared_ptr<Transitions> pTrans = nullptr;
    CHECK_ERROR (Transitions::createNew (gTransConfig, nullptr, pTrans),
                 E_DATA_VECTOR_NULL);
}

/* Test a config with an invalid comparison enum. */
TEST (Transitions_VerifyConfig, InvalidCmpEnum)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::vector<std::shared_ptr<Transitions::TransitionBase>> transConfig =
    {
        TR_CREATE_UINT8  ( DV_ELEM_TEST0,  CMP_LAST,  1,  STATE_C ),
    };

    std::shared_ptr<Transitions> pTrans = nullptr;
    CHECK_ERROR (Transitions::createNew (transConfig, pDv, pTrans),
                 E_INVALID_ENUM);
}

/* Test a config with an invalid State ID. */
TEST (Transitions_VerifyConfig, InvalidStateIdEnum)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::vector<std::shared_ptr<Transitions::TransitionBase>> transConfig =
    {
        TR_CREATE_UINT8  ( DV_ELEM_TEST0,  CMP_EQUALS,  1,  STATE_LAST ),
    };

    std::shared_ptr<Transitions> pTrans = nullptr;
    CHECK_ERROR (Transitions::createNew (transConfig, pDv, pTrans),
                 E_INVALID_ENUM);
}

/* Test a config with an elem not in DV. */
TEST (Transitions_VerifyConfig, InvalidElem)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::vector<std::shared_ptr<Transitions::TransitionBase>> transConfig =
    {
        TR_CREATE_UINT8  ( DV_ELEM_TEST11,  CMP_EQUALS,  1,  STATE_A),
    };

    std::shared_ptr<Transitions> pTrans = nullptr;
    CHECK_ERROR (Transitions::createNew (transConfig, pDv, pTrans),
                 E_INVALID_ELEM);
}

/* Test a config with an incorrect elem type. */
TEST (Transitions_VerifyConfig, IncorrectElemType)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::vector<std::shared_ptr<Transitions::TransitionBase>> transConfig =
    {
        TR_CREATE_UINT16  ( DV_ELEM_TEST0,  CMP_EQUALS,  1,  STATE_A),
    };

    std::shared_ptr<Transitions> pTrans = nullptr;
    CHECK_ERROR (Transitions::createNew (transConfig, pDv, pTrans),
                 E_INCORRECT_TYPE);
}

/* Test a valid config. */
TEST (Transitions_VerifyConfig, Success)
{
    INIT_DATA_VECTOR (gDvConfig);
    std::shared_ptr<Transitions> pTrans = nullptr;
    CHECK_SUCCESS (Transitions::createNew (gTransConfig, pDv, pTrans));
}

/* Tests verifying checkTransitions. */
TEST_GROUP (Transitions_CheckTransitions)
{

};

TEST (Transitions_CheckTransitions, Success)
{
    INIT_DATA_VECTOR (gDvConfig);
    std::shared_ptr<Transitions> pT = nullptr;
    CHECK_SUCCESS (Transitions::createNew (gTransConfig, pDv, pT));

    // Expect no conditions met.
    CHECK_TRANSITION (false, STATE_LAST);

    // Test all transitions, starting with elem 10.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST10, (bool) true));
    CHECK_TRANSITION (true, STATE_C);

    // Elem 9.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST9, (double) -1.229));
    CHECK_TRANSITION (true, STATE_C);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST9, (double) -1.23));
    CHECK_TRANSITION (true, STATE_B);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST9, (double) -1.24));
    CHECK_TRANSITION (true, STATE_B);

    // Elem 8.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST8, (float) -1.23));
    CHECK_TRANSITION (true, STATE_B);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST8, (float) -1.231));
    CHECK_TRANSITION (true, STATE_A);

    // Elem 7.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST7, (int64_t) -1));
    CHECK_TRANSITION (true, STATE_A);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST7, (int64_t) 1));
    CHECK_TRANSITION (true, STATE_D);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST7, (int64_t) 2));
    CHECK_TRANSITION (true, STATE_D);

    // Elem 6.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST6, (int32_t) 1));
    CHECK_TRANSITION (true, STATE_D);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST6, (int32_t) 2));
    CHECK_TRANSITION (true, STATE_C);

    // Elem 5.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST5, (int16_t) 2));
    CHECK_TRANSITION (true, STATE_C);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST5, (int16_t) 1));
    CHECK_TRANSITION (true, STATE_B);

    // Elem 4.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST4, (int8_t) -1));
    CHECK_TRANSITION (true, STATE_B);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST4, (int8_t) -2));
    CHECK_TRANSITION (true, STATE_A);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST4, (int8_t) -3));
    CHECK_TRANSITION (true, STATE_A);

    // Elem 3.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, (uint64_t) 1));
    CHECK_TRANSITION (true, STATE_A);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, (uint64_t) 0));
    CHECK_TRANSITION (true, STATE_D);

    // Elem 2.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST2, (uint32_t) 1));
    CHECK_TRANSITION (true, STATE_D);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST2, (uint32_t) 2));
    CHECK_TRANSITION (true, STATE_C);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST2, (uint32_t) 3));
    CHECK_TRANSITION (true, STATE_C);

    // Elem 1.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST1, (uint16_t) 1));
    CHECK_TRANSITION (true, STATE_C);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST1, (uint16_t) 2));
    CHECK_TRANSITION (true, STATE_B);

    // Elem 0.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST0, (uint8_t) 2));
    CHECK_TRANSITION (true, STATE_B);
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST0, (uint8_t) 1));
    CHECK_TRANSITION (true, STATE_A);
}
