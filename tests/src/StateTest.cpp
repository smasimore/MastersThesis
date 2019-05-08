#include "CppUTest/TestHarness.h"

#include "State.hpp"
#include "Errors.h"

TEST_GROUP (States) 
{
};

/* Test to create a State with data, then try to access the State's data */
TEST (States, AccessData)
{
    std::vector<int> data = { 1, 2, 3, 4 };
    State S = State (data);
    std::vector<int> stateData;
    Error_t ret = S.getData (stateData);

    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (data == stateData);
}