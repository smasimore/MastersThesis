#include "StateMachine.hpp"

/******************** PUBLIC FUNCTIONS **************************/

Error_t StateMachine::fromDefault (StateMachine **ppStateMachine)
{
	// Create StateMachine case and store in param
    static StateMachine stateMachineInstance = StateMachine (1, 2);
    *ppStateMachine = &stateMachineInstance;
    return E_SUCCESS;
   
}

Error_t StateMachine::fromArr (StateMachine **ppStateMachine, int32_t c[])
{
    // Arbitrary calculations to obtain A, B data from array
    int a = c[0];
    int b = 0;
    for (int32_t i = 0; i < 4; i++)
    {
        b += c[i];
    }
    // Create StateMachine case and store in param
    static StateMachine stateMachineInstance = StateMachine (a, b);
    *ppStateMachine = &stateMachineInstance;
    return E_SUCCESS;
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
	result = StateMachine::b;
	return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

StateMachine::StateMachine (int32_t a, int32_t b)
{
	StateMachine::a = a;
	StateMachine::b = b;
}

