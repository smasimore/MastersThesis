#include <memory.h>
#include <vector>

#include "CppUTest/TestHarness.h"
#include "Errors.h"
#include "UDPClient.hpp"
#include "UDPServer.hpp"



/********************************* TESTS **************************************/

TEST_GROUP (Sockets)
{
};

/* Test socket initialization */
TEST(Sockets, Init)
{

    Error_t ret;
    uint16_t port = 8008;

    std::shared_ptr<UDPServer> pServer;
    std::shared_ptr<UDPClient> pClient;

    // Create blocking sockets
    ret = UDPServer::createNew(pServer, port, true);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = UDPClient::createNew(pClient, port, true);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);

    pServer.reset();
    pClient.reset();

    //Create non-blocking sockets
    ret = UDPServer::createNew(pServer, port, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = UDPClient::createNew(pClient, port, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);


}

TEST(Sockets, ServerToClient)
{
    Error_t ret;
    uint16_t port = 8008;
    uint32_t loopbackIPAddr = (0xFF&127) << 24 ||
                              (0&0xFF) << 16   ||
                              (0&0xFF) << 8    ||
                              (1&0xFF);
    std::vector<uint8_t> buf{0,1,2,3};
    std::vector<uint8_t> recvBuf(4);

    // Create Server and Client objects
    std::shared_ptr<UDPServer> pServer;
    std::shared_ptr<UDPClient> pClient;

    ret = UDPServer::createNew(pServer, port, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = UDPClient::createNew(pClient, port, false);
    CHECK_EQUAL(E_SUCCESS, ret);

    //// Test working case

    // Test non-blocking send/receive functionality
    ret = pServer->send(buf, buf.size(), loopbackIPAddr);
    CHECK_EQUAL(E_SUCCESS, ret);

    // recv with peek
    uint32_t srcAddr = 0;
    size_t bytesReceived = 0;
    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, true);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(buf == recvBuf);
    CHECK_TRUE(bytesReceived == buf.size());
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    // recv without peek
    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(buf == recvBuf);
    CHECK_TRUE(bytesReceived == buf.size());
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    // recv again should return 0 bytes
    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(bytesReceived == 0);

    // Test sending only a portion of buf
    size_t len = 2;
    ret = pServer->send(buf, len, loopbackIPAddr);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(std::equal(recvBuf.begin(), recvBuf.begin()+len, recvBuf.begin()));
    CHECK_TRUE(bytesReceived == len);
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    //// Test failure modes

    // Invalid IP
    uint32_t badIP = 0;
    ret = pServer->send(buf, buf.size(), badIP);
    CHECK_EQUAL(E_FAILED_TO_SEND_DATA, ret);

    // Empty buffer
    std::vector<uint8_t> emptyBuf;
    ret = pServer->send(emptyBuf, emptyBuf.size(), loopbackIPAddr);
    CHECK_EQUAL(E_INVALID_BUF_LEN, ret);

    // buf too small
    ret = pServer->send(buf, buf.size()+1, loopbackIPAddr);
    CHECK_EQUAL(E_INVALID_BUF_LEN, ret);

    // recvBuf too small
    std::vector<uint8_t> largeBuf{1,2,3,4,5};
    ret = pServer->send(largeBuf, largeBuf.size(), loopbackIPAddr);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_RECV_TRUNC, ret);
    CHECK_TRUE(bytesReceived == 4);
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    // recv again should return 0 bytes
    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(bytesReceived == 0);

    //// Close client

    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pServer->send(buf, buf.size(), loopbackIPAddr);
    CHECK_EQUAL(E_FAILED_TO_SEND_DATA, ret);
    ret = pClient->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_FAILED_TO_RECV_DATA, ret);

}


