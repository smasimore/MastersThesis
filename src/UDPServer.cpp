#include <sys/types.h>
#include <sys/socket.h>

#include <UDPServer.hpp>

/* Use Internet Protocol*/
const int UDPServer::DOMAIN = AF_INET;

/* Specifies that this will be a UDP socket*/
const int UDPServer::TYPE = SOCK_DGRAM;

/* Default */
const int UDPServer::PROTOCOL = 0;



// TODO: finish reading this http://man7.org/linux/man-pages/man7/udp.7.html

Error_t UDPServer::createNew(UDPServer*& pSocketRet, uint16_t port)
{
    Error_t ret;
    pSocketRet = new UDPServer(ret, port);

    if(pSocketRet == nullptr){
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    if(ret != E_SUCCESS){
        return E_FAILED_TO_CREATE_SOCKET;
    }

    return E_SUCCESS;
}

Error_t UDPServer::send(uint8_t* buf, int len, uint8_t* dstIPAddr, bool blocking)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }



    return E_SUCCESS;
}

Error_t UDPServer::recv(uint8_t* buf, int len, uint8_t* srcIPAddr, bool blocking)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }

    return E_SUCCESS;
}


Wait, I still need to create separate server and client objects, right?


UDPServer::UDPServer(Error_t& ret, uint16_t port){
    ret = E_SUCCESS;
    mPort = port;
    mInitialized = false;

    // Initialize the socket and hold on to its file descriptor
    mSocket = socket(DOMAIN, TYPE, PROTOCOL);

    if(mSocket < 1){
        ret = E_FAILED_TO_CREATE_SOCKET;
    }
    else{

    // Assign a name to the socket
    int retSock = bind(mSocket, )



    mInitialized = true;
    }
}
