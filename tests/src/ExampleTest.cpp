#include "Errors.h"
#include "Example.hpp"

#include "CppUTest/TestHarness.h"

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
