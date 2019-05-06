#include "CppUTest/TestHarness.h"

#include "StateMachine.hpp"
#include "Errors.h"

TEST_GROUP(StateMachines) {
};

TEST(StateMachines, Default_Case) {
	StateMachine SM = StateMachine::from_default();
	int resultA;
	int resultB;
	Error_t retA = SM.getA(resultA);
	Error_t retB = SM.getB(resultB);

	// Default Case returns A = 1, B = 2
	CHECK_TRUE(E_SUCCESS == retA);
	CHECK_TRUE(E_SUCCESS == retB);
	CHECK_EQUAL(1, resultA);
	CHECK_EQUAL(2, resultB);
}

TEST(StateMachines, Defined_Case) {
	int data_example[] = { 1, 1, 1, 1 };
	StateMachine SM = StateMachine::from_arr(data_example);
	int resultA;
	int resultB;
	Error_t retA = SM.getA(resultA);
	Error_t retB = SM.getB(resultB);

	// Defined Case for Array returns A = arr[0], B = sum of arr[0] to arr[3]
	CHECK_TRUE(E_SUCCESS == retA);
	CHECK_TRUE(E_SUCCESS == retB);
	CHECK_EQUAL(1, resultA);
	CHECK_EQUAL(4, resultB);
}

