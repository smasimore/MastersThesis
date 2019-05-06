#include <cstdio>
#include <vector>
#include "Errors.h"

#ifndef State_HPP
#define State_HPP
class State {
public:
	State();
	State(std::vector<int> int_data);
	Error_t printData();
	Error_t getData(std::vector<int> &result);
private:
	std::vector<int> StateData;
};
#endif

