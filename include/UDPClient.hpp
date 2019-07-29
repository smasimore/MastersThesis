/**
 * Class to interface with low level Linux networking 
 * 
 * UDPClients are used to send messages to a specified IP address and port
 *
 */

# ifndef UDP_CLIENT_HPP
# define UDP_CLIENT_HPP

#include <stdint.h>
#include <memory>
#include <vector>

#include "Errors.h"

class UDPClient
{
public: 
    ~UDPClient ();

    /**
     * Create a UDPClient
     *
     * @param   pClientRet      reference to a smart pointer to a UDPClient
     *                          object. Upon completion, this pointer will
     *                          point to the newly created UDPClient.
     * @param   kBlocking       Determines whether send/recv operations will
     *                          block or complete immediately
     *
     * @ret     E_SUCCESS                       Create new UDPClient succeeded
     *          E_FAILED_TO_ALLOCATE_SOCKET     UDPClient object was not created
     *          E_FAILED_TO_CREATE_SOCKET       Internal linux socket object was
     *                                          not created. pClientRet was not
     *                                          modified.
     **/
    static Error_t createNew (std::shared_ptr<UDPClient>& pClientRet,
                            bool kBlocking);

    /**
     * Send a message to a specified server
     *
     * @param   kBuf            Buffer of bytes to be sent. Must have at least
     *                          one element.
     * @param   kLen            Number of bytes to send. The first kLen bytes
     *                          in kBuf will be sent. If kLen is larger than
     *                          kBuf.size(), an error will be returned.
     * @param   kDstIPAddr      Destination IP address
     * @param   kDstPort        Destination port
     *
     * @ret     E_SUCCESS                       Send succeeded, and kLen bytes
     *                                          were sent
     *          E_SOCKET_NOT_INITIALIZED        createNew() has not been called
     *                                          or failed when it was called
     *          E_INVALID_BUF_LEN               kBuf has a size less than 1 or
     *                                          fewer than kLen elements
     *          E_FAILED_TO_SEND_DATA           Send failed
     *          E_PARTIAL_SEND                  Send succeeded, but less than
     *                                          kLen bytes were sent
     *
     **/
    Error_t send (std::vector<uint8_t>& kBuf, size_t kLen, uint32_t kDstIPAddr,
                 uint16_t kDstPort);

    /**
     * Close the socket
     *
     * @ret     E_SUCCESS                       Socket closed successfully
     *          E_FAILED_TO_CLOSE_SOCKET        An error occurred while closing
     *                                          the socket. This may be caused
     *                                          by a invalid socket, an
     *                                          internal I/O error, or a signal
     *                                          interrupting this function
     *
     **/
    Error_t closeSocket ();

private:

    /**
     * UDPClient Constructor
     *
     * @param   sockFD         Socket file descriptor
     *
     **/
    UDPClient (int sockFD);

    static const int DOMAIN;
    static const int TYPE;
    static const int PROTOCOL;

    bool mInitialized;
    int mSocket;
};

# endif // UDP_CLIENT_HPP
