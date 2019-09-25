#include <unistd.h>

#include "StateMachine.hpp"
#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"

#include "CppUTest/TestHarness.h"

/************************** TESTER FUNCTIONS **********************************/

// global variable for use with tester functions
volatile int32_t gVar1;

Error_t multiplyParam1 (int32_t param)
{
    gVar1 = gVar1 * param;
    return E_SUCCESS;
}

Error_t addParam1 (int32_t param)
{
    gVar1 = gVar1 + param;
    return E_SUCCESS;
}

Error_t subtractParam1 (int32_t param)
{
    gVar1 = gVar1 - param;
    return E_SUCCESS;
}

Error_t fail (int32_t param)
{
    return E_TEST_ERROR;
}

// Implement periodic with thread manager
volatile bool gThreadStopped;

Error_t stateThreadFunc (std::shared_ptr<StateMachine> pSM)
{
    Error_t ret;
    while (!gThreadStopped)
    {
        ret = pSM->periodic ();
        if (ret != E_SUCCESS)
        {
            gThreadStopped = true;
            return ret;
        }
    }
    return E_SUCCESS;
}

/******************************** TESTS ***************************************/

TEST_GROUP (StateMachines)
{
};

/* Test to create a State Machine from existing vector of states.
   This creates the StateMachine immediately with the necessary states
   instead of having to add states after the object is constructed. */
TEST (StateMachines, DefinedStateCase)
{
    // Create basic loop transitions for the states
    std::vector<std::string> transitionsA = { "StateB" };
    std::vector<std::string> transitionsB = { "StateC" };
    std::vector<std::string> transitionsC = { "StateA" };

    // Create vector of states for createNew function
    std::vector<StateMachine::State_t> stateVec 
        = { {"StateA", transitionsA, {}},
            {"StateB", transitionsB, {}},
            {"StateC", transitionsC, {}} };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::createNew (pSM, stateVec);

    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Attempt to call function to find states
    std::shared_ptr<State> stateResult (nullptr);
    ret = pSM->findState (stateResult, "StateA");
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (stateResult != nullptr);

    // Access data of found state
    std::vector<std::string> *pDataResult;
    ret = stateResult->getTransitions (&pDataResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsA == *pDataResult);

    // Attempt to find an invalid state
    ret = pSM->findState (stateResult, "StateD");
    CHECK_TRUE (E_NAME_NOTFOUND == ret);

    // Get the current State info (StateA, since first in vector)
    std::string nameResult;
    ret = pSM->getCurrentStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateA");

    std::vector<std::string> transitionsResult;
    ret = pSM->getCurrentStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == transitionsA);

    // Force a valid transition from StateA to StateB
    ret = pSM->switchState ("StateB");
    CHECK_TRUE (E_SUCCESS == ret);

    // Check if current State is StateB
    ret = pSM->getCurrentStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateB");

    ret = pSM->getCurrentStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == transitionsB);

    // Attempt to force an invalid transition from StateB to StateA
    ret = pSM->switchState ("StateA");
    CHECK_TRUE (E_INVALID_TRANSITION == ret);

    // Check if current State is still StateB
    ret = pSM->getCurrentStateName (nameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (nameResult == "StateB");

    ret = pSM->getCurrentStateTransitions (transitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitionsResult == transitionsB);
}

/* Test to manage States with action sequences within the StateMachine */
TEST (StateMachines, ManageActionSequence)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam1;
    Error_t (*pFuncA) (int32_t) = addParam1;
    Error_t (*pFuncS) (int32_t) = subtractParam1;

    // create actions from timestamp, function, and params
    State::Action_t actionM { 1, pFuncM, 3 };
    State::Action_t actionA { 2, pFuncA, 5 };
    State::Action_t actionS { 3, pFuncS, 3 };

    // create corresponding vectors of actions
    std::vector<State::Action_t> actionsA = { actionM };
    std::vector<State::Action_t> actionsB = { actionA };
    std::vector<State::Action_t> actionsC = { actionS };

    // Create basic loop transitions for the states
    std::vector<std::string> transitionsA = { "StateB" };
    std::vector<std::string> transitionsB = { "StateC" };
    std::vector<std::string> transitionsC = { "StateA" };

    // Create storage vector for constructor
    std::vector<StateMachine::State_t> stateVec
        = { {"StateA", transitionsA, actionsA},
            {"StateB", transitionsB, actionsB},
            {"StateC", transitionsC, actionsC} };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::createNew (pSM, stateVec);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Create local map and search iterator
    std::map<int32_t, std::vector<State::Action_t>>
        localMap;
    std::map<int32_t, std::vector<State::Action_t>>
        ::const_iterator search;

    // First state is StateA, retrieve its action sequence
    ret = pSM->getCurrentActionSequence (localMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // At timestamp 1, action sequence contains multiply function and param 3
    search = localMap.find (1);
    CHECK_EQUAL (search->first, 1);
    CHECK_EQUAL (search->second[0].func, pFuncM);
    CHECK_EQUAL (search->second[0].param, 3);

    // Transition to StateB, then retrieve its action sequence
    ret = pSM->switchState ("StateB");
    CHECK_TRUE (E_SUCCESS == ret);
    ret = pSM->getCurrentActionSequence (localMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // At timestamp 2, action sequence contains addition function and param 5
    search = localMap.find (2);
    CHECK_EQUAL (search->first, 2);
    CHECK_EQUAL (search->second[0].func, pFuncA);
    CHECK_EQUAL (search->second[0].param, 5);
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
    State::Action_t actionM { 0, pFuncM, 3 };
    State::Action_t actionA { 0, pFuncA, 5 };
    State::Action_t actionS { 0, pFuncS, 3 };
    State::Action_t actionF { 1, pFuncF, 3 };


    // create corresponding input vector of tuples
    std::vector<State::Action_t> vecInA = { actionM, actionA };
    std::vector<State::Action_t> vecInB = { actionA, actionS };
    std::vector<State::Action_t> vecInC = { actionM, actionF };

    // Create basic loop transitions for the states
    std::vector<std::string> transitionsA = { "StateB" };
    std::vector<std::string> transitionsB = { "StateC" };
    std::vector<std::string> transitionsC = { "StateA" };

    // Create storage vector for constructor
    std::vector<StateMachine::State_t> stateVec
        = { { "StateA", transitionsA, vecInA },
            { "StateB", transitionsB, vecInB },
            { "StateC", transitionsC, vecInC } };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::createNew (pSM, stateVec);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Set global variable for testing
    gVar1 = 3;

    // First state is StateA; action sequence multiplies by 3 then adds 5
    ret = pSM->executeCurrentSequence ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 14);

    // Switch to StateB; action sequence adds 5 then subtracts 3
    ret = pSM->switchState ("StateB");
    CHECK_TRUE (E_SUCCESS == ret);
    ret = pSM->executeCurrentSequence ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 16);

    // Switch to StateC; action sequence multiplies by 3 then fails
    ret = pSM->switchState ("StateC");
    CHECK_TRUE (E_SUCCESS == ret);
    ret = pSM->executeCurrentSequence ();
    CHECK_TRUE (E_TEST_ERROR == ret);
    CHECK_EQUAL (gVar1, 48);
}

/* Test the periodic function with a basic placeholder time variable */
TEST (StateMachines, ExecuteActionsPeriodic)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam1;
    Error_t (*pFuncA) (int32_t) = addParam1;
    Error_t (*pFuncS) (int32_t) = subtractParam1;
    Error_t (*pFuncF) (int32_t) = fail;

    // create tuples of ascending timestamps, function pointer, and param
    State::Action_t actionM { 2, pFuncM, 3 };
    State::Action_t actionA { 4, pFuncA, 5 };
    State::Action_t actionS { 6, pFuncS, 3 };
    State::Action_t actionF { 8, pFuncF, 3 };

    // create a corresponding input vector of tuples
    std::vector<State::Action_t> actionsA = 
        { actionM, actionA, actionS, actionF };

    // Create basic loop transitions for the states
    std::vector<std::string> transitionsA = { "StateB" };
    std::vector<std::string> transitionsB = { "StateA" };

    // Create storage vector for constructor, using the same action sequence
    std::vector<StateMachine::State_t> stateVec
        = { {"StateA", transitionsA, actionsA},
            {"StateB", transitionsB, actionsA} };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::createNew (pSM, stateVec);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Set global variable for testing
    gVar1 = 3;

    // Set time variable to 0
    pSM->timeElapsed = 0;
    // Call periodic function; global variable should be unchanged
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 3);

    // Set time to 2
    pSM->timeElapsed = 2;
    // Call periodic function; global variable should multiply by 3
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 9);

    // Set time to 4
    pSM->timeElapsed = 4;
    // Call periodic function; global variable should add by 5
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 14);

    // Set time to 6
    pSM->timeElapsed = 6;
    // Call periodic function; global variable should subtract by 3
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 11);

    // Set time to 8
    pSM->timeElapsed = 8;
    // Call periodic function; function should fail
    ret = pSM->periodic ();
    CHECK_TRUE (E_TEST_ERROR == ret);
    CHECK_EQUAL (gVar1, 11);

    // Action sequence ends after 8s; call periodic to check behavior
    // Action iterator should still be pointing to the failing function
    pSM->timeElapsed = 9;
    ret = pSM->periodic ();
    CHECK_TRUE (E_TEST_ERROR == ret);
    CHECK_EQUAL (gVar1, 11);

    // Switch to StateB; StateB action sequence is the same as StateA
    pSM->switchState ("StateB");

    // Reset global variable and time
    gVar1 = 3;
    pSM->timeElapsed = 0;

    // Call periodic function; global variable should be unchanged
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 3);

    // Set time to 2
    pSM->timeElapsed = 2;
    // Call periodic function; global variable should multiply by 3
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 9);

    // Set time to 4
    pSM->timeElapsed = 4;
    // Call periodic function; global variable should add by 5
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 14);

    // Set time to 6
    pSM->timeElapsed = 6;
    // Call periodic function; global variable should subtract by 3
    ret = pSM->periodic ();
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (gVar1, 11);

    // Set time to 8
    pSM->timeElapsed = 8;
    // Call periodic function; function should fail
    ret = pSM->periodic ();
    CHECK_TRUE (E_TEST_ERROR == ret);
    CHECK_EQUAL (gVar1, 11);

    // Action sequence ends after 8s; call periodic to check behavior
    // Action iterator should still be pointing to failing function
    pSM->timeElapsed = 9;
    ret = pSM->periodic ();
    CHECK_TRUE (E_TEST_ERROR == ret);
    CHECK_EQUAL (gVar1, 11);
}

/* Test a periodic thread with the action sequence */
TEST (StateMachines, ExecutePeriodicThread)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam1;
    Error_t (*pFuncA) (int32_t) = addParam1;
    Error_t (*pFuncS) (int32_t) = subtractParam1;
    Error_t (*pFuncF) (int32_t) = fail;

    // create tuples of ascending timestamps, function pointer, and param
    State::Action_t actionM {2, pFuncM, 3};
    State::Action_t actionA {4, pFuncA, 5};
    State::Action_t actionS {6, pFuncS, 3};
    State::Action_t actionF {8, pFuncF, 3};


    // create a corresponding input vector of tuples
    std::vector<State::Action_t> vecInA =
    { actionM, actionA, actionS, actionF };

    // Create basic loop transitions for the states
    std::vector<std::string> transitionsA = { "StateB" };
    std::vector<std::string> transitionsB = { "StateA" };

    // Create storage vector for constructor, using the same action sequence
    std::vector<StateMachine::State_t> stateVec
        = { {"StateA", transitionsA, vecInA},
            {"StateB", transitionsB, vecInA} };

    // Create State Machine from vector of States
    std::unique_ptr<StateMachine> pSM (nullptr);
    Error_t ret = StateMachine::createNew (pSM, stateVec);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pSM != nullptr);

    // Initialize ThreadManager
    ThreadManager *pThreadManager = nullptr;
    ret = ThreadManager::getInstance (&pThreadManager);
    CHECK_TRUE (E_SUCCESS == ret);

    pthread_t stateThread;
    gThreadStopped = false;

    ThreadManager::ThreadFunc_t *pStateThreadFunc =
        (ThreadManager::ThreadFunc_t *) &stateThreadFunc;

    // Create the the threads
    ret = pThreadManager->createThread (stateThread, pStateThreadFunc,
                                        &pSM, sizeof (pSM),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::ALL);
    CHECK_TRUE (E_SUCCESS == ret);

    // Set global variable for testing
    gVar1 = 3;

    // Set time variable to 0, global variable should be unchanged
    pSM->timeElapsed = 0;
    usleep (10);
    CHECK_EQUAL (gVar1, 3);

    // Set time to 2, global variable should multiply by 3
    pSM->timeElapsed = 2;
    usleep (10);
    CHECK_EQUAL (gVar1, 9);

    // Set time to 4, global variable should add by 5
    pSM->timeElapsed = 4;
    usleep (10);
    CHECK_EQUAL (gVar1, 14);

    // Set time to 6, global variable should subtract by 3
    pSM->timeElapsed = 6;
    usleep (10);
    CHECK_EQUAL (gVar1, 11);

    // Set time to 8, function should fail and thread should stop
    pSM->timeElapsed = 8;
    usleep (10);
    CHECK_EQUAL (gVar1, 11);

    // Sleep to allow state thread to run
    usleep (10);

    // Check that the thread finished
    CHECK_TRUE (gThreadStopped);
    Error_t threadRet;
    ret = pThreadManager->waitForThread (stateThread, threadRet);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (E_TEST_ERROR == threadRet);
}