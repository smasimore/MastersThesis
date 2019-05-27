#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::fromDefault ()
{
	// Create default case and store in param
	return StateMachine(1, 2);
}

Error_t StateMachine::fromArr (StateMachine **ppStateMachine, int32_t c[])
{
	// Arbitrary calculations to obtain A, B data from array
	int a = c[0];
	int b = 0;
	for (int32_t i = 0; i < 4; i++) {
		b += c[i];
	}
    // Create case from array and store in param
	return StateMachine (a, b);
}

Error_t StateMachine::printData ()
{
	printf("A: %" PRId32 "B: %" PRId32 "", StateMachine::a, StateMachine::b);
	return E_SUCCESS;
}

Error_t StateMachine::getA (int32_t &result)
{
	result = StateMachine::a;
	return E_SUCCESS;
}

Error_t StateMachine::getB (int32_t &result)
{
	result = b;
	return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine (int32_t a, int32_t b)
{
	StateMachine::a = a;
	StateMachine::b = b;
}

