#include "CppUTest/TestHarness.h"
#include "Errors.h"
#include "UDPSocket.hpp"



/********************************* TESTS **************************************/

TEST_GROUP (NetworkInterface)
{
};

/* Test networkInterface initialization */
TEST(NetworkInterface, NetworkInit)
{

    Error_t ret;
    uint16_t port = 50000;
    UDPSocket* pSocket = nullptr;

    ret = UDPSocket::createNew(pSocket, port);
    CHECK_EQUAL(E_SUCCESS, ret);

    delete pSocket;
}

TEST(NetworkInterface, SendRecv)
{
    Error_t ret;
    uint16_t port = 50000;
    uint8_t loopbackIPAddr[4] = {127,0,0,1};
    uint8_t buf[4] = {0,1,2,3};
    uint8_t recBuf[4];

    UDPSocket* pSocket = nullptr;

    ret = UDPSocket::createNew(pSocket, port);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pSocket->send(buf, 4, loopbackIPAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pSocket->recv(recBuf, 4, loopbackIPAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);

    for(int i=0; i<4; i++){
        CHECK_EQUAL(recBuf[i], buf[i]);
    }

    delete pSocket;

}


