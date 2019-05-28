#include "State.hpp"
#include "Errors.h"
#include <unordered_map>

#include "CppUTest/TestHarness.h"

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


/* Test if states can be mapped by name; code will live under StateMachine eventually */
TEST (States, Map_States)
{
    std::unordered_map<std::string, State> stateMap;
    std::vector<int> dataA = { 1, 2, 3, 4 };
    std::vector<int> dataB = { 2, 3, 4, 5 };
    std::vector<int> dataC = { 3, 4, 5, 6 };
    stateMap["stateA"] = State (dataA);
    stateMap["stateB"] = State (dataB);
    stateMap["stateC"] = State (dataC);

    auto search = stateMap.find ("stateA");
    CHECK_EQUAL (search->first, "stateA");
    std::vector<int> dataResult;
    Error_t ret = search->second.getData (dataResult);
    CHECK_TRUE (dataResult == dataA);
    CHECK_TRUE (ret == E_SUCCESS);

    search = stateMap.find ("stateB");
    CHECK_EQUAL (search->first, "stateB");
    ret = search->second.getData (dataResult);
    CHECK_TRUE (dataResult == dataB);
    CHECK_TRUE (ret == E_SUCCESS);

    search = stateMap.find ("stateC");
    CHECK_EQUAL (search->first, "stateC");
    ret = search->second.getData (dataResult);
    CHECK_TRUE (dataResult == dataC);
    CHECK_TRUE (ret == E_SUCCESS);
}