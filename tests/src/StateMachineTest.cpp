#include "StateMachine.hpp"
#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"

#include "CppUTest/TestHarness.h"

/************************** TESTER FUNCTIONS **********************************/

// global variable for use with tester functions
int32_t varGlobal1;

Error_t multiplyParam1 (int32_t param)
{
    varGlobal1 = varGlobal1 * param;
    return E_SUCCESS;
}

Error_t addParam1 (int32_t param)
{
    varGlobal1 = varGlobal1 + param;
    return E_SUCCESS;
}

Error_t subtractParam1 (int32_t param)
{
    varGlobal1 = varGlobal1 - param;
    return E_SUCCESS;
}

Error_t fail (int32_t param)
{
    return E_INTED;
}

/******************************** TESTS ***************************************/

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

/* Test to manage States with action sequences within the StateMachine */
TEST (StateMachines, ManageActionSequence)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam1;
    Error_t (*pFuncA) (int32_t) = addParam1;
    Error_t (*pFuncS) (int32_t) = subtractParam1;

    // create tuples of timestamp, function pointer, and param
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup1 (1, pFuncM, 3);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup2 (2, pFuncA, 5);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup3 (3, pFuncS, 3);

    // create corresponding input vector of tuples
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecInA =
    { tup1 };
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecInB =
    { tup2 };
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecInC =
    { tup3 };

    // Create State Objects with basic, loop transitions
    std::vector<std::string> tempA = { "StateB" };
    std::vector<std::string> tempB = { "StateC" };
    std::vector<std::string> tempC = { "StateA" };

    // Create storage vector for constructor
    std::vector<std::tuple<std::string, std::vector<std::string>>> storageVec
        = { std::make_tuple ("StateA", tempA, vecInA),
            std::make_tuple ("StateB", tempB, vecInB),
            std::make_tuple ("StateC", tempC, vecInC) };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::fromStates (pSM, storageVec);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Create local map and search iterator
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        localMap;
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        ::const_iterator search;

    // First state is StateA, retrieve its action sequence
    ret = pSM->getCurrentActionSequence (localMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // At timestamp 1, action sequence contains multiply function and param 3
    search = localMap.find (1);
    CHECK_EQUAL (search->first, 1);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncM);
    CHECK_EQUAL (std::get<1> (search->second[0]), 3);

    // Transition to StateB, then retrieve its action sequence
    ret = pSM->switchState ("StateB");
    CHECK_TRUE (E_SUCCESS == ret);
    ret = pSM->getCurrentActionSequence (localMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // At timestamp 2, action sequence contains addition function and param 5
    search = localMap.find (2);
    CHECK_EQUAL (search->first, 2);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncA);
    CHECK_EQUAL (std::get<1> (search->second[0]), 5);

}

/* Test to arbitrarily execute the action sequences in StateMachine */
TEST (StateMachines, ExecuteActionSequence)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam1;
    Error_t (*pFuncA) (int32_t) = addParam1;
    Error_t (*pFuncS) (int32_t) = subtractParam1;
    Error_t (*pFuncF) (int32_t) = fail;

    // create tuples of timestamp, function pointer, and param
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup1 (0, pFuncM, 3);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup2 (0, pFuncA, 5);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup3 (0, pFuncS, 3);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup4 (1, pFuncF, 3);


    // create corresponding input vector of tuples
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecInA =
    { tup1, tup2 };
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecInB =
    { tup2, tup3 };
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecInC =
    { tup1, tup4 };

    // Create State Objects with basic, loop transitions
    std::vector<std::string> tempA = { "StateB" };
    std::vector<std::string> tempB = { "StateC" };
    std::vector<std::string> tempC = { "StateA" };

    // Create storage vector for constructor
    std::vector<std::tuple<std::string, std::vector<std::string>>> storageVec
        = { std::make_tuple ("StateA", tempA, vecInA),
            std::make_tuple ("StateB", tempB, vecInB),
            std::make_tuple ("StateC", tempC, vecInC) };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::fromStates (pSM, storageVec);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Create global variable for testing
    varGlobal1 = 3;

    // First state is StateA; action sequence multiplies by 3 then adds 5
    ret = pSM->executeCurrentSequence ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (varGlobal1, 14);

    // Switch to StateB; action sequence adds 5 then subtracts 3
    ret = pSM->executeCurrentSequence ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (varGlobal1, 16);

    // Switch to StateC; action sequence multiplies by 3 then fails
    ret = pSM->executeCurrentSequence ();
    CHECK_TRUE (false);
    CHECK_EQUAL (varGlobal1, 48);
}
