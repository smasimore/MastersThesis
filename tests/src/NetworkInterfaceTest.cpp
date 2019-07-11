#include "CppUTest/TestHarness.h"
#include "Errors.h"
#include <memory.h>
#include "UDPClient.hpp"
#include "UDPServer.hpp"



/********************************* TESTS **************************************/

TEST_GROUP (NetworkInterface)
{
};

/* Test networkInterface initialization */
TEST(NetworkInterface, NetworkInit)
{

    Error_t ret;
    uint16_t port = 50000;

    std::shared_ptr<UDPServer> pServer;
    std::shared_ptr<UDPClient> pClient;

    ret = UDPServer::createNew(pServer, port);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = UDPClient::createNew(pClient, port);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);

}

TEST(NetworkInterface, SendRecv)
{
    Error_t ret;
    uint16_t port = 50000;
    uint8_t loopbackIPAddr[4] = {127,0,0,1};
    uint8_t buf[4] = {0,1,2,3};
    uint8_t recBuf[4];

    // Create Server and Client objects
    std::shared_ptr<UDPServer> pServer;
    std::shared_ptr<UDPClient> pClient;

    ret = UDPServer::createNew(pServer, port);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = UDPClient::createNew(pClient, port);
    CHECK_EQUAL(E_SUCCESS, ret);


    // Test non-blocking send/receive functionality
    ret = pServer->send(buf, 4, loopbackIPAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pClient->recv(recBuf, 4, loopbackIPAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);

    for(int i=0; i<4; i++){
        CHECK_EQUAL(recBuf[i], buf[i]);
    }

    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);

}


