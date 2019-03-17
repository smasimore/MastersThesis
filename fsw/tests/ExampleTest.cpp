#include "CppUTest/TestHarness.h"
 
TEST_GROUP(Examples)
{
};
 
TEST(Examples, Fail)
{
  CHECK_EQUAL(0, 1);
}
