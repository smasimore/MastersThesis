#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::fromDefault ()
{
	// Create default case and store in param
	return StateMachine(1, 2);
}

Error_t StateMachine::fromArr (StateMachine **ppStateMachine, int32_t c[])
{
StateMachine StateMachine::fromDefault () {
	// Arbitrary default declaration
	return StateMachine(1, 2);
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

<<<<<<< HEAD
StateMachine::StateMachine (int32_t a, int32_t b)
{
=======
StateMachine::StateMachine (int32_t a, int32_t b) {
>>>>>>> c10e065b5bdd0951b3eca9c9c8437a339e7491b1
	StateMachine::a = a;
	StateMachine::b = b;
}

