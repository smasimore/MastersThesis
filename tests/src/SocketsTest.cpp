#include <memory.h>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "CppUTest/TestHarness.h"
#include "Errors.h"
#include "UDPServer.hpp"
#include "UDPClient.hpp"
#include "ThreadManager.hpp"

/**************************** HELPER FUNCTIONS ********************************/

/**
 * Params to pass args to send/recv functions.
 */
struct SendFuncArgs
{
    std::shared_ptr<UDPClient> pClient;
    std::vector<uint8_t>& buf;
    size_t len;
    uint32_t ip;
    uint16_t port;
};

struct RecvFuncArgs
{
    std::shared_ptr<UDPServer> pServer;
};

Error_t recvDataBlocking (RecvFuncArgs args)
{
    Error_t ret;

    std::vector<uint8_t> recvBuf (4);
    size_t bytesReceived;
    uint32_t srcAddr = 0;

    ret = args.pServer->recv (recvBuf, bytesReceived, srcAddr, false);
    // It isn't necessary to check the values of recBuf and bytesReceived here,
    // because that functionality will be verified in other tests.

    return ret;
}

Error_t sendDataBlocking (SendFuncArgs args)
{
    Error_t ret;

    ret = args.pClient->send (args.buf, args.len, args.ip, args.port);

    return ret;
}

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
    ret = UDPClient::createNew (pClient, true);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = UDPServer::createNew (pServer, serverPort, true);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pClient->closeSocket ();
    CHECK_EQUAL (E_SUCCESS, ret);

    pClient.reset ();
    pServer.reset ();

    //Create non-blocking sockets
    ret = UDPClient::createNew(pClient, false);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = UDPServer::createNew(pServer, serverPort, false);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pClient->closeSocket();
    CHECK_EQUAL (E_SUCCESS, ret);
}

/* Test sending and receiving data non non-blocking sockets */
TEST (Sockets, SendRecvNonBlock)
{
    Error_t ret;
    uint16_t serverPort = 8008;
    // Local network ip: 127.0.0.1
    uint32_t loopbackIPAddr = (1 & 0xFF) << 24 |
                              (0 & 0xFF) << 16 |
                              (0 & 0xFF) << 8  |
                              (127 & 0xFF);

    std::vector<uint8_t> buf {0,1,2,3};
    std::vector<uint8_t> recvBuf (4);

    // Create Server and Client objects
    std::shared_ptr<UDPClient> pClient;
    std::shared_ptr<UDPServer> pServer;

    ret = UDPClient::createNew(pClient, false);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = UDPServer::createNew(pServer, serverPort, false);
    CHECK_EQUAL (E_SUCCESS, ret);

    //// Test working case

    // Test non-blocking send/receive functionality
    ret = pClient->send (buf, buf.size (), loopbackIPAddr, serverPort);
    CHECK_EQUAL (E_SUCCESS, ret);

    // recv with peek
    uint32_t srcAddr = 0;
    size_t bytesReceived = 0;
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, true);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_TRUE (buf == recvBuf);
    CHECK_EQUAL (buf.size (), bytesReceived);
    CHECK_TRUE (srcAddr = loopbackIPAddr);

    // recv without peek
    ret = pServer->recv (recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_TRUE (buf == recvBuf);
    CHECK_EQUAL (buf.size (), bytesReceived);
    CHECK_TRUE (srcAddr = loopbackIPAddr);

    // recv again should receive nothing
    ret = pServer->recv (recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL (E_WOULD_BLOCK, ret);
    CHECK_EQUAL (0, bytesReceived);

    // Test sending only a portion of buf
    size_t len = 2;
    ret = pClient->send (buf, len, loopbackIPAddr, serverPort);
    CHECK_EQUAL (E_SUCCESS, ret);

    ret = pServer->recv (recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_TRUE (std::equal(recvBuf.begin (), recvBuf.begin ()+len, buf.begin ()));
    CHECK_EQUAL (len, bytesReceived);
    CHECK_TRUE (srcAddr = loopbackIPAddr);

    //// Test failure modes

    // Empty buffer
    std::vector<uint8_t> emptyBuf;
    ret = pClient->send (emptyBuf, emptyBuf.size (), loopbackIPAddr, serverPort);
    CHECK_EQUAL (E_INVALID_BUF_LEN, ret);

    // buf too small
    ret = pClient->send (buf, buf.size ()+1, loopbackIPAddr, serverPort);
    CHECK_EQUAL (E_INVALID_BUF_LEN, ret);

    // recvBuf too small
    std::vector<uint8_t> largeBuf {1,2,3,4,5};
    ret = pClient->send (largeBuf, largeBuf.size (), loopbackIPAddr, serverPort);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pServer->recv (recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL (E_RECV_TRUNC, ret);
    CHECK_TRUE (std::equal(recvBuf.begin (), recvBuf.end (), largeBuf.begin ()))
    CHECK_EQUAL (largeBuf.size (), bytesReceived);
    CHECK_TRUE (srcAddr = loopbackIPAddr);

    // recv again should receive nothing
    ret = pServer->recv(recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL (E_WOULD_BLOCK, ret);
    CHECK_EQUAL (0, bytesReceived);

    //// Close client

    ret = pClient->closeSocket ();
    CHECK_EQUAL (E_SUCCESS, ret);

    ret = pClient->send (buf, buf.size (), loopbackIPAddr, serverPort);
    CHECK_EQUAL (E_FAILED_TO_SEND_DATA, ret);
    ret = pServer->recv (recvBuf, bytesReceived, srcAddr, false);
    CHECK_EQUAL (E_WOULD_BLOCK, ret);

}

/* Test sending and receiving data non blocking sockets */
TEST (Sockets, SendRecvBlock)
{
    Error_t ret;

    //// Set up client socket

    uint16_t serverPort = 8008;

    // Local network ip: 127.0.0.1
    uint32_t loopbackIPAddr = (1 & 0xFF) << 24 |
                              (0 & 0xFF) << 16 |
                              (0 & 0xFF) << 8  |
                              (127 & 0xFF);

    std::vector<uint8_t> buf {0,1,2,3};

    // Create Server and Client objects
    std::shared_ptr<UDPClient> pClient;
    std::shared_ptr<UDPServer> pServer;

    ret = UDPClient::createNew(pClient, true);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = UDPServer::createNew(pServer, serverPort, true);
    CHECK_EQUAL (E_SUCCESS, ret);

    //// Set up threads

    // Initialize ThreadManager
    ThreadManager *pThreadManager = nullptr;
    ret = ThreadManager::getInstance (&pThreadManager);
    CHECK_EQUAL (E_SUCCESS, ret);

    pthread_t sendThread;
    pthread_t recvThread;

    ThreadManager::ThreadFunc_t *pSendFunc =
        (ThreadManager::ThreadFunc_t *) &sendDataBlocking;
    ThreadManager::ThreadFunc_t *pRecvFunc =
        (ThreadManager::ThreadFunc_t *) &recvDataBlocking;

    struct SendFuncArgs sendArgs = {pClient, buf, buf.size (), loopbackIPAddr,
                                  serverPort};
    struct RecvFuncArgs recvArgs = {pServer};

    // Create the the threads
    ret = pThreadManager->createThread (recvThread, pRecvFunc,
                                        &recvArgs, sizeof (recvArgs),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pThreadManager->createThread (sendThread, pSendFunc,
                                        &sendArgs, sizeof (sendArgs),
                                        ThreadManager::MIN_NEW_THREAD_PRIORITY
                                            + 1,
                                        ThreadManager::Affinity_t::ALL);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Wait for threads.
    Error_t threadReturn;
    ret = pThreadManager->waitForThread (sendThread, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (E_SUCCESS, threadReturn);
    ret = pThreadManager->waitForThread (recvThread, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    CHECK_EQUAL (E_SUCCESS, threadReturn);
}

/**
 *  Tests communication between two sbRIOs. Each RIO sends and receives a
 *  string.
 *
 *  Currently the address of each sbRIO must be hard-coded (by setting rioNo).
 *  When config file parsing has been implemented, addresses and serial numbers
 *  should be read from a config.
 *
 *  This will only succeed if two sbRIOs are connected to the same network.
 *  To disable this test, use the IGNORE_TEST macro
 */
IGNORE_TEST (Sockets, RocketNetworkComms)
{
    // Last 2 digits of serial number
    int rioNo = 0x07;
    // int rioNo = 0xDB;

    Error_t ret;
    uint16_t serverPort = 8008;

    std::string sendStr0 = "It is better to send than recv";
    std::string sendStr1 = "ditto";
    std::vector<uint8_t> rio0SendBuf {sendStr0.begin (), sendStr0.end ()};
    std::vector<uint8_t> rio1SendBuf {sendStr1.begin (), sendStr1.end ()};
    std::vector<uint8_t> recvBuf (256);

    if (rioNo == 0xDB)
    {
        uint32_t rio1Addr = (0x07 & 0xFF) << 24 | // Last 2 digits of dest. SN
                            (1 & 0xFF) << 16 |
                            (1 & 0xFF) << 8 |
                            (10 & 0xFF);

        // Create Server and Client objects
        std::shared_ptr<UDPClient> pClient;
        std::shared_ptr<UDPServer> pServer;

        ret = UDPClient::createNew (pClient, true);
        CHECK_EQUAL (E_SUCCESS, ret);
        ret = UDPServer::createNew (pServer, serverPort, true);
        CHECK_EQUAL (E_SUCCESS, ret);

        // RIO 4db sends first

        // Test non-blocking send/receive functionality
        ret = pClient->send (rio0SendBuf, rio0SendBuf.size (), rio1Addr, serverPort);
        CHECK_EQUAL (E_SUCCESS, ret);

        // recv with peek
        uint32_t srcAddr = 0;
        size_t bytesReceived = 0;
        ret = pServer->recv (recvBuf, bytesReceived, srcAddr, true);

        std::cout << "Received '" <<
                std::string (recvBuf.begin (), recvBuf.end ()) <<
                "' from " <<
                srcAddr <<
                std::endl;

        CHECK_EQUAL (E_SUCCESS, ret);
        CHECK_EQUAL (rio1SendBuf.size (), bytesReceived);
        CHECK_TRUE (std::equal(rio1SendBuf.begin (), rio1SendBuf.end (),
                              recvBuf.begin ()));
        CHECK_TRUE (srcAddr = rio1Addr);
    }
    else if (rioNo == 0x07) // 507
    {
        uint32_t rio0Addr = (0xDB & 0xFF) << 24 | // Last 2 digits of dest. SN
                            (1 & 0xFF) << 16 |
                            (1 & 0xFF) << 8 |
                            (10 & 0xFF);

        // Create Server and Client objects
        std::shared_ptr<UDPClient> pClient;
        std::shared_ptr<UDPServer> pServer;

        ret = UDPClient::createNew (pClient, true);
        CHECK_EQUAL (E_SUCCESS, ret);
        ret = UDPServer::createNew (pServer, serverPort, true);
        CHECK_EQUAL (E_SUCCESS, ret);

        // RIO 4db sends first

        // recv with peek
        uint32_t srcAddr = 0;
        size_t bytesReceived = 0;
        ret = pServer->recv (recvBuf, bytesReceived, srcAddr, true);
        CHECK_EQUAL (E_SUCCESS, ret);

        std::cout << "Received '" <<
                std::string(recvBuf.begin (), recvBuf.end ()) <<
                "' from " <<
                srcAddr <<
                std::endl;

        CHECK_EQUAL (rio0SendBuf.size (), bytesReceived);
        CHECK_TRUE (std::equal (rio0SendBuf.begin (), rio0SendBuf.end (),
                               recvBuf.begin ()));
        CHECK_TRUE (srcAddr = rio0Addr);

        // Test non-blocking send/receive functionality
        ret = pClient->send (rio1SendBuf, rio1SendBuf.size (), rio0Addr, serverPort);
        CHECK_EQUAL (E_SUCCESS, ret);
    }
}
