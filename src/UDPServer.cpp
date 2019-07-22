#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <errno.h>

#include <UDPServer.hpp>

/* Use Internet Protocol*/
const int UDPServer::DOMAIN = AF_INET;

/* Specifies that this will be a UDP socket*/
const int UDPServer::TYPE = SOCK_DGRAM;

/* Default */
const int UDPServer::PROTOCOL = 0;



// TODO: finish reading this http://man7.org/linux/man-pages/man7/udp.7.html

Error_t UDPServer::createNew(std::shared_ptr<UDPServer>& pServerRet,
                            uint16_t kPort, bool kBlocking)
{
    Error_t ret;
    // Can't use make_shared here, because UDPServer constructor is private
    pServerRet.reset(new UDPServer(ret, kPort, kBlocking));

    if(pServerRet == nullptr){
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    if(ret != E_SUCCESS){
        return ret;
    }

    return E_SUCCESS;
}

Error_t UDPServer::send(std::vector<uint8_t> kBuf, int kLen,
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

    struct in_addr dest_addr;
    memset((void*)(&dest_addr), 0, (unsigned int)sizeof(dest_addr));

    // Fill client information
    dest_addr.s_addr = kDstIPAddr;

    // sendto() will return the number of bytes sent if successful
    int n = sendto(mSocket, &kBuf[0], kLen, 0, (const struct sockaddr*)&dest_addr,
            sizeof(dest_addr));

    if(n < 0)
    {
        return E_FAILED_TO_SEND_DATA;
    }
    else if(n != kLen){
        return E_PARTIAL_SEND;
    }

    return E_SUCCESS;
}

Error_t UDPServer::recv(std::vector<uint8_t> kBuf, int& lenRet,
                        uint32_t& srcIPAddrRet, bool kPeek)
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
    lenRet = recvfrom(mSocket, &kBuf[0], kBuf.size(), flags,
            (struct sockaddr*)&srcAddr, &srcAddrLen);

    srcIPAddrRet = srcAddr.s_addr;

    if(lenRet < 0)
    {
        return E_FAILED_TO_RECV_DATA;
    }
    else if(lenRet > kBuf.size()){
        // Buffer was not large enough for the received message and the message
        // was truncated
        return E_RECV_TRUNC;
    }
    else if(srcAddrLen > origSrcAddrLen){
        // Source address was longer than the buffer supplied, and the returned
        // address was truncated.
        return E_INVALID_SRC_ADDR;
    }
    else if(mBlocking && lenRet == 0 ){
        return E_CLIENT_SHUTDOWN;
    }

    return E_SUCCESS;
}


UDPServer::UDPServer(Error_t& ret, uint16_t kPort, bool kBlocking)
{
    mPort = kPort;
    mInitialized = false;
    mBlocking = kBlocking;

    int sockOptions = 0;
    if(kBlocking)
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

    // Set the socket to reuse the address.
    // If this option is not set, opening a socket on the same port as a
    // recently closed socket will fail.
    int option = 1;
    setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));


    struct sockaddr_in addr;
    memset((void*)(&addr), 0, (unsigned int)sizeof(addr));

    // Fill server information
    addr.sin_family    = AF_INET; // IPv4
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(mPort);

    // Assign a name to the socket
    int retSock = bind(mSocket, (const struct sockaddr *)&addr,
            sizeof(addr));


    if (retSock < 0)
    {
        // Prints for testing only. TODO: remove
        std::cout<< "Socket bind failed: " << strerror(errno) << std::endl;
        ret = E_FAILED_TO_BIND_TO_SOCKET;
        return;
    }


    ret = E_SUCCESS;
    mInitialized = true;
}
