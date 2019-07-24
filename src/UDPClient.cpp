#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include <UDPClient.hpp>

/* Use Internet Protocol*/
const int UDPClient::DOMAIN = AF_INET;

/* Specifies that this will be a UDP socket*/
const int UDPClient::TYPE = SOCK_DGRAM;

/* Default */
const int UDPClient::PROTOCOL = 0;



UDPClient::~UDPClient()
{
    closeSocket();
}

Error_t UDPClient::createNew(std::shared_ptr<UDPClient>& pClientRet,
        uint16_t kPort, bool kBlocking)
{
    Error_t ret;
    // Can't use make_shared here, because UDPClient constructor is private
    pClientRet.reset(new UDPClient(ret, kPort, kBlocking));

    if(pClientRet == nullptr){
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    if(ret != E_SUCCESS){
        return ret;
    }

    return E_SUCCESS;
}

Error_t UDPClient::send(std::vector<uint8_t> kBuf, size_t kLen,
                        uint32_t kDstIPAddr)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }
    else if(kLen > kBuf.size() || kBuf.size() <= 0)
    {
        return E_INVALID_BUF_LEN;
    }


    struct sockaddr_in destAddr;
    memset((void*)(&destAddr), 0, (unsigned int)sizeof(destAddr));

    // Fill client information
    destAddr.sin_family = DOMAIN;
    destAddr.sin_port = htons(mPort);
    destAddr.sin_addr.s_addr = kDstIPAddr;

    // sendto() will return the number of bytes sent if successful
    int n = sendto(mSocket, &kBuf[0], kLen, 0,
            (const struct sockaddr*)&destAddr, sizeof(destAddr));

    if(n < 0)
    {
        return E_FAILED_TO_SEND_DATA;
    }
    else if((size_t)n != kLen){
        return E_PARTIAL_SEND;
    }

    return E_SUCCESS;
}

Error_t UDPClient::recv(std::vector<uint8_t>& kBuf, size_t& lenRet,
                        uint32_t& retSrcAddr, bool kPeek)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }
    else if (kBuf.size() < 1)
    {
        return E_INVALID_BUF_LEN;
    }


    struct in_addr srcAddr;
    memset((void*)(&srcAddr), 0, (unsigned int)sizeof(srcAddr));


    // MSG_TRUNC causes recvfrom() return the total size of the received packet
    // even if it is larger than the buffer supplied.
    int flags = MSG_TRUNC;
    if(kPeek)
    {
        flags |= MSG_PEEK;
    }

    socklen_t origSrcAddrLen = sizeof(srcAddr);
    socklen_t srcAddrLen = origSrcAddrLen;

    // recvfrom() will return the number of bytes received if successful
    // or the number of bytes that could be received if the buffer is too small
    int ret = recvfrom(mSocket, &kBuf[0], kBuf.size(), flags, (struct sockaddr*)&srcAddr,
            &srcAddrLen);

    retSrcAddr = srcAddr.s_addr;
    lenRet = (size_t)ret;

    if(ret < 0)
    {
        lenRet = 0;
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No data available right now
            return E_WOULD_BLOCK;
        }
        return E_FAILED_TO_RECV_DATA;
    }
    else
    {
        lenRet = ret;
        if(lenRet > kBuf.size())
        {
            // Buffer was not large enough for the received message, and it was
            // truncated

            return E_RECV_TRUNC;
        }
        else if(srcAddrLen > origSrcAddrLen)
        {
            // Source address was longer than the buffer supplied, and the returned
            // address was truncated.
            return E_INVALID_SRC_ADDR;
        }
    }

    return E_SUCCESS;
}


Error_t UDPClient::closeSocket()
{
    int retClose = close(mSocket);

    if(retClose < 0){
        return E_FAILED_TO_CLOSE_SOCKET;
    }

    return E_SUCCESS;
}


UDPClient::UDPClient(Error_t& ret, uint16_t kPort, bool kBlocking)
{
    mPort = kPort;
    mInitialized = false;

    int sockOptions = 0;
    if(!kBlocking)
    {
        sockOptions = SOCK_NONBLOCK;
    }

    // Initialize the socket and hold on to its file descriptor
    mSocket = socket(DOMAIN, TYPE | sockOptions, PROTOCOL);

    if(mSocket < 1)
    {
        // Prints for testing only. TODO: remove
        std::cout<< "Socket create failed: " << strerror(errno) << std::endl;
        ret = E_FAILED_TO_CREATE_SOCKET;
        return;
    }

    ret = E_SUCCESS;
    mInitialized = true;
}
