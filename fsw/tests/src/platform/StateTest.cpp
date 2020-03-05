#include "State.hpp"
#include "Errors.hpp"
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

    // create actions with timestamp, function pointer, and param
    State::Action_t actionM {0, pFuncM, 3};
    State::Action_t actionA {1, pFuncA, 5};
    State::Action_t actionS {2, pFuncS, 3};

    // create corresponding input vector of actions
    std::vector<State::Action_t> actionsIn = { actionM, actionA, actionS };

    // construct a state with input vector
    State S = State ("", {} , actionsIn);

    // retrieve the Action Sequence
    std::map<int32_t, std::vector<State::Action_t>>
        *pLocalMap;
    Error_t ret = S.getActionSequence (&pLocalMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // search the map
    std::map<int32_t, std::vector<State::Action_t>>
        ::const_iterator search;

    // at key 0, vector of tuple of pointer to function multiply and param 3
    search = pLocalMap->find (0);
    CHECK_EQUAL (search->first, 0);
    CHECK_EQUAL (search->second[0].func, pFuncM);
    CHECK_EQUAL (search->second[0].param, 3);

    // at key 1, vector of tuple of pointer to function add and param 5
    search = pLocalMap->find (1);
    CHECK_EQUAL (search->first, 1);
    CHECK_EQUAL (search->second[0].func, pFuncA);
    CHECK_EQUAL (search->second[0].param, 5);

    // at key 2, tuple of pointer to function subtract and param 3
    search = pLocalMap->find (2);
    CHECK_EQUAL (search->first, 2);
    CHECK_EQUAL (search->second[0].func, pFuncS);
    CHECK_EQUAL (search->second[0].param, 3);
}

/* Create an action sequence with a shared timestamp between functions */
TEST (States, SharedActions)
{
    // set up function pointers
    Error_t (*pFuncM) (int32_t) = multiplyParam;
    Error_t (*pFuncA) (int32_t) = addParam;
    Error_t (*pFuncS) (int32_t) = subtractParam;

    // create tuples of timestamp, function pointer, and param
    State::Action_t actionM {0, pFuncM, 3};
    State::Action_t actionA {0, pFuncA, 5};
    State::Action_t actionS {0, pFuncS, 3};

    // create corresponding input vector of tuples
    std::vector<State::Action_t> actionsIn = { actionM, actionA, actionS };

    // construct a state with input vector
    State S = State ("", {}, actionsIn);

    // retrieve the Action Sequence
    std::map < int32_t, std::vector < State::Action_t >>
        *pLocalMap;
    Error_t ret = S.getActionSequence (&pLocalMap);
    CHECK_TRUE (E_SUCCESS == ret);

    // search the map
    std::map<int32_t, std::vector<State::Action_t>>
        ::const_iterator search;

    // at timestamp 0, a vector of three tuples: multiply/3, add/5, subtract/3
    search = pLocalMap->find (0);
    CHECK_EQUAL (search->first, 0);

    // verify first element of vector
    CHECK_EQUAL (search->second[0].func, pFuncM);
    CHECK_EQUAL (search->second[0].param, 3);

    // verify second element of vector
    CHECK_EQUAL (search->second[1].func, pFuncA);
    CHECK_EQUAL (search->second[1].param, 5);

    // verify third element of vector
    CHECK_EQUAL (search->second[2].func, pFuncS);
    CHECK_EQUAL (search->second[2].param, 3);
}
