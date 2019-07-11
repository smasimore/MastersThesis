#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <UDPClient.hpp>

/* Use Internet Protocol*/
const int UDPClient::DOMAIN = AF_INET;

/* Specifies that this will be a UDP socket*/
const int UDPClient::TYPE = SOCK_DGRAM;

/* Default */
const int UDPClient::PROTOCOL = 0;



// TODO: finish reading this http://man7.org/linux/man-pages/man7/udp.7.html

Error_t UDPClient::createNew(std::shared_ptr<UDPClient>& pClientRet, uint16_t port)
{
    Error_t ret;
    // Can't use make_shared here, because UDPClient constructor is private
    pClientRet.reset(new UDPClient(ret, port));

    if(pClientRet == nullptr){
        return E_FAILED_TO_ALLOCATE_SOCKET;
    }

    if(ret != E_SUCCESS){
        return E_FAILED_TO_CREATE_SOCKET;
    }

    return E_SUCCESS;
}

Error_t UDPClient::send(uint8_t* buf, int len, uint8_t* dstIPAddr,
                        bool blocking)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }



    return E_SUCCESS;
}

Error_t UDPClient::recv(uint8_t* buf, int len, uint8_t* srcIPAddr,
                        bool blocking)
{
    if(!mInitialized)
    {
        return E_SOCKET_NOT_INITIALIZED;
    }

    return E_SUCCESS;
}


UDPClient::UDPClient(Error_t& ret, uint16_t port)
{
    ret = E_SUCCESS;
    mInitialized = true;
}
