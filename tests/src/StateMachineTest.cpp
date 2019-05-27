#include "CppUTest/TestHarness.h"

#include "StateMachine.hpp"
#include "Errors.h"

TEST_GROUP (StateMachines) 
{
};

/* Test to create a StateMachine from default hardcoded case, then verify StateMachine data */
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

/* Test to create a StateMachine from a defined case using data from an array, then verify internal calculations from data */
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
}

