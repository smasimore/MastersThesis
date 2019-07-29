#include <memory.h>
#include <vector>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "Errors.h"
#include "UDPServer.hpp"
#include "UDPClient.hpp"

/********************************* TESTS **************************************/

TEST_GROUP (Sockets)
{
};

/* Test socket initialization */
TEST (Sockets, Init)
{

    Error_t ret;
    uint16_t serverPort = 8008;

    std::shared_ptr<UDPClient> pClient;
    std::shared_ptr<UDPServer> pServer;

    // Create blocking sockets
    ret = UDPClient::createNew(pClient, true);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = UDPServer::createNew(pServer, serverPort, true);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);

    pClient.reset();
    pServer.reset();

    //Create non-blocking sockets
    ret = UDPClient::createNew(pClient, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = UDPServer::createNew(pServer, serverPort, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);
}

TEST (Sockets, SendRecv)
{
    Error_t ret;
    uint16_t serverPort = 8009;
    // Local network ip: 127.0.0.1
    uint32_t loopbackIPAddr = (1 & 0xFF) << 24 |
                              (0 & 0xFF) << 16 |
                              (0 & 0xFF) << 8  |
                              (127 & 0xFF);

    std::vector<uint8_t> buf{0,1,2,3};
    std::vector<uint8_t> recvBuf(4);

    // Create Server and Client objects
    std::shared_ptr<UDPClient> pClient;
    std::shared_ptr<UDPServer> pServer;

    ret = UDPClient::createNew(pClient, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = UDPServer::createNew(pServer, serverPort, false);
    CHECK_EQUAL(E_SUCCESS, ret);

    //// Test working case

    // Test non-blocking send/receive functionality
    ret = pClient->send(buf, buf.size(), loopbackIPAddr, serverPort);
    CHECK_EQUAL(E_SUCCESS, ret);

    // recv with peek
    uint32_t srcAddr = 0;
    size_t bytesReceived = 0;
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, true);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(buf == recvBuf);
    CHECK_EQUAL(buf.size(), bytesReceived);
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    // recv without peek
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(buf == recvBuf);
    CHECK_EQUAL(buf.size(), bytesReceived);
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    // recv again should receive nothing
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_WOULD_BLOCK, ret);
    CHECK_EQUAL(0, bytesReceived);

    // Test sending only a portion of buf
    size_t len = 2;
    ret = pClient->send(buf, len, loopbackIPAddr, serverPort);
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_SUCCESS, ret);
    CHECK_TRUE(std::equal(recvBuf.begin(), recvBuf.begin()+len, buf.begin()));
    CHECK_EQUAL(len, bytesReceived);
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    //// Test failure modes

    // Empty buffer
    std::vector<uint8_t> emptyBuf;
    ret = pClient->send(emptyBuf, emptyBuf.size(), loopbackIPAddr, serverPort);
    CHECK_EQUAL(E_INVALID_BUF_LEN, ret);

    // buf too small
    ret = pClient->send(buf, buf.size()+1, loopbackIPAddr, serverPort);
    CHECK_EQUAL(E_INVALID_BUF_LEN, ret);

    // recvBuf too small
    std::vector<uint8_t> largeBuf{1,2,3,4,5};
    ret = pClient->send(largeBuf, largeBuf.size(), loopbackIPAddr, serverPort);
    CHECK_EQUAL(E_SUCCESS, ret);
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_RECV_TRUNC, ret);
    CHECK_TRUE(std::equal(recvBuf.begin(), recvBuf.end(), largeBuf.begin()))
    CHECK_EQUAL(largeBuf.size(), bytesReceived);
    CHECK_TRUE(srcAddr = loopbackIPAddr);

    // recv again should receive nothing
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_WOULD_BLOCK, ret);
    CHECK_EQUAL(0, bytesReceived);

    //// Close client

    ret = pClient->closeSocket();
    CHECK_EQUAL(E_SUCCESS, ret);

    ret = pClient->send(buf, buf.size(), loopbackIPAddr, serverPort);
    CHECK_EQUAL(E_FAILED_TO_SEND_DATA, ret);
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL(E_WOULD_BLOCK, ret);

    // TODO: Use threads to test blocking sockets

}


