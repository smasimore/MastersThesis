/* All #include statements should come before the TestHelpers include */
#include "Actions.hpp"
#include "StateMachine.hpp"

#include "TestHelpers.hpp"

/**
 * Check actions to execute, execute them, and verify DV changed to expected
 * values.
 *
 * @param  kTimeS    Time in seconds to use as time elapsed in state.
 * @param  kExpVals  Struct of values expected for each DV elem.
 */
#define EXECUTE_AND_CHECK_ACTIONS(kTimeS, kExpVals)                            \
{                                                                              \
    std::vector<std::shared_ptr<Actions::ActionBase>> actionsToExecute;        \
    Time::TimeNs_t timeNs = kTimeS * Time::NS_IN_S;                       \
    CHECK_SUCCESS (pActions->checkActions (timeNs, actionsToExecute));         \
    for (std::shared_ptr<Actions::ActionBase> action : actionsToExecute )      \
    {                                                                          \
        CHECK_SUCCESS (action->execute (pDv));                                 \
    }                                                                          \
    DvVals actualVals = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                     \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0,  actualVals.u8));                 \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST1,  actualVals.u16));                \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST2,  actualVals.u32));                \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST3,  actualVals.u64));                \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST4,  actualVals.i8));                 \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST5,  actualVals.i16));                \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST6,  actualVals.i32));                \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST7,  actualVals.i64));                \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST8,  actualVals.fl));                 \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST9,  actualVals.db));                 \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST10, actualVals.bl));                 \
    CHECK_EQUAL (kExpVals.u8,  actualVals.u8);                                 \
    CHECK_EQUAL (kExpVals.u16, actualVals.u16);                                \
    CHECK_EQUAL (kExpVals.u32, actualVals.u32);                                \
    CHECK_EQUAL (kExpVals.u64, actualVals.u64);                                \
    CHECK_EQUAL (kExpVals.i8,  actualVals.i8);                                 \
    CHECK_EQUAL (kExpVals.i16, actualVals.i16);                                \
    CHECK_EQUAL (kExpVals.i32, actualVals.i32);                                \
    CHECK_EQUAL (kExpVals.i64, actualVals.i64);                                \
    CHECK_EQUAL (kExpVals.fl,  actualVals.fl);                                 \
    CHECK_EQUAL (kExpVals.db,  actualVals.db);                                 \
    CHECK_EQUAL (kExpVals.bl,  actualVals.bl);                                 \
}

/**
 * Struct to store DV vals for verification.
 */
struct DvVals
{
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t   i8;
    int16_t  i16;
    int32_t  i32;
    int64_t  i64;
    float    fl;
    double   db;
    bool     bl;
};

/**
 * Data Vector config to support global Actions config.
 */
static DataVector::Config_t gDvConfig =
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT8  ( DV_ELEM_TEST0,   0       ),
        DV_ADD_UINT16 ( DV_ELEM_TEST1,   0       ),
        DV_ADD_UINT32 ( DV_ELEM_TEST2,   0       ),
        DV_ADD_UINT64 ( DV_ELEM_TEST3,   0       ),
        DV_ADD_INT8   ( DV_ELEM_TEST4,   0       ),
        DV_ADD_INT16  ( DV_ELEM_TEST5,   0       ),
        DV_ADD_INT32  ( DV_ELEM_TEST6,   0       ),
        DV_ADD_INT64  ( DV_ELEM_TEST7,   0       ),
        DV_ADD_FLOAT  ( DV_ELEM_TEST8,   0       ),
        DV_ADD_DOUBLE ( DV_ELEM_TEST9,   0       ),
        DV_ADD_BOOL   ( DV_ELEM_TEST10,  false   ),
        DV_ADD_UINT32 ( DV_ELEM_STATE,   STATE_A ),
    }},
};

/**
 * Actions config with 1 of each element type.
 */
static Actions::Config_t gActionsConfig =
{
    {0 * Time::NS_IN_S, 
        {
            ACT_CREATE_UINT8   ( DV_ELEM_TEST0,    1      ),
            ACT_CREATE_UINT16  ( DV_ELEM_TEST1,    10     ),
            ACT_CREATE_UINT32  ( DV_ELEM_TEST2,    20     ),
        }}, 

    {.5 * Time::NS_IN_S,
        {
            ACT_CREATE_UINT64  ( DV_ELEM_TEST3,    500    ),
            ACT_CREATE_INT8    ( DV_ELEM_TEST4,   -1      ),
            ACT_CREATE_INT16   ( DV_ELEM_TEST5,   -10     ),
            ACT_CREATE_INT32   ( DV_ELEM_TEST6,   -20     ),
        }},

    {10 * Time::NS_IN_S,
        {
            ACT_CREATE_INT64   ( DV_ELEM_TEST7,    -500   ),
            ACT_CREATE_FLOAT   ( DV_ELEM_TEST8,    1.23   ),
            ACT_CREATE_DOUBLE  ( DV_ELEM_TEST9,    -4.567 ),
            ACT_CREATE_BOOL    ( DV_ELEM_TEST10,   true   ),
        }},
};

/* Tests verifying verifyConfig. */
TEST_GROUP (Actions_VerifyConfig)
{

};

/* Test creating a Actions object with a null DV. */
TEST (Actions_VerifyConfig, DvNull)
{
    std::shared_ptr<Actions> pActions = nullptr;
    CHECK_ERROR (Actions::createNew (gActionsConfig, nullptr, DV_ELEM_STATE, 
                                     pActions),
                 E_DATA_VECTOR_NULL);
}

/* Test a config with an elem not in DV. */
TEST (Actions_VerifyConfig, InvalidElem)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Set one action elem to not be in DV.
    Actions::Config_t actionsConfig = gActionsConfig;
    (++(actionsConfig.begin ()))->second[0]->mElem = DV_ELEM_TEST11;

    std::shared_ptr<Actions> pActions = nullptr;
    CHECK_ERROR (Actions::createNew (actionsConfig, pDv, DV_ELEM_STATE, 
                                     pActions),
                 E_INVALID_ELEM);
}

/* Test a config with an incorrect elem type. */
TEST (Actions_VerifyConfig, IncorrectElemType)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Set one action to have incorrect type.
    Actions::Config_t actionsConfig =
    {
        {0 * Time::NS_IN_S, 
            {
                ACT_CREATE_UINT8  ( DV_ELEM_TEST1,  1 ),
            }}, 
    };

    std::shared_ptr<Actions> pActions = nullptr;
    CHECK_ERROR (Actions::createNew (actionsConfig, pDv, DV_ELEM_STATE, 
                                     pActions),
                 E_INCORRECT_TYPE);
}

/* Test a config attempting to change the state DV elem value. */
TEST (Actions_VerifyConfig, InvalidAction)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Set one action elem attempting to change the state.
    Actions::Config_t actionsConfig =
    {
        {0 * Time::NS_IN_S, 
            {
                ACT_CREATE_UINT32  ( DV_ELEM_STATE,  STATE_B ),
            }}, 
    };

    std::shared_ptr<Actions> pActions = nullptr;
    CHECK_ERROR (Actions::createNew (actionsConfig, pDv, DV_ELEM_STATE, 
                                     pActions),
                 E_INVALID_ACTION);
}

/* Test a valid config. */
TEST (Actions_VerifyConfig, Success)
{
    INIT_DATA_VECTOR (gDvConfig);
    std::shared_ptr<Actions> pActions = nullptr;
    CHECK_SUCCESS (Actions::createNew (gActionsConfig, pDv, DV_ELEM_STATE, 
                                       pActions));
}

/* Tests verifying checkActions. */
TEST_GROUP (Actions_CheckActions)
{

};

/* Test success case. */
TEST (Actions_CheckActions, Success)
{
    INIT_DATA_VECTOR (gDvConfig);
    std::shared_ptr<Actions> pActions = nullptr;
    CHECK_SUCCESS (Actions::createNew (gActionsConfig, pDv, DV_ELEM_STATE, 
                                       pActions));

    // Execute actions with time elapsed = 0 seconds.
    DvVals expVals = {1, 10, 20, 0, 0, 0, 0, 0, 0, 0, false};
    EXECUTE_AND_CHECK_ACTIONS (0, expVals);

    // Execute actions with time elapsed = .4 seconds. Expect no change.
    EXECUTE_AND_CHECK_ACTIONS (.4, expVals);

    // Execute actions with time elapsed = .5 seconds.
    expVals = {1, 10, 20, 500, -1, -10, -20, 0, 0, 0, false};
    EXECUTE_AND_CHECK_ACTIONS (.5, expVals);

    // Execute actions with time elapsed = 5 seconds. Expect no change.
    EXECUTE_AND_CHECK_ACTIONS (5, expVals);

    // Execute actions with time elapsed = 10 seconds.
    expVals = {1, 10, 20, 500, -1, -10, -20, -500, 1.23, -4.567, true};
    EXECUTE_AND_CHECK_ACTIONS (10, expVals);
}
