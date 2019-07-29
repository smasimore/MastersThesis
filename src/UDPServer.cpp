#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <errno.h>

#include "UDPServer.hpp"

/* Use Internet Protocol*/
const int UDPServer::DOMAIN = AF_INET;
/* Specifies that this will be a UDP socket*/
const int UDPServer::TYPE = SOCK_DGRAM;
/* Default */
const int UDPServer::PROTOCOL = 0;

/******************** PUBLIC FUNCTIONS **************************/

Error_t UDPServer::createNew (std::shared_ptr<UDPServer>& pServerRet,
                            uint16_t kPort, bool kBlocking)
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

    // Set the socket to reuse the address.
    // If this option is not set, opening a socket on the same port as a
    // recently closed socket will fail.
    int option = 1;
    setsockopt (sockFD, SOL_SOCKET, SO_REUSEADDR, &option, sizeof (option) );

    struct sockaddr_in addr;
    memset ( (void*)(&addr), 0, (unsigned int)sizeof(addr) );

    // Fill server information
    addr.sin_family = AF_INET; // IPv4
    addr.sin_addr.s_addr = INADDR_ANY;
    // Convert port to from host to network byte order
    addr.sin_port = htons (kPort);

    // Assign a name to the socket
    int retBind = bind (sockFD, (const struct sockaddr *)&addr,
            sizeof (addr) );

    if (retBind < 0)
    {
        return E_FAILED_TO_BIND_TO_SOCKET;
    }

    pServerRet.reset (new UDPServer (sockFD, kPort) );

    if (pServerRet == nullptr)
    {
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    return E_SUCCESS;
}

Error_t UDPServer::recv (std::vector<uint8_t>& kBuf, size_t& lenRet,
                        uint32_t& srcIPAddrRet, bool kPeek)
{
    if (!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }
    else if (kBuf.size () < 1)
    {
        return E_INVALID_BUF_LEN;
    }

    struct sockaddr_in srcAddr;
    memset ( (void*)(&srcAddr), 0, (unsigned int)sizeof (srcAddr) );

    // MSG_TRUNC causes recvfrom() return the total size of the received packet
    // even if it is larger than the buffer supplied.
    int flags = MSG_TRUNC;
    if (kPeek)
    {
        flags |= MSG_PEEK;
    }

    socklen_t origSrcAddrLen = sizeof (srcAddr);
    socklen_t srcAddrLen = origSrcAddrLen;

    // recvfrom() will return the number of bytes received if successful
    // or the number of bytes that could be received if the buffer is too small
    int ret = recvfrom (mSocket, &kBuf[0], kBuf.size (), flags,
            (struct sockaddr*)&srcAddr, &srcAddrLen);

    srcIPAddrRet = srcAddr.sin_addr.s_addr;

    if (ret < 0)
    {
        lenRet = 0;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No data available right now
            return E_WOULD_BLOCK;
        }
        return E_FAILED_TO_RECV_DATA;
    }
    else
    {
        lenRet = ret;
        if (lenRet > kBuf.size () )
        {
            // Buffer was not large enough for the received message and the message
            // was truncated
            return E_RECV_TRUNC;
        }
        else if (srcAddrLen > origSrcAddrLen)
        {
            // Source address was longer than the buffer supplied, and the returned
            // address was truncated.
            return E_INVALID_SRC_ADDR;
        }
    }

    return E_SUCCESS;
}

/******************** PRIVATE FUNCTIONS **************************/

UDPServer::UDPServer (int kSockFD, uint16_t kPort)
{
    mInitialized = false;
    mPort = kPort;
    mSocket = kSockFD;

    if (mSocket > 0)
    {
        mInitialized = true;
    }
}
