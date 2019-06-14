#include <sys/socket.h>


bool mInitialized = false;

NetworkInterface::NetworkInterface(){}


Error_t NetworkInterface::networkInit(u16 port)
{
    mPort = port;
    mInitialized = true;

    return E_SUCCESS;
}

Error_t NetworkInterface::send(u8* buf, int len, u8* dstIPAddr, bool blocking)
{
    if(!mInitialized)
    {
        return E_NETWORK_NOT_INITIALIZED;
    }

}

Error_t NetworkInterface::recv(u8* buf, int len, u8* srcIPAddr, bool blocking)
{
    if(!mInitialized)
    {
        return E_NETWORK_NOT_INITIALIZED;
    }

}