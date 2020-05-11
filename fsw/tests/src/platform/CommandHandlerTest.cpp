#include <limits>
#include <type_traits>

#include "CommandHandler.hpp"

/* All #include statements should come before the CppUTest include */
#include "TestHelpers.hpp"

/**
 * Initialize DV and handler using global configs.
 */
#define INIT_CH_SUCCESS                                                        \
    INIT_DATA_VECTOR (gDvConfig);                                              \
    std::unique_ptr<CommandHandler> pCh = nullptr;                             \
    CHECK_SUCCESS (CommandHandler::createNew (gChConfig, pDv, pCh));         

/**
 * Check values of all elems in global DV config.
 *
 * @param  kCmd             Expected value in cmd element.
 * @param  kCmdReq          Expected value in cmd req element.
 * @param  kCmdWriteElem    Expected value in cmd write elem element.
 * @param  kCmdWriteVal     Expected value in cmd write val element.
 * @param  kCmdReqNum       Expected value in last cmd req element.
 * @param  kLastCmdProcNum  Expected value in last cmd proc element.
 * @param  kTest0           Expected value in test0 element.
 * @param  kTestActVar0     Var to store test0 value. Necessary since type is
 *                          dependent on test case.
 */
#define CHECK_DV(kCmd, kCmdReq, kCmdWriteElem, kCmdWriteVal, kCmdReqNum,       \
                 kLastCmdProcNum, kTest0, kTest0ActVar)                        \
{                                                                              \
    uint8_t cmdAct = CMD_LAST;                                                 \
    uint8_t cmdReqAct = CMD_LAST;                                              \
    uint32_t cmdWriteElemAct = DV_ELEM_LAST;                                   \
    uint64_t cmdWriteValAct = 0;                                               \
    uint32_t cmdReqNumAct = 0;                                                 \
    uint32_t lastCmdProcNumAct = 0;                                            \
    kTest0ActVar = 0;                                                          \
    CHECK_SUCCESS (pDv->read (DV_ELEM_CMD, cmdAct));                           \
    CHECK_SUCCESS (pDv->read (DV_ELEM_CMD_REQ, cmdReqAct));                    \
    CHECK_SUCCESS (pDv->read (DV_ELEM_CMD_WRITE_ELEM, cmdWriteElemAct));       \
    CHECK_SUCCESS (pDv->read (DV_ELEM_CMD_WRITE_VAL, cmdWriteValAct));         \
    CHECK_SUCCESS (pDv->read (DV_ELEM_CMD_REQ_NUM, cmdReqNumAct));             \
    CHECK_SUCCESS (pDv->read (DV_ELEM_LAST_CMD_PROC_NUM, lastCmdProcNumAct));  \
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0, kTest0ActVar));                   \
    CHECK_EQUAL (kCmd, cmdAct);                                                \
    CHECK_EQUAL (kCmdReq, cmdReqAct);                                          \
    CHECK_EQUAL (kCmdWriteElem, cmdWriteElemAct);                              \
    CHECK_EQUAL (kCmdWriteVal, cmdWriteValAct);                                \
    CHECK_EQUAL (kCmdReqNum, cmdReqNumAct);                                    \
    CHECK_EQUAL (kLastCmdProcNum, lastCmdProcNumAct);                          \
    CHECK_EQUAL (kTest0, kTest0ActVar);                                        \
}

/**
 * Global DV config containing all handler elems.
 */
static DataVector::Config_t gDvConfig = 
{
    {DV_REG_TEST0,
    {
        DV_ADD_UINT8  ( DV_ELEM_CMD,               CMD_NONE     ),
        DV_ADD_UINT8  ( DV_ELEM_CMD_REQ,           CMD_NONE     ),
        DV_ADD_UINT32 ( DV_ELEM_CMD_WRITE_ELEM,    DV_ELEM_LAST ),
        DV_ADD_UINT64 ( DV_ELEM_CMD_WRITE_VAL,     0            ),
        DV_ADD_UINT32 ( DV_ELEM_CMD_REQ_NUM,       0            ),
        DV_ADD_UINT32 ( DV_ELEM_LAST_CMD_PROC_NUM, 0            ),
        DV_ADD_UINT8  ( DV_ELEM_TEST0,             0            ),
    }},
};

/**
 * Global CH valid config.
 */
static CommandHandler::Config_t gChConfig
{
    DV_ELEM_CMD,
    DV_ELEM_CMD_REQ,
    DV_ELEM_CMD_WRITE_ELEM,
    DV_ELEM_CMD_WRITE_VAL,
    DV_ELEM_CMD_REQ_NUM,
    DV_ELEM_LAST_CMD_PROC_NUM,
};

/* Test config error handling. */
TEST_GROUP (CommandHandler_Config)
{
};

/* Test initialization of handler with null DV. */
TEST (CommandHandler_Config, NullDv)
{
    std::unique_ptr<CommandHandler> pCh = nullptr;
    CHECK_ERROR (CommandHandler::createNew (gChConfig, nullptr, pCh),
                   E_DATA_VECTOR_NULL);
}

/* Test initialization of handler with elements not in DV. */
TEST (CommandHandler_Config, InvalidElem)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<CommandHandler> pCh = nullptr;

    // cmd
    CommandHandler::Config_t chConfig = gChConfig;
    chConfig.cmd = DV_ELEM_TEST1;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_ELEM);

    // cmd req
    chConfig = gChConfig;
    chConfig.cmdReq = DV_ELEM_TEST1;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_ELEM);

    // write elem
    chConfig = gChConfig;
    chConfig.cmdWriteElem = DV_ELEM_TEST1;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_ELEM);

    // write val
    chConfig = gChConfig;
    chConfig.cmdWriteVal = DV_ELEM_TEST1;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_ELEM);

    // last req num
    chConfig = gChConfig;
    chConfig.cmdReqNum = DV_ELEM_TEST1;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_ELEM);

    // last proc num
    chConfig = gChConfig;
    chConfig.lastCmdProcNum = DV_ELEM_TEST1;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_ELEM);
}

/* Test initialization of handler with invalid element types. */
TEST (CommandHandler_Config, InvalidType)
{
    INIT_DATA_VECTOR (gDvConfig);

    std::unique_ptr<CommandHandler> pCh = nullptr;

    // cmd
    CommandHandler::Config_t chConfig = gChConfig;
    chConfig.cmd = DV_ELEM_CMD_WRITE_ELEM;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_TYPE);

    // cmd req
    chConfig = gChConfig;
    chConfig.cmdReq = DV_ELEM_CMD_WRITE_ELEM;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_TYPE);

    // write elem
    chConfig = gChConfig;
    chConfig.cmdWriteElem = DV_ELEM_CMD;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_TYPE);

    // write val
    chConfig = gChConfig;
    chConfig.cmdWriteVal = DV_ELEM_CMD;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_TYPE);

    // last req num
    chConfig = gChConfig;
    chConfig.cmdReqNum = DV_ELEM_CMD;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_TYPE);

    // last proc num
    chConfig = gChConfig;
    chConfig.lastCmdProcNum = DV_ELEM_CMD;
    CHECK_ERROR (CommandHandler::createNew (chConfig, pDv, pCh),
                 E_INVALID_TYPE);
}

/* Test successful initialization of handler. */
TEST (CommandHandler_Config, Success)
{
    INIT_CH_SUCCESS;
}

/* Test the handler run function. */
TEST_GROUP (CommandHandler_Run)
{
};

/* Test running handler with invalid cmd req. */
TEST (CommandHandler_Run, InvalidCmdReq)
{
    INIT_CH_SUCCESS;

    // Set invalid cmd req and cmd req number.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_LAST));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 1));

    // Run handler.
    CHECK_ERROR (pCh->run (), E_INVALID_CMD);
}

/* Test running handler with write elem not in DV. */
TEST (CommandHandler_Run, InvalidWriteElem)
{
    INIT_CH_SUCCESS;

    // Set WRITE cmd, invalid write elem, and cmd req number.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_WRITE));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_WRITE_ELEM, 
                               (uint32_t) DV_ELEM_TEST1));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 1));

    // Run handler.
    CHECK_ERROR (pCh->run (), E_INVALID_ELEM);
}

/* Test successful run of launch cmd req. */
TEST (CommandHandler_Run, LaunchSuccess)
{
    INIT_CH_SUCCESS;

    // Verify initial state.
    uint8_t test0Var;
    CHECK_DV (CMD_NONE,     // cmd
              CMD_NONE,     // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              0,            // cmd req num
              0,            // proc req num
              0,            // test0 val
              test0Var);

    // Set LAUNCH cmd and cmd req number.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_LAUNCH));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 1));

    // Run handler.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_LAUNCH,   // cmd
              CMD_LAUNCH,   // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              1,            // cmd req num
              1,            // proc req num
              0,            // test0 val
              test0Var);

    // Run handler again with no new command.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_NONE,     // cmd
              CMD_LAUNCH,   // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              1,            // cmd req num
              1,            // proc req num
              0,            // test0 val
              test0Var);
}

/* Test successful run of abort cmd req. */
TEST (CommandHandler_Run, AbortSuccess)
{
    INIT_CH_SUCCESS;

    // Verify initial state.
    uint8_t test0Var;
    CHECK_DV (CMD_NONE,     // cmd
              CMD_NONE,     // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              0,            // cmd req num
              0,            // proc req num
              0,            // test0 val
              test0Var);

    // Set LAUNCH cmd and cmd req number.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_ABORT));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 1));

    // Run handler.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_ABORT,    // cmd
              CMD_ABORT,    // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              1,            // cmd req num
              1,            // proc req num
              0,            // test0 val
              test0Var);

    // Run handler again with no new command.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_NONE,     // cmd
              CMD_ABORT,    // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              1,            // cmd req num
              1,            // proc req num
              0,            // test0 val
              test0Var);
}

/**
 * Helper function to test writing each DV elem type.
 */
template <class T>
void testWriteSuccess ()
{
    // Set up DV and CH.
    DataVector::Config_t dvConfig = gDvConfig;
    DataVectorElementType_t type = DV_T_LAST;
    if (std::is_same<T, uint8_t>::value) 
    {
        type = DV_T_UINT8;
    }
    else if (std::is_same<T, uint16_t>::value) 
    {
        type = DV_T_UINT16;
    }
    else if (std::is_same<T, uint32_t>::value) 
    {
        type = DV_T_UINT32;
    }
    else if (std::is_same<T, uint64_t>::value) 
    {
        type = DV_T_UINT64;
    }
    else if (std::is_same<T, int8_t>::value) 
    {
        type = DV_T_INT8;
    }
    else if (std::is_same<T, int16_t>::value) 
    {
        type = DV_T_INT16;
    }
    else if (std::is_same<T, int32_t>::value) 
    {
        type = DV_T_INT32;
    }
    else if (std::is_same<T, int64_t>::value) 
    {
        type = DV_T_INT64;
    }
    else if (std::is_same<T, float>::value) 
    {
        type = DV_T_FLOAT;
    }
    else if (std::is_same<T, double>::value) 
    {
        type = DV_T_DOUBLE;
    }
    else if (std::is_same<T, bool>::value) 
    {
        type = DV_T_BOOL;
    }
    dvConfig[0].elems[6].type = type;
    INIT_DATA_VECTOR (dvConfig);
    std::unique_ptr<CommandHandler> pCh = nullptr;
    CHECK_SUCCESS (CommandHandler::createNew (gChConfig, pDv, pCh));

    // Set WRITE cmd, cmd req number, write elem, and write val.
    uint64_t max = DataVector::toUInt64 (std::numeric_limits<T>::max ());
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_WRITE));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 1));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_WRITE_ELEM, 
                               (uint32_t) DV_ELEM_TEST0));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_WRITE_VAL, 
                               DataVector::toUInt64 (max)));

    // Run handler.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state
    T test0Var;
    CHECK_DV (CMD_WRITE,     // cmd
              CMD_WRITE,     // cmd req
              DV_ELEM_TEST0, // write elem
              max,           // write val
              1,             // cmd req num
              1,             // proc req num
              (T) max,       // test0 val
              test0Var);

    // Run handler again with no new command.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_NONE,      // cmd
              CMD_WRITE,     // cmd req
              DV_ELEM_TEST0, // write elem
              max,           // write val
              1,             // cmd req num
              1,             // proc req num
              (T) max,       // test0 val
              test0Var);

}

/* Test successful run of write to each elem type. */
TEST (CommandHandler_Run, WriteSuccess)
{
    testWriteSuccess<uint8_t>  ();
    testWriteSuccess<uint16_t> ();
    testWriteSuccess<uint32_t> ();
    testWriteSuccess<uint64_t> ();
    testWriteSuccess<int8_t>   ();
    testWriteSuccess<int16_t>  ();
    testWriteSuccess<int32_t>  ();
    testWriteSuccess<int64_t>  ();
    testWriteSuccess<float>    ();
    testWriteSuccess<double>   ();
    testWriteSuccess<bool>     ();
}

/* Test successful sequential runs of each command. */
TEST (CommandHandler_Run, LaunchAbortWriteSuccess)
{
    INIT_CH_SUCCESS;

    // Verify initial state.
    uint8_t test0Var;
    CHECK_DV (CMD_NONE,     // cmd
              CMD_NONE,     // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              0,            // cmd req num
              0,            // proc req num
              0,            // test0 val
              test0Var);

    // Set LAUNCH cmd and cmd req number.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_LAUNCH));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 1));

    // Run handler.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_LAUNCH,   // cmd
              CMD_LAUNCH,   // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              1,            // cmd req num
              1,            // proc req num
              0,            // test0 val
              test0Var);

    // Set ABORT cmd and cmd req number.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_ABORT));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 2));

    // Run handler again.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_ABORT,    // cmd
              CMD_ABORT,    // cmd req
              DV_ELEM_LAST, // write elem
              0,            // write val
              2,            // cmd req num
              2,            // proc req num
              0,            // test0 val
              test0Var);

    // Set WRITE cmd, cmd req number, write elem, and write val.
    uint64_t max = DataVector::toUInt64 (std::numeric_limits<uint8_t>::max ());
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ, (uint8_t) CMD_WRITE));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_REQ_NUM, (uint32_t) 3));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_WRITE_ELEM, 
                               (uint32_t) DV_ELEM_TEST0));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CMD_WRITE_VAL, 
                               DataVector::toUInt64 (max)));

    // Run handler again.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state
    CHECK_DV (CMD_WRITE,     // cmd
              CMD_WRITE,     // cmd req
              DV_ELEM_TEST0, // write elem
              max,           // write val
              3,             // cmd req num
              3,             // proc req num
              (uint8_t) max, // test0 val
              test0Var);

    // Run handler again with no new command.
    CHECK_SUCCESS (pCh->run ());
   
    // Verify new state.
    CHECK_DV (CMD_NONE,      // cmd
              CMD_WRITE,     // cmd req
              DV_ELEM_TEST0, // write elem
              max,           // write val
              3,             // cmd req num
              3,             // proc req num
              (uint8_t) max, // test0 val
              test0Var);
}
