#include "State.hpp"
#include "Errors.h"

State::State() {
	StateData = { 1, 2, 3, 4 };
};
State::State(std::vector<int> int_data) {
	StateData = int_data;
}
Error_t State::printData() {
	for (int i:StateData) {
		printf("data@%d: %d\n", i, StateData[i]);
	}
	return E_SUCCESS;
}
Error_t State::getData(std::vector<int> &result) {
	result = StateData;
	return E_SUCCESS;
}