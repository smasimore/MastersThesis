/**
 * Class to interface with low level Linux networking 
 * 
 */

# ifndef UDP_SERVER_HPP
# define UDP_SERVER_HPP

#include <stdint.h>
#include <memory>
#include <vector>

#include "Errors.h"


class UDPServer
{
public:

    /**
     * Create a UDPServer
     *
     * @param   pServerRet      reference to a smart pointer to a UDPServer
     *                          object. Upon completion, this pointer will
     *                          point to the newly created UDPServer.
     * @param   kPort           Network port on which this server will operate
     * @param   kBlocking       Determines whether send/recv operations will
     *                          block or complete immediately
     *
     * @ret     E_SUCCESS                       Create new UDPServer succeeded
     *          E_FAILED_TO_ALLOCATE_SOCKET     UDPServer object was not created
     *          [other]                         passed from UDPServer
     *                                          constructor
     *
     **/
    static Error_t createNew(std::shared_ptr<UDPServer>& pServerRet,
                            uint16_t kPort, bool kBlocking);

    /**
     * Send a message to a specified client
     *
     * @param   kBuf            Buffer of bytes to be sent. Must have at least
     *                          one element.
     * @param   kLen            Number of bytes to send. The first kLen bytes
     *                          in kBuf will be sent. If kLen is larger than
     *                          kBuf.size(), an error will be returned.
     * @param   kDstIPAddr      Destination IP address
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
    Error_t send(std::vector<uint8_t> kBuf, int kLen, uint32_t kDstIPAddr);

    /**
     * Receive a message from a client
     *
     * @param   kBuf            Buffer where received bytes will be stored
     * @param   lenRet          Number of bytes received will be stored here
     * @param   srcIPAddrRet    The IP address of the sending client will be
     *                          stored here
     * @param   kPeek           Peek at contents but don't remove them from the
     *                          network buffer. If true, received bytes will be
     *                          copied to kBuf and can also be read again with
     *                          a subsequent call to recv().
     *
     * @ret     E_SUCCESS                       Receive succeeded and lenRet
     *                                          bytes were received.
     *          E_SOCKET_NOT_INITIALIZED        createNew() has not been called
     *                                          or failed when it was called
     *          E_INVALID_BUF_LEN               kBuf has a size less than 1
     *          E_FAILED_TO_RECV_DATA           Receive failed
     *          E_RECV_TRUNC                    kBuf is too small to hold the
     *                                          message that was received. The
     *                                          first kBuf.size() bytes were
     *                                          copied to kBuf and the rest were
     *                                          discarded.
     *          E_INVALID_SRC_ADDR              Failed to store the client's
     *                                          IP address in srcIPAddrRet. This
     *                                          may happen when receiving from a
     *                                          device with an ipv6 address.
     *          E_CLIENT_SHUTDOWN               Client has shutdown cleanly and
     *                                          no more data is available.
     *
     **/
    Error_t recv(std::vector<uint8_t> kBuf, int& lenRet, uint32_t& srcIPAddrRet,
                 bool kPeek);


private:
    /**
     *
     *
     * @ret      E_SUCCESS                      Socket object successfully
     *                                          created and bound to port
     *           E_FAILED_TO_CREATE_SOCKET      Internal linux socket object was
     *                                          not created
     *           E_FAILED_TO_BIND_TO_SOCKET     Socket was not bound to the
     *                                          specified port
     **/
    UDPServer(Error_t& ret, uint16_t kPort, bool kBlocking);

    static const int DOMAIN;
    static const int TYPE;
    static const int PROTOCOL;

    bool mInitialized;
    uint16_t mPort;
    int mSocket;
    bool mBlocking;
};



# endif // UDP_SERVER_HPP
