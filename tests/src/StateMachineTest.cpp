#include "StateMachine.hpp"
#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"

#include "CppUTest/TestHarness.h"

/*****TEMPORARY MACRO*****/
#define INIT_SM                             \
    Error_t ret;                            \
    StateMachine *pSM = nullptr;            \
    ret = StateMachine::fromDefault (&pSM);


TEST_GROUP (StateMachines) 
{
};

/* Test to create a StateMachine from default hardcoded case, 
   then verify StateMachine data */
TEST (StateMachines, DefaultCase) 
{
    StateMachine *pSM = nullptr;
    Error_t ret = StateMachine::fromDefault (&pSM);
    int32_t resultA;
    int32_t resultB;
    Error_t retA = pSM->getA (resultA);
    Error_t retB = pSM->getB (resultB);

    // Default Case returns A = 1, B = 2
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (E_SUCCESS == retA);
    CHECK_TRUE (E_SUCCESS == retB);
    CHECK_EQUAL (1, resultA);
    CHECK_EQUAL (2, resultB);
}

/* Test to create a StateMachine from a defined case using data from an array, 
   then verify internal calculations from data */
TEST (StateMachines, DefinedCase)
{
    int32_t data_example[] = { 1, 1, 1, 1 };
    StateMachine *pSM = nullptr;
    Error_t ret = StateMachine::fromArr (&pSM, data_example);
    int32_t resultA;
    int32_t resultB;
    Error_t retA = pSM->getA (resultA);
    Error_t retB = pSM->getB (resultB);

    // Defined Case for Array returns A = arr[0], B = sum of arr[0] to arr[3]
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (E_SUCCESS == retA);
    CHECK_TRUE (E_SUCCESS == retB);
    CHECK_EQUAL (1, resultA);
    CHECK_EQUAL (4, resultB);

    // Delete the map to resolve memory leaks
    pSM->deleteMap();
}

/* Test to create a StateMachine as before, then run State Mapping code*/
TEST (StateMachines, AddStates)
{
    StateMachine *pSM = nullptr;
    Error_t ret = StateMachine::fromDefault (&pSM);

    // Check state machine initialization
    CHECK_TRUE (E_SUCCESS == ret);

    // Create states using secondary iteration of temporary constructor
    std::vector<std::string> tempA = { "A", "B", "C" };
    std::vector<std::string> tempB = { "B", "C", "D" };
    std::vector<std::string> tempC = { "C", "D", "E" };
    State stateA ("StateA", tempA);
    State stateB ("StateB", tempB);
    State stateC ("StateC", tempC);

    // Add states to StateMachine
    Error_t retA = pSM->addState (stateA);
    Error_t retB = pSM->addState (stateB);
    Error_t retC = pSM->addState (stateC);

    CHECK_TRUE (retA == E_SUCCESS);
    CHECK_TRUE (retB == E_SUCCESS);
    CHECK_TRUE (retC == E_SUCCESS);

    // Attempt to add a State with duplicate name
    State stateD ("StateA", tempA);
    Error_t retD = pSM->addState (stateD);

    CHECK_TRUE (retD == E_DUPLICATE_NAME);

    // Attempt to call function to find states
    State stateResult ("", {});
    Error_t retFind = pSM->findState(stateResult, "StateA");
    CHECK_TRUE (retFind == E_SUCCESS);

    // Access data of found state
    std::vector<std::string> dataResult;
    Error_t retData = stateResult.getTransitions(dataResult);
    CHECK_TRUE (retData == E_SUCCESS);
    CHECK_TRUE (tempA == dataResult);

    // Attempt to find an invalid state
    retFind = pSM->findState(stateResult, "StateD");
    CHECK_TRUE (retFind == E_NAME_NOTFOUND);

    // Need to manually clear the states at end to avoid a memory leak.
    pSM->deleteMap ();
}

/* Test to create a State Machine from existing vector of states. 
   This creates the StateMachine immediately with the necessary states
   instead of having to add states after the object is constructed. */
TEST (StateMachines, DefinedStateCase)
{
    // Create State Objects
    std::vector<std::string> tempA = { "A", "B", "C" };
    std::vector<std::string> tempB = { "B", "C", "D" };
    std::vector<std::string> tempC = { "C", "D", "E" };
    State stateA ("StateA", tempA);
    State stateB ("StateB", tempB);
    State stateC ("StateC", tempC);

    // Create storage vector for constructor
    std::vector<State> storageVec = { stateA, stateB, stateC };

    // Create State Machine from vector of States
    StateMachine *pSM = nullptr;
    Error_t ret = StateMachine::fromStates (&pSM, storageVec);

    CHECK_TRUE (ret == E_SUCCESS);

    // Attempt to call function to find states
    State stateResult ("", {});
    Error_t retFind = pSM->findState(stateResult, "StateA");
    CHECK_TRUE (retFind == E_SUCCESS);

    // Access data of found state
    std::vector<std::string> dataResult;
    Error_t retData = stateResult.getTransitions(dataResult);
    CHECK_TRUE (retData == E_SUCCESS);
    CHECK_TRUE (tempA == dataResult);

    // Attempt to find an invalid state
    retFind = pSM->findState(stateResult, "StateD");
    CHECK_TRUE (retFind == E_NAME_NOTFOUND);

    // Still need to manually clear states despite using this method.
    pSM->deleteMap ();
}

/* Test to access temporary state data*/
