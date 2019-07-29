#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include "UDPClient.hpp"

/* Use Internet Protocol*/
const int UDPClient::DOMAIN = AF_INET;
/* Specifies that this will be a UDP socket*/
const int UDPClient::TYPE = SOCK_DGRAM;
/* Default */
const int UDPClient::PROTOCOL = 0;

/******************** PUBLIC FUNCTIONS **************************/

UDPClient::~UDPClient ()
{
    closeSocket ();
}

Error_t UDPClient::createNew (std::shared_ptr<UDPClient>& pClientRet,
                            bool kBlocking)
{
    int sockOptions = 0;
    if (!kBlocking)
    {
        sockOptions = SOCK_NONBLOCK;
    }

    // Initialize the socket and hold on to its file descriptor
    int sockFD = socket (DOMAIN, TYPE | sockOptions, PROTOCOL);

    if (sockFD < 1)
    {
        return E_FAILED_TO_CREATE_SOCKET;
    }

    // Can't use make_shared here, because UDPClient constructor is private
    pClientRet.reset (new UDPClient (sockFD) );

    if (pClientRet == nullptr)
    {
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    return E_SUCCESS;
}

Error_t UDPClient::send (std::vector<uint8_t>& kBuf, size_t kLen,
                        uint32_t kDstIPAddr, uint16_t kDstPort)
{
    if (!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }
    else if (kLen > kBuf.size () || kBuf.size () <= 0)
    {
        return E_INVALID_BUF_LEN;
    }

    struct sockaddr_in destAddr;
    memset ( (void*)(&destAddr), 0, (unsigned int)sizeof (destAddr) );

    // Fill client information
    destAddr.sin_family = DOMAIN;

    destAddr.sin_port = htons (kDstPort);
    destAddr.sin_addr.s_addr = kDstIPAddr;

    // sendto() will return the number of bytes sent if successful
    int n = sendto (mSocket, &kBuf[0], kLen, 0,
            (const struct sockaddr*)&destAddr, sizeof (destAddr) );

    if (n < 0)
    {
        return E_FAILED_TO_SEND_DATA;
    }
    else if ( (size_t)n != kLen)
    {
        return E_PARTIAL_SEND;
    }

    return E_SUCCESS;
}


Error_t UDPClient::closeSocket ()
{
    int retClose = close (mSocket);

    if (retClose < 0)
    {
        return E_FAILED_TO_CLOSE_SOCKET;
    }

    return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

UDPClient::UDPClient (int kSockFD)
{
    mInitialized = false;
    mSocket = kSockFD;
    if (mSocket > 0)
    {
        mInitialized = true;
    }
}
