#include "StateMachine.hpp"

StateMachine StateMachine::from_default() {
	// Arbitrary default declaration
	return StateMachine(1, 2);
}
StateMachine StateMachine::from_arr(int c[]) {
	// Arbitrary calculations to obtain A, B data from array
	int a = c[0];
	int b = 0;
	for (int i = 0; i < 4; i++) {
		b += c[i];
	}
	return StateMachine(a, b);
}
Error_t StateMachine::printData() {
	printf("A: %d, B: %d", p_a, p_b);
	return E_SUCCESS;
}
Error_t StateMachine::getA(int &result) {
	result = p_a;
	return E_SUCCESS;
}
Error_t StateMachine::getB(int &result) {
	result = p_b;
	return E_SUCCESS;
}
StateMachine::StateMachine(int a, int b) {
	p_a = a;
	p_b = b;
}

