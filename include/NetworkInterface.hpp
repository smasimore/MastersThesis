/**
 * Class to interface with low level Linux networking 
 * 
 */

# ifndef HETWORK_INTERFACE_HPP
# define NETWORK_INTERFACE_HPP

#include "Errors.h"

class NetworkInterface 
{
public: 
    static Error_t networkInit(u16 port);


    static Error_t send(u8* buf, int len, u8* dstIPAddr, bool blocking);


    static Error_t recv(u8* buf, int len, u8* srcIPAddr, bool blocking);


private: 

    NetworkInterface();

    static bool mInitialized;
    static u16 mPort; 




}



# endif // HETWORK_INTERFACE_HPP