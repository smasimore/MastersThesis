#include <iostream>

#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Log.hpp"
#include "ThreadManager.hpp"
 
TEST_GROUP (ThreadManager)
{
};

/* Test ThreadManager singleton. */
TEST (ThreadManager, ConstructTwo)
{
    // Get first instance.
    ThreadManager *pThreadManagerOne = nullptr;
    Error_t ret = ThreadManager::getInstance (&pThreadManagerOne);
    CHECK_TRUE (ret == E_SUCCESS);
    CHECK_TRUE (pThreadManagerOne != nullptr);
    CHECK_TRUE (typeid (*pThreadManagerOne) == typeid (ThreadManager));

    // Get second instance.
    ThreadManager *pThreadManagerTwo = nullptr;
    ret = ThreadManager::getInstance (&pThreadManagerTwo);
    CHECK_TRUE (ret == E_SUCCESS);
    CHECK_TRUE (pThreadManagerTwo != nullptr);
    CHECK_TRUE (typeid (*pThreadManagerTwo) == typeid (ThreadManager));

    // Verify they point to the same ThreadManager.
    CHECK_TRUE (pThreadManagerOne == pThreadManagerTwo);
}

/* Test creating threads with invalid params. */
TEST (ThreadManager, CreateThreadInvalid)
{

}
