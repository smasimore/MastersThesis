#include "CppUTest/TestHarness.h"
#include "Example.h"
 
TEST_GROUP(Examples)
{
};
 
TEST(Examples, Pass)
{
  CHECK_EQUAL(0, retZero());
}

TEST(Examples, Fail)
{
  CHECK_EQUAL(1, retZero());
}
