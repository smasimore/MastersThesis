#include "CppUTest/TestHarness.h"

#include "State.hpp"
#include "Errors.h"

TEST_GROUP(States) {
};

TEST(States, Access_Data) {
	std::vector<int> data = { 1, 2, 3, 4 };
	State S = State(data);
	std::vector<int> stateData;
	Error_t ret = S.getData(stateData);

	CHECK_TRUE(E_SUCCESS == ret);
	for (unsigned i = 0; i < data.size(); i++) {
		CHECK_EQUAL(data.at(i), stateData.at(i));
	}
}