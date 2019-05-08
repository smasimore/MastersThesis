#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

StateMachine StateMachine::fromDefault () {
	// Arbitrary default declaration
	return StateMachine(1, 2);
}

StateMachine StateMachine::fromArr (int32_t c[]) {
	// Arbitrary calculations to obtain A, B data from array
	int a = c[0];
	int b = 0;
	for (int32_t i = 0; i < 4; i++) {
		b += c[i];
	}
	return StateMachine (a, b);
}

Error_t StateMachine::printData () {
	printf("A: %" PRId32 "B: %" PRId32 "", StateMachine::a, StateMachine::b);
	return E_SUCCESS;
}

Error_t StateMachine::getA (int32_t &result) {
	result = StateMachine::a;
	return E_SUCCESS;
}

Error_t StateMachine::getB (int32_t &result) {
	result = b;
	return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine (int32_t a, int32_t b) {
	StateMachine::a = a;
	StateMachine::b = b;
}

