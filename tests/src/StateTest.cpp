#include "State.hpp"
#include "Errors.h"
#include <unordered_map>

#include "CppUTest/TestHarness.h"

/************************** TESTER FUNCTIONS **********************************/

// global variable for use with tester functions
int32_t gVar;

Error_t multiplyParam (int32_t param)
{
    gVar = gVar * param;
    return E_SUCCESS;
}

Error_t addParam (int32_t param)
{
    gVar = gVar + param;
    return E_SUCCESS;
}

Error_t subtractParam (int32_t param)
{
    gVar = gVar - param;
    return E_SUCCESS;
}

/******************************** TESTS ***************************************/

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

/* Test if states can be mapped by name; code will live under StateMachine */
TEST (States, MapStates)
{
    std::unordered_map<std::string, State> stateMap;
    std::vector<int> dataA = { 1, 2, 3, 4 };
    std::vector<int> dataB = { 2, 3, 4, 5 };
    std::vector<int> dataC = { 3, 4, 5, 6 };
    stateMap.insert (std::make_pair ("stateA", State (dataA)));
    stateMap.insert (std::make_pair ("stateB", State (dataB)));
    stateMap.insert (std::make_pair ("stateC", State (dataC)));

    std::unordered_map<std::string, State>::iterator search =
       stateMap.find ("stateA");
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

/* Create an action sequence with unique timestamps per function */
TEST (States, UniqueActions)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam;
    Error_t (*pFuncA) (int32_t) = addParam;
    Error_t (*pFuncS) (int32_t) = subtractParam;

    // create tuples of timestamp, function pointer, and param
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup1 (0, pFuncM, 3);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup2 (1, pFuncA, 5);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup3 (2, pFuncS, 3);

    // create corresponding input vector of tuples
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecIn =
        { tup1, tup2, tup3 };

    // construct a state with input vector
    State S = State ("", {} , vecIn);

    // retrieve the Action Sequence
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        localMap;
    Error_t ret = S.getActionSequence (localMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // search the map
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        ::const_iterator search;

    // at key 0, vector of tuple of pointer to function multiply and param 3
    search = localMap.find (0);
    CHECK_EQUAL (search->first, 0);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncM);
    CHECK_EQUAL (std::get<1> (search->second[0]), 3);

    // at key 1, vector of tuple of pointer to function add and param 5
    search = localMap.find (1);
    CHECK_EQUAL (search->first, 1);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncA);
    CHECK_EQUAL (std::get<1> (search->second[0]), 5);

    // at key 2, tuple of pointer to function subtract and param 3
    search = localMap.find (2);
    CHECK_EQUAL (search->first, 2);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncS);
    CHECK_EQUAL (std::get<1> (search->second[0]), 3);
}

/* Create an action sequence with a shared timestamp between functions */
TEST (States, SharedActions)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam;
    Error_t (*pFuncA) (int32_t) = addParam;
    Error_t (*pFuncS) (int32_t) = subtractParam;

    // create tuples of timestamp, function pointer, and param
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup1 (0, pFuncM, 3);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup2 (0, pFuncA, 5);
    std::tuple<int32_t, Error_t (*) (int32_t), int32_t> tup3 (0, pFuncS, 3);

    // create corresponding input vector of tuples
    std::vector<std::tuple<int32_t, Error_t (*) (int32_t), int32_t>> vecIn =
    { tup1, tup2, tup3 };

    // construct a state with input vector
    State S = State ("", {}, vecIn);

    // retrieve the Action Sequence
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        localMap;
    Error_t ret = S.getActionSequence (localMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // search the map
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        ::const_iterator search;

    // at timestamp 0, a vector of three tuples: multiply/3, add/5, subtract/3
    search = localMap.find (0);
    CHECK_EQUAL (search->first, 0);

    // verify first element of vector
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncM);
    CHECK_EQUAL (std::get<1> (search->second[0]), 3);

    // verify second element of vector
    CHECK_EQUAL (std::get<0> (search->second[1]), pFuncA);
    CHECK_EQUAL (std::get<1> (search->second[1]), 5);

    // verify third element of vector
    CHECK_EQUAL (std::get<0> (search->second[2]), pFuncS);
    CHECK_EQUAL (std::get<1> (search->second[2]), 3);
}