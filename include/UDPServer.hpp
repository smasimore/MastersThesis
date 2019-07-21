/**
 * Class to interface with low level Linux networking 
 * 
 */

# ifndef UDP_SERVER_HPP
# define UDP_SERVER_HPP

#include <stdint.h>
#include <memory>

#include "Errors.h"


class UDPServer
{
public:

    static Error_t createNew(std::shared_ptr<UDPServer>& pServerRet, uint16_t kPort, bool kBlocking);


    // TODO: use std::array<uint8_t> rather than uint8_t*
    Error_t send(uint8_t* kBuf, int kLen, uint8_t* kDstIPAddr);

    // TODO: use std::array<uint8_t> rather than uint8_t*
    Error_t recv(uint8_t* kBuf, int& kLen, bool kPeek);


private:

    UDPServer(Error_t& ret, uint16_t kPort, bool kBlocking);

    static const int DOMAIN;
    static const int TYPE;
    static const int PROTOCOL;

    bool mInitialized;
    uint16_t mPort;
    int mSocket;
};



# endif // UDP_SERVER_HPP
