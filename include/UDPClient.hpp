/**
 * Class to interface with low level Linux networking 
 * 
 */

# ifndef UDP_CLIENT_HPP
# define UDP_CLIENT_HPP

#include <stdint.h>
#include <memory>

#include "Errors.h"


class UDPClient
{
public: 
    static Error_t createNew(std::shared_ptr<UDPClient>& pClientRet, uint16_t port);


    // TODO: use std::array<uint8_t> rather than uint8_t*
    Error_t send(uint8_t* buf, int len, uint8_t* dstIPAddr, bool blocking);

    // TODO: use std::array<uint8_t> rather than uint8_t*
    Error_t recv(uint8_t* buf, int len, uint8_t* srcIPAddr, bool blocking);

private:

    UDPClient(Error_t& ret, uint16_t port);

    static const int DOMAIN;
    static const int TYPE;
    static const int PROTOCOL;

    bool mInitialized;
    uint16_t mPort;
    int mSocket;
};



# endif // UDP_CLIENT_HPP
