#include "CppUTest/TestHarness.h"
#include "Errors.h"
#include "NetworkInterface.hpp"



/********************************* TESTS **************************************/

TEST_GROUP (NetworkInterface)
{
};

/* Test networkInterface initialization */
TEST(NetworkInterface, NetworkInit)
{
    Error_t ret;
    u16 port = 8000; 
    u8 buf[4] = {0,1,2,3};

    ret = NetworkInterface::send(buf, 4, false);
    CHECK_EQUAL(E_NETWORK_NOT_INITIALIZED, ret);

    ret = NetworkInterface::recv(buf, 4, false);
    CHECK_EQUAL(E_NETWORK_NOT_INITIALIZED, ret);

    ret = NetworkInterface::networkInit(port);
    CHECK_EQUAL(E_SUCCESS, ret);
}


