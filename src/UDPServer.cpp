#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include <UDPServer.hpp>

/* Use Internet Protocol*/
const int UDPServer::DOMAIN = AF_INET;

/* Specifies that this will be a UDP socket*/
const int UDPServer::TYPE = SOCK_DGRAM;

/* Default */
const int UDPServer::PROTOCOL = 0;



// TODO: finish reading this http://man7.org/linux/man-pages/man7/udp.7.html

Error_t UDPServer::createNew(std::shared_ptr<UDPServer>& pServerRet, uint16_t port)
{
    Error_t ret;
    // Can't use make_shared here, because UDPServer constructor is private
    pServerRet.reset(new UDPServer(ret, port));

    if(pServerRet == nullptr){
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    if(ret != E_SUCCESS){
        return E_FAILED_TO_CREATE_SOCKET;
    }

    return E_SUCCESS;
}

Error_t UDPServer::send(uint8_t* buf, int len, uint8_t* dstIPAddr,
                        bool blocking)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }



    return E_SUCCESS;
}

Error_t UDPServer::recv(uint8_t* buf, int len, uint8_t* srcIPAddr,
                        bool blocking)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }

    return E_SUCCESS;
}


UDPServer::UDPServer(Error_t& ret, uint16_t port)
{
    mPort = port;
    mInitialized = false;

    // Initialize the socket and hold on to its file descriptor
    mSocket = socket(DOMAIN, TYPE, PROTOCOL);

    if(mSocket < 1)
    {
        ret = E_FAILED_TO_CREATE_SOCKET;
        return;
    }

    struct sockaddr_in addr;
    memset((void*)(&addr), 0, (unsigned int)sizeof(addr));

    // Filling server information
    addr.sin_family    = AF_INET; // IPv4
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Assign a name to the socket
    int retSock = bind(mSocket, (const struct sockaddr *)&addr,
            sizeof(addr));

    if (retSock < 0)
    {
        ret = E_FAILED_TO_BIND_TO_SOCKET;
        return;
    }


    ret = E_SUCCESS;
    mInitialized = true;
}
