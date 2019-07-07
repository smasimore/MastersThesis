#include "StateMachine.hpp"
#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"

#include "CppUTest/TestHarness.h"

TEST_GROUP (StateMachines) 
{
};

/* Test to create a StateMachine from default hardcoded case, 
   then verify StateMachine data */
TEST (StateMachines, DefaultCase) 
{
    StateMachine *pSM = nullptr;
    Error_t ret = StateMachine::fromDefault (&pSM);
    CHECK_TRUE (E_SUCCESS == ret);

    // Default Case returns A = 1, B = 2
    int32_t resultA;
    ret = pSM->getA (resultA);
    CHECK_TRUE (E_SUCCESS == ret);

    int32_t resultB;
    ret = pSM->getB (resultB);
    CHECK_TRUE (E_SUCCESS == ret);

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
    CHECK_TRUE (E_SUCCESS == ret);

    // Defined Case for Array returns A = arr[0], B = sum of arr[0] to arr[3]
    int32_t resultA;
    ret = pSM->getA (resultA);
    CHECK_TRUE (E_SUCCESS == ret);

    int32_t resultB;
    ret = pSM->getB (resultB);
    CHECK_TRUE (E_SUCCESS == ret);

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
    ret = pSM->addState (stateA);
    CHECK_TRUE (E_SUCCESS == ret);

    ret = pSM->addState (stateB);
    CHECK_TRUE (E_SUCCESS == ret);

    ret = pSM->addState (stateC);
    CHECK_TRUE (E_SUCCESS == ret);

    // Attempt to add a State with duplicate name
    State stateD ("StateA", tempA);
    ret = pSM->addState (stateD);
    CHECK_TRUE (E_DUPLICATE_NAME == ret);

    // Attempt to call function to find states
    State stateResult ("", {});
    ret = pSM->findState(stateResult, "StateA");
    CHECK_TRUE (E_SUCCESS == ret);

    // Access data of found state
    std::vector<std::string> dataResult;
    ret = stateResult.getTransitions(dataResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (tempA == dataResult);

    // Attempt to find an invalid state
    ret = pSM->findState(stateResult, "StateD");
    CHECK_TRUE (E_NAME_NOTFOUND == ret);

    // Need to manually clear the states at end to avoid a memory leak.
    pSM->deleteMap ();
    pSM->deleteState ();
}

/* Test to create a State Machine from existing vector of states. 
   This creates the StateMachine immediately with the necessary states
   instead of having to add states after the object is constructed. */
TEST (StateMachines, DefinedStateCase)
{
    // Create State Objects with basic, loop transitions
    std::vector<std::string> tempA = { "StateB" };
    std::vector<std::string> tempB = { "StateC" };
    std::vector<std::string> tempC = { "StateA" };
    State stateA ("StateA", tempA);
    State stateB ("StateB", tempB);
    State stateC ("StateC", tempC);

    // Create storage vector for constructor
    std::vector<State> storageVec = { stateA, stateB, stateC };

    // Create State Machine from vector of States
    StateMachine *pSM = nullptr;
    Error_t ret = StateMachine::fromStates (&pSM, storageVec);

    CHECK_TRUE (E_SUCCESS == ret);

    // Attempt to call function to find states
    State stateResult ("", {});
    ret = pSM->findState(stateResult, "StateA");
    CHECK_TRUE (E_SUCCESS == ret);

    // Access data of found state
    std::vector<std::string> dataResult;
    ret = stateResult.getTransitions(dataResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (tempA == dataResult);

    // Attempt to find an invalid state
    ret = pSM->findState(stateResult, "StateD");
    CHECK_TRUE (E_NAME_NOTFOUND == ret);

    // Get the current State info (StateA, since first in vector)
    std::string nameResult;
    ret = pSM->getStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateA");

    std::vector<std::string> transitionsResult;
    ret = pSM->getStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == tempA);

    // Force a valid transition from StateA to StateB
    ret = pSM->switchState ("StateB");
    CHECK_TRUE (E_SUCCESS == ret);

    // Check if current State is StateB
    ret = pSM->getStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateB");

     ret = pSM->getStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == tempB);

    // Attempt to force an invalid transition from StateB to StateA
    ret = pSM->switchState ("StateA");
    CHECK_TRUE (E_INVALID_TRANSITION == ret);

    // Check if current State is still StateB
    ret = pSM->getStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateB");

     ret = pSM->getStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == tempB);

    // Still need to manually clear states despite using this method.
    pSM->deleteMap ();
    pSM->deleteState ();
}


// TODO: Functionality tests do not work when creating a new test group! Might
//  be related to singleton issue and trying to reuse the same constructor
//  within the tests. Commented out the test group for now, so just test for
//  new functionality within above test group, then try to resolve in separate
//  PR to address memory improvements of StateMachine.
//TEST (StateMachines, StateTransitions)
//{
//    // Create State Objects with single "loop" transitions
//    std::vector<std::string> transA = { "StateB" };
//    std::vector<std::string> transB = { "StateC" };
//    std::vector<std::string> transC = { "StateA" };
//    State stateA ("StateA", transA);
//    State stateB ("StateB", transB);
//    State stateC ("StateC", transC);
//
//    // Create storage vector for constructor
//    std::vector<State> storageVec = { stateA, stateB, stateC };
//
//    // Create State Machine from vector of States
//    StateMachine *pSM = nullptr;
//    Error_t ret = StateMachine::fromStates (&pSM, storageVec);
//
//    CHECK_TRUE (ret == E_SUCCESS);
//
//    // Get the current state info (StateA, since first in vector)
//    std::string nameResult;
//    ret = pSM->getStateName (nameResult);
//
//    CHECK_TRUE (ret == E_SUCCESS);
//    CHECK_EQUAL (nameResult, "StateA");
//
//    //std::string nameResult;
//    //ret = pStateCurr->getName (nameResult);
//
//    //CHECK_TRUE (ret == E_SUCCESS);
//    //CHECK_EQUAL (nameResult, "StateA");
//
//    // Delete map to pass memory leak tests
//    pSM->deleteMap ();
//    pSM->deleteState ();
//}
