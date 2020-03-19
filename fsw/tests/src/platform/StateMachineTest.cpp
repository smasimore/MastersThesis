#include <unistd.h>

#include "StateMachine.hpp"
#include "Errors.hpp"
#include "Log.hpp"

#include "TestHelpers.hpp"

/**
 * Default initial time for initialising SM.
 */
#define INITIAL_TIME_NS 0

/**
 * Check DV state against expected.
 *
 * @param  kExpState  State expected in DV.
 */
#define CHECK_STATE(kExpState)                                                 \
{                                                                              \
    uint32_t actualState = STATE_LAST;                                         \
    CHECK_SUCCESS (pDv->read (DV_ELEM_STATE, actualState));                    \
    CHECK_EQUAL (kExpState, actualState);                                      \
}

/**
 * Step the SM and check the Data Vector values.
 * values.
 *
 * @param  kTimeNs   Time in nanoseconds to use as current time.
 * @param  kExpVals  Struct of values expected for each DV elem.
 */
#define STEP_AND_CHECK_DV(kTimeNs, kExpVals)                                    \
{                                                                              \
    CHECK_SUCCESS (pSm->step (kTimeNs));                                        \
    DvVals actualVals = {STATE_A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};            \
    CHECK_SUCCESS (pDv->read (DV_ELEM_STATE,  actualVals.state));              \
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
    CHECK_EQUAL (kExpVals.state, actualVals.state);                            \
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
    uint32_t state;
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
 * DV config to go with gSmConfig.
 */
static DataVector::Config_t gDvConfig =
{
    {
        {DV_REG_TEST0,
        {
            DV_ADD_UINT32 (DV_ELEM_STATE,  STATE_A),
            DV_ADD_UINT8  (DV_ELEM_TEST0,  0      ),
            DV_ADD_UINT16 (DV_ELEM_TEST1,  0      ),
            DV_ADD_UINT32 (DV_ELEM_TEST2,  0      ),
            DV_ADD_UINT64 (DV_ELEM_TEST3,  0      ),
            DV_ADD_INT8   (DV_ELEM_TEST4,  0      ),
            DV_ADD_INT16  (DV_ELEM_TEST5,  0      ),
            DV_ADD_INT32  (DV_ELEM_TEST6,  0      ),
            DV_ADD_INT64  (DV_ELEM_TEST7,  0      ),
            DV_ADD_FLOAT  (DV_ELEM_TEST8,  0      ),
            DV_ADD_DOUBLE (DV_ELEM_TEST9,  0      ),
            DV_ADD_BOOL   (DV_ELEM_TEST10, false  ),
        }},
    }
};

static StateMachine::Config_t gSmConfig =
{
    //////////////////////////////// STATE_A ///////////////////////////////////
    // ID
    {STATE_A,
    // ACTIONS
    {{0 * Time::NS_IN_SECOND,
         {ACT_CREATE_UINT8  (DV_ELEM_TEST0,  1),
          ACT_CREATE_UINT16 (DV_ELEM_TEST1,  1)}},
     {1 * Time::NS_IN_SECOND,
         {ACT_CREATE_UINT32 (DV_ELEM_TEST2,  1),
          ACT_CREATE_UINT8  (DV_ELEM_TEST0,  2)}}},
    // TRANSITIONS
    {TR_CREATE_UINT8  ( DV_ELEM_TEST0,   CMP_EQUALS,               2,  STATE_B),
     TR_CREATE_UINT16 ( DV_ELEM_TEST1,   CMP_GREATER_THAN,         1,  STATE_C), 
     TR_CREATE_UINT32 ( DV_ELEM_TEST2,   CMP_GREATER_EQUALS_THAN,  2,  STATE_D)}},


    //////////////////////////////// STATE_B ///////////////////////////////////
    // ID
    {STATE_B,
    // ACTIONS
    {{0 * Time::NS_IN_SECOND,
         {ACT_CREATE_UINT64 (DV_ELEM_TEST3,  1),
          ACT_CREATE_INT8   (DV_ELEM_TEST4,  2)}},
     {.5 * Time::NS_IN_SECOND,
         {ACT_CREATE_INT16  (DV_ELEM_TEST5,  3),
          ACT_CREATE_INT8   (DV_ELEM_TEST4,  0)}}},
    // TRANSITIONS
    {TR_CREATE_UINT64 ( DV_ELEM_TEST3,   CMP_LESS_THAN,            1,  STATE_A),
     TR_CREATE_INT8   ( DV_ELEM_TEST4,   CMP_LESS_EQUALS_THAN,     1,  STATE_C),
     TR_CREATE_INT16  ( DV_ELEM_TEST5,   CMP_EQUALS,               1,  STATE_D)}},


    //////////////////////////////// STATE_C ///////////////////////////////////
    // ID
    {STATE_C,
    // ACTIONS
    {{1 * Time::NS_IN_SECOND,
         {ACT_CREATE_INT32  (DV_ELEM_TEST6,  1)}},
     {2 * Time::NS_IN_SECOND,
         {ACT_CREATE_INT64  (DV_ELEM_TEST7, -1),
          ACT_CREATE_FLOAT  (DV_ELEM_TEST8, -1.1)}}},
    // TRANSITIONS
    {TR_CREATE_INT32  ( DV_ELEM_TEST6,   CMP_GREATER_THAN,         1,  STATE_A),
     TR_CREATE_INT64  ( DV_ELEM_TEST7,   CMP_GREATER_EQUALS_THAN,  1,  STATE_B),
     TR_CREATE_FLOAT  ( DV_ELEM_TEST8,   CMP_LESS_THAN,           -1,  STATE_D)}},


    //////////////////////////////// STATE_D ///////////////////////////////////
    // ID
    {STATE_D,
    // ACTIONS
    {{0 * Time::NS_IN_SECOND,
         {ACT_CREATE_BOOL   (DV_ELEM_TEST10, true)}},
     {1 * Time::NS_IN_SECOND,
         {ACT_CREATE_DOUBLE (DV_ELEM_TEST9, -1)}}},
    // TRANSITIONS
    {TR_CREATE_DOUBLE ( DV_ELEM_TEST9,   CMP_LESS_EQUALS_THAN,    -1,  STATE_A),
     TR_CREATE_BOOL   ( DV_ELEM_TEST10,  CMP_EQUALS,           false,  STATE_D)}},
};

/***************************** CONFIG TESTS ***********************************/

/* Tests to verify error handling of SM config. */
TEST_GROUP (StateMachine_Config)
{
};

/* Test null DV ptr. */
TEST (StateMachine_Config, DvNull)
{
    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (gSmConfig, nullptr, INITIAL_TIME_NS, 
                                          DV_ELEM_STATE, pSm),
                 E_DATA_VECTOR_NULL);
}

/* Test invalid state elem. */
TEST (StateMachine_Config, InvalidStateElem)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (gSmConfig, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_TEST11, pSm),
                 E_INVALID_ELEM);
}

/* Test non-uint32_t state elem. */
TEST (StateMachine_Config, IncorrectStateElemType)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (gSmConfig, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_TEST0, pSm),
                 E_INCORRECT_TYPE);
}

/* Test config with no states. */
TEST (StateMachine_Config, EmptyConfig)
{
    INIT_DATA_VECTOR (gDvConfig);

    StateMachine::Config_t config = {};

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (config, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_STATE, pSm),
                 E_NO_STATES);
}

/* Test config with dupe state ID. */
TEST (StateMachine_Config, DupeState)
{
    INIT_DATA_VECTOR (gDvConfig);

    StateMachine::Config_t config = {{STATE_A, {}, {}}, {STATE_A, {}, {}}};

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (config, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_STATE, pSm),
                 E_DUPLICATE_STATE);
}

/* Test config with invalid transition target state. */
TEST (StateMachine_Config, InvalidTransition)
{
    INIT_DATA_VECTOR (gDvConfig);

    StateMachine::Config_t config = 
    {
        {STATE_A, 
         {},
         {TR_CREATE_UINT8  ( DV_ELEM_TEST0,   CMP_EQUALS,  1,  STATE_B)}},
    };

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (config, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_STATE, pSm),
                 E_INVALID_TRANSITION);
}

/* Test config with invalid state ID. */
TEST (StateMachine_Config, InvalidStateId)
{
    INIT_DATA_VECTOR (gDvConfig);

    StateMachine::Config_t config = {{STATE_LAST, {}, {}}};

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (config, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_STATE, pSm),
                 E_INVALID_ENUM);
}

/* Test initial state in DV invalid. */
TEST (StateMachine_Config, InvalidInitialState)
{
    INIT_DATA_VECTOR (gDvConfig);

    StateMachine::Config_t config = {{STATE_B, {}, {}}};

    std::unique_ptr<StateMachine> pSm;
    CHECK_ERROR (StateMachine::createNew (config, pDv, INITIAL_TIME_NS, 
                                          DV_ELEM_STATE, pSm),
                 E_STATE_NOTFOUND);
}

/* Test with an action attempting to change the state. */
TEST (StateMachine_Config, InvalidAction)
{
    DataVector::Config_t dvConfig = {
        {
            {DV_REG_TEST0,
            {
                DV_ADD_INT16  ( DV_ELEM_TEST0,  0       ),
                DV_ADD_BOOL   ( DV_ELEM_TEST1,  false   ),
                DV_ADD_UINT64 ( DV_ELEM_TEST2,  0       ),
                DV_ADD_UINT32 ( DV_ELEM_STATE,  STATE_A ),
            }},
        }
    };

    Actions::Config_t actionsConfigA =
    {
        {0 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_INT16  ( DV_ELEM_TEST0,    1),
                ACT_CREATE_UINT64 ( DV_ELEM_TEST2,    1)
            }},

        {1 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_BOOL   ( DV_ELEM_TEST1,    true),
            }},
    };

    // Create Actions config with an transition overwriting state elem.
    Actions::Config_t actionsConfigB =
    {
        {0 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_INT16  ( DV_ELEM_TEST0,    2),
                ACT_CREATE_BOOL   ( DV_ELEM_TEST1,    false)
            }},

        {1 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_UINT64   ( DV_ELEM_TEST2,  2),
                ACT_CREATE_UINT32   ( DV_ELEM_STATE,  STATE_B)
            }},
    };

    // Create vector of states for createNew function
    StateMachine::Config_t smConfig =
    {
        {STATE_A, actionsConfigA, {}},
        {STATE_B, actionsConfigB, {}}
    };

    // Init other components
    INIT_DATA_VECTOR (dvConfig);
    Time::TimeNs_t timeNs = 0;

    // Attempt to create State Machine, should fail due to invalid action.
    std::unique_ptr<StateMachine> pSm (nullptr);
    CHECK_ERROR (StateMachine::createNew (smConfig, pDv, timeNs, DV_ELEM_STATE, 
                                          pSm),
                 E_INVALID_ACTION);
}

/* Test valid config. */
TEST (StateMachine_Config, Success)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<StateMachine> pSm;
    CHECK_SUCCESS (StateMachine::createNew (gSmConfig, pDv, INITIAL_TIME_NS, 
                                            DV_ELEM_STATE, pSm));
}

/****************************** STEP TESTS ************************************/

TEST_GROUP (StateMachine_Step)
{
};

/* Test invalid current time. */
TEST (StateMachine_Step, InvalidTime)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<StateMachine> pSm;
    CHECK_SUCCESS (StateMachine::createNew (gSmConfig, pDv, INITIAL_TIME_NS + 1, 
                                            DV_ELEM_STATE, pSm));
    CHECK_ERROR (pSm->step (INITIAL_TIME_NS), E_INVALID_TIME);
}

/* Test no actions or transitions. */
TEST (StateMachine_Step, NoActionsOrTransitions)
{
    INIT_DATA_VECTOR (gDvConfig);

    StateMachine::Config_t config = {{STATE_A, {}, {}}};

    std::unique_ptr<StateMachine> pSm;
    CHECK_SUCCESS (StateMachine::createNew (config, pDv, INITIAL_TIME_NS, 
                                            DV_ELEM_STATE, pSm));

    DvVals expVals = {STATE_A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (0 * Time::NS_IN_SECOND, expVals);
}

/* Test iterator reset. */
TEST (StateMachine_Step, ActionsIteratorReset)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<StateMachine> pSm;
    CHECK_SUCCESS (StateMachine::createNew (gSmConfig, pDv, 
                                            INITIAL_TIME_NS, DV_ELEM_STATE, 
                                            pSm));

    // Expect first set of STATE_A's actions to have run.
    DvVals expVals = {STATE_A, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (0 * Time::NS_IN_SECOND, expVals);

    // Expect second set of STATE_A's actions to have run and to remain in 
    // STATE_A since the transition check runs before actions are executed. 
    expVals = {STATE_A, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (1 * Time::NS_IN_SECOND, expVals);

    // Expect transition to STATE_B and for STATE_B's first set of actions to 
    // have run. 
    expVals = {STATE_B, 2, 1, 1, 1, 2, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (2 * Time::NS_IN_SECOND, expVals);
    
    // Reset first set of DV values set by STATE_A.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST0, (uint8_t) 0));
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST1, (uint16_t) 0));

    // Trigger transition back to STATE_A. Expect 2nd set of STATE_B actions to
    // not run, and expect first set of STATE_A's first set of actions to have 
    // run again.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, (uint64_t) 0));
    expVals = {STATE_A, 1, 1, 1, 0, 2, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (3 * Time::NS_IN_SECOND, expVals);
}

/* Cycle through every state and action. */
TEST (StateMachine_Step, Success)
{

    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<StateMachine> pSm;
    CHECK_SUCCESS (StateMachine::createNew (gSmConfig, pDv, 
                                            INITIAL_TIME_NS, DV_ELEM_STATE, 
                                            pSm));

    // Expect first set of STATE_A's actions to have run.
    DvVals expVals = {STATE_A, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (0 * Time::NS_IN_SECOND, expVals);

    // Expect second set of STATE_A's actions to have run and to remain in 
    // STATE_A since the transition check runs before actions are executed. 
    expVals = {STATE_A, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (1 * Time::NS_IN_SECOND, expVals);

    // Expect transition to STATE_B and for STATE_B's first set of actions to 
    // have run. 
    expVals = {STATE_B, 2, 1, 1, 1, 2, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (2 * Time::NS_IN_SECOND, expVals);

    // Expect no change.
    expVals = {STATE_B, 2, 1, 1, 1, 2, 0, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (2.25 * Time::NS_IN_SECOND, expVals);

    // Expect second set of STATE_B's actions to have run. 
    expVals = {STATE_B, 2, 1, 1, 1, 0, 3, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (2.5 * Time::NS_IN_SECOND, expVals);

    // Expect transition to STATE_C and for no STATE_C actions to have run. 
    expVals = {STATE_C, 2, 1, 1, 1, 0, 3, 0, 0, 0, 0, false};
    STEP_AND_CHECK_DV (3 * Time::NS_IN_SECOND, expVals);

    // Expect STATE_C's first set of actions to have run. 
    expVals = {STATE_C, 2, 1, 1, 1, 0, 3, 1, 0, 0, 0, false};
    STEP_AND_CHECK_DV (4 * Time::NS_IN_SECOND, expVals);

    // Expect STATE_C's second set of actions to have run. 
    expVals = {STATE_C, 2, 1, 1, 1, 0, 3, 1, -1, -1.1, 0, false};
    STEP_AND_CHECK_DV (5 * Time::NS_IN_SECOND, expVals);

    // Expect transition to STATE_D and for STATE_D's first set of actions to 
    // have run. 
    expVals = {STATE_D, 2, 1, 1, 1, 0, 3, 1, -1, -1.1, 0, true};
    STEP_AND_CHECK_DV (6 * Time::NS_IN_SECOND, expVals);

    // Expect STATE_D's second set of actions to have run. 
    expVals = {STATE_D, 2, 1, 1, 1, 0, 3, 1, -1, -1.1, -1, true};
    STEP_AND_CHECK_DV (7 * Time::NS_IN_SECOND, expVals);

    // Expect transition back to STATE_A and for STATE_A's first set of actions
    // to run again.
    expVals = {STATE_A, 1, 1, 1, 1, 0, 3, 1, -1, -1.1, -1, true};
    STEP_AND_CHECK_DV (7 * Time::NS_IN_SECOND, expVals);
}

/* Test State Machine integration with State, Transitions, and Time classes. */
TEST (StateMachine_Step, Transitions)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Set initial state.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST5, (int16_t) 18));
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST10, (bool) false));
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, (uint64_t) 16));

    Transitions::Config_t transitionsA =
    {
        TR_CREATE_BOOL   ( DV_ELEM_TEST10,  CMP_EQUALS,        true,  STATE_B),
        TR_CREATE_UINT64 ( DV_ELEM_TEST3,   CMP_GREATER_THAN,  16,    STATE_C)
    };
    Transitions::Config_t transitionsB =
    {
        TR_CREATE_INT16  ( DV_ELEM_TEST5,   CMP_EQUALS,        19,    STATE_A),
        TR_CREATE_UINT64 ( DV_ELEM_TEST3,   CMP_GREATER_THAN,  16,    STATE_C)
    };
    Transitions::Config_t transitionsC =
    {
        TR_CREATE_INT16  ( DV_ELEM_TEST5,   CMP_EQUALS,        19,    STATE_A),
        TR_CREATE_BOOL   ( DV_ELEM_TEST10,  CMP_EQUALS,        true,  STATE_B),
    };

    // Create config.
    StateMachine::Config_t smConfig
        = {{STATE_A, {}, transitionsA},
           {STATE_B, {}, transitionsB},
           {STATE_C, {}, transitionsC}};

    // Init State Machine and Time. 
    Time* pTime = nullptr;
    Time::TimeNs_t timeNs = 0;
    std::unique_ptr<StateMachine> pSm (nullptr);
    CHECK_SUCCESS (Time::getInstance (pTime));
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    CHECK_SUCCESS (StateMachine::createNew (smConfig, pDv, timeNs,
                                            DV_ELEM_STATE, pSm));

    // Verify initial state.
    CHECK_STATE (STATE_A);

    // Step SM and expect no change in state.
    pSm->step (timeNs);
    CHECK_STATE (STATE_A);

    // Transition to StateC.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, (uint64_t) 17));
    CHECK_SUCCESS (pSm->step (timeNs));
    CHECK_STATE (STATE_C);

    // Transition to StateA.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST5, (int16_t) 19));
    CHECK_SUCCESS (pSm->step (timeNs));
    CHECK_STATE (STATE_A);

    // Remain in StateA.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST3, (uint64_t) 15));
    CHECK_SUCCESS (pSm->step (timeNs));
    CHECK_STATE (STATE_A);

    // Transition to StateB.
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST10, (bool) true));
    CHECK_SUCCESS (pSm->step (timeNs));
    CHECK_STATE (STATE_B);
}

/* Test State Machine integration with State, Actions, and Time classes. */
TEST (StateMachine_Step, Actions)
{
    INIT_DATA_VECTOR (gDvConfig);

    Actions::Config_t actionsConfigA =
    {
        {0 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_INT16  ( DV_ELEM_TEST5,    1),
                ACT_CREATE_UINT64 ( DV_ELEM_TEST3,    1)
            }},

        {1 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_BOOL   ( DV_ELEM_TEST10,    true),
            }},
    };

    Actions::Config_t actionsConfigB =
    {
        {0 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_INT16  ( DV_ELEM_TEST5,    2),
                ACT_CREATE_BOOL  ( DV_ELEM_TEST10,    false)
            }},

        {1 * Time::NS_IN_SECOND,
            {
                ACT_CREATE_UINT64   ( DV_ELEM_TEST3,  2),
            }},
    };

    // Create vector of states for createNew function
    StateMachine::Config_t smConfig =
    {
        {STATE_A, actionsConfigA, {}},
        {STATE_B, actionsConfigB, {}}
    };

    // Init State Machine and Data Vector.
    Time* pTime = nullptr;
    Time::TimeNs_t timeNs = 0;
    std::unique_ptr<StateMachine> pSm (nullptr);
    CHECK_SUCCESS (Time::getInstance (pTime));
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    CHECK_SUCCESS (StateMachine::createNew (smConfig, pDv, timeNs,
                                            DV_ELEM_STATE, pSm));

    // Execute A's actions (expect first set of actions to execute);
    DvVals expVals = {STATE_A, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, false};
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    STEP_AND_CHECK_DV (timeNs, expVals);

    // Execute A's actions (expect no change).
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    STEP_AND_CHECK_DV (timeNs, expVals);

    // Sleep for 1s.
    sleep (1);

    // Execute A's actions (expect 2nd set to run).
    expVals.bl = true;
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    STEP_AND_CHECK_DV (timeNs, expVals);

    // Switch to B.
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    CHECK_SUCCESS (pSm->switchState (STATE_B, timeNs));

    // Sleep for 1s.
    sleep (1);

    // Execute B's actions (expect all to run)
    expVals.state = STATE_B;
    expVals.u64 = 2;
    expVals.bl = false;
    expVals.i16 = 2;
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    STEP_AND_CHECK_DV (timeNs, expVals);

    // Switch back to A to verify actions will re-run.
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    CHECK_SUCCESS (pSm->switchState (STATE_A, timeNs));

    // First expect only first set of actions to run.
    expVals.state = STATE_A;
    expVals.u64 = 1;
    expVals.i16 = 1;
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    STEP_AND_CHECK_DV (timeNs, expVals);

    // Sleep for 1s.
    sleep (1);

    // Expect remaining actions to run.
    expVals.bl = true;
    CHECK_SUCCESS (pTime->getTimeNs (timeNs));
    STEP_AND_CHECK_DV (timeNs, expVals);
}
