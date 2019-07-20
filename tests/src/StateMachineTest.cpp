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
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::fromDefault (pSM);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

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
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::fromArr (pSM, data_example);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Defined Case for Array returns A = arr[0], B = sum of arr[0] to arr[3]
    int32_t resultA;
    ret = pSM->getA (resultA);
    CHECK_TRUE (E_SUCCESS == ret);

    int32_t resultB;
    ret = pSM->getB (resultB);
    CHECK_TRUE (E_SUCCESS == ret);

    CHECK_EQUAL (1, resultA);
    CHECK_EQUAL (4, resultB);
}

/* Test to create a StateMachine as before, then run State Mapping code*/
TEST (StateMachines, AddStates)
{
    //StateMachine *pSM = nullptr;
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::fromDefault (pSM);

    // Check state machine initialization
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Create states using secondary iteration of temporary constructor
    std::vector<std::string> tempA = { "A", "B", "C" };
    std::vector<std::string> tempB = { "B", "C", "D" };
    std::vector<std::string> tempC = { "C", "D", "E" };

    // Add states to StateMachine
    ret = pSM->addState ("StateA", tempA);
    CHECK_TRUE (E_SUCCESS == ret);

    ret = pSM->addState ("StateB", tempB);
    CHECK_TRUE (E_SUCCESS == ret);

    ret = pSM->addState ("StateC", tempC);
    CHECK_TRUE (E_SUCCESS == ret);

    // Attempt to add a State with duplicate name
    ret = pSM->addState ("StateA", tempA);
    CHECK_TRUE (E_DUPLICATE_NAME == ret);

    // Attempt to call function to find states
    std::shared_ptr<State> stateResult(nullptr);
    ret = pSM->findState(stateResult, "StateA");
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (stateResult != nullptr);

    // Access data of found state
    std::vector<std::string> dataResult;
    ret = stateResult->getTransitions(dataResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (tempA == dataResult);

    // Attempt to find an invalid state
    ret = pSM->findState(stateResult, "StateD");
    CHECK_TRUE (E_NAME_NOTFOUND == ret);
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

    // Create storage vector for constructor
    std::vector<std::tuple<std::string, std::vector<std::string>>> storageVec
        = { std::make_tuple("StateA", tempA),
            std::make_tuple("StateB", tempB),
            std::make_tuple("StateC", tempC) };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::fromStates (pSM, storageVec);

    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Attempt to call function to find states
    std::shared_ptr<State> stateResult (nullptr);
    ret = pSM->findState(stateResult, "StateA");
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (stateResult != nullptr);

    // Access data of found state
    std::vector<std::string> dataResult;
    ret = stateResult->getTransitions(dataResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (tempA == dataResult);

    // Attempt to find an invalid state
    ret = pSM->findState(stateResult, "StateD");
    CHECK_TRUE (E_NAME_NOTFOUND == ret);

    // Get the current State info (StateA, since first in vector)
    std::string nameResult;
    ret = pSM->getCurrentStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateA");

    std::vector<std::string> transitionsResult;
    ret = pSM->getCurrentStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == tempA);

    // Force a valid transition from StateA to StateB
    ret = pSM->switchState ("StateB");
    CHECK_TRUE (E_SUCCESS == ret);

    // Check if current State is StateB
    ret = pSM->getCurrentStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateB");

     ret = pSM->getCurrentStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == tempB);

    // Attempt to force an invalid transition from StateB to StateA
    ret = pSM->switchState ("StateA");
    CHECK_TRUE (E_INVALID_TRANSITION == ret);

    // Check if current State is still StateB
    ret = pSM->getCurrentStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateB");

     ret = pSM->getCurrentStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == tempB);
}
