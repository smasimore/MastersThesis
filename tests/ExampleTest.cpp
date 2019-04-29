#include "CppUTest/TestHarness.h"

#include "Errors.h"
#include "Example.hpp"
 
TEST_GROUP (Examples)
{
};
 
TEST (Examples, Pass)
{
    uint8_t result;
    Error_t ret = retZero (result);

    CHECK_TRUE (E_SUCCESS == ret);
    CHECK_EQUAL (0, result);
}
