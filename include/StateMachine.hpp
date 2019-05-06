#ifndef StateMachine_HPP
#define StateMachine_HPP

#include <cstdio>
#include "Errors.h"

struct StateMachine {
public:
	// StateMachineTest should use "named constructor idioms" rather than public overloaded constructors
	// Each below static function will return a StateMachineTest object from certain parameters.

	// There should be a hardcoded default case alongside any parser configurations.
	static StateMachine from_default();
	// Example of creating an object from data
	static StateMachine from_arr(int c[]);
	// Print the data for user verification, for now
	Error_t printData();
	// Attempt to access data for testing purposes
	Error_t getA(int &result);
	Error_t getB(int &result);
private:
	// Constructor can be kept private to avoid confusion with "named constructor idioms"
	StateMachine(int a, int b);
	// Skeleton data for testing purposes
	int p_a;
	int p_b;
};
#endif
