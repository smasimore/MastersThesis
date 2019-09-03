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
    // Create the State's name data
    std::string name = "StateA";
    // Create the State's valid transitions data
    std::vector<std::string> transitions = { "StateB", "StateC" };
    // Create the State
    State S = State (name, transitions);

    // Access the State's name data
    std::string *pNameResult;
    Error_t ret = S.getName (&pNameResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (pNameResult->compare (name) == 0);

    // Access the State's transition data
    std::vector<std::string> *pTransitionsResult;
    ret = S.getTransitions (&pTransitionsResult);
    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_TRUE (transitions == *pTransitionsResult);
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
        *pLocalMap;
    Error_t ret = S.getActionSequence (&pLocalMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // search the map
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        ::const_iterator search;

    // at key 0, vector of tuple of pointer to function multiply and param 3
    search = pLocalMap->find (0);
    CHECK_EQUAL (search->first, 0);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncM);
    CHECK_EQUAL (std::get<1> (search->second[0]), 3);

    // at key 1, vector of tuple of pointer to function add and param 5
    search = pLocalMap->find (1);
    CHECK_EQUAL (search->first, 1);
    CHECK_EQUAL (std::get<0> (search->second[0]), pFuncA);
    CHECK_EQUAL (std::get<1> (search->second[0]), 5);

    // at key 2, tuple of pointer to function subtract and param 3
    search = pLocalMap->find (2);
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
        *pLocalMap;
    Error_t ret = S.getActionSequence (&pLocalMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // search the map
    std::map<int32_t, std::vector<std::tuple<Error_t (*) (int32_t), int32_t>>>
        ::const_iterator search;

    // at timestamp 0, a vector of three tuples: multiply/3, add/5, subtract/3
    search = pLocalMap->find (0);
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
