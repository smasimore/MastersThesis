/**
 * Functions used to synchronize the flight computer clocks. Uses ntp instead
 * of PTP (more accurate than ntp) due to PTP hardware requirements. The 
 * serverSync function is intended to run on the Control Node and the 
 * syncClient function is intended to run on the Device Nodes during 
 * initialization of the flight software. Protocol:
 *
 * 1) Server starts ntp daemon.
 * 2) Server sends SERVER_READY msg to each client.
 * 3) Server blocks on receiving response from each client.
 * 4) Clients block on receiving SERVER_READY msg from server.
 * 5) After received, client runs ntpdate to sync with server once.
 * 6) Client sends CLIENT_SYNC_SUCCESS or CLIENT_SYNC_FAIL response to server 
 *    based on result of ntpdate call.
 * 7) Server receives client responses. If all succeeded to sync, 
 *    synchronization was successful. Otherwise failed.
 * 8) Server stops ntp daemon.
 *
 * NOTES
 *
 *     #1 The client binaries must be started before the server binary. 
 *        Otherwise, the SERVER_READY message sent from server to client will
 *        be missed.
 *     #2 Clock synchronization can take up to 10 seconds.
 *     #3 The ntp client uses the SCHED_OTHER scheduling policy and runs on CPU 
 *        0 or 1.
 *     #4 These functions only works on the sbRIO. If future implementations
 *        wish to synchronize with the ground computer, modifications to 
 *        system (x) calls will be required.
 *
 */

# ifndef CLOCK_SYNC_HPP
# define CLOCK_SYNC_HPP

#include <memory>
#include <vector>

#include "NetworkManager.hpp"

namespace ClockSync
{

    /**
     * Start the ntp daemon if it isn't already running and send a message
     * to each of the clients letting them know it is running. Wait for each
     * client to send a success/failure message to verify clocks have been
     * synchronized. Stops the ntp daemon after sync is complete.
     *
     * WARNING #1: If this function fails, the ntp daemon may continue running. 
     *             The program should exit on failure.
     * WARNING #2: This method blocks until a response messages is received from
     *             each client. 
     *
     * @param   kPNm                      Pointer to Network Manager.
     * @param   kClientNodes              List of client nodes.
     *
     * @ret     E_SUCCESS                 Successfully synchronized with 
     *                                    clients.
     *          E_NETWORK_MANAGER_NULL    Network Manager pointer null.
     *          E_NO_CLIENTS              Client list empty.
     *          E_NETWORK_MANAGER_TX_FAIL Failed to send message to clients.
     *          E_NETWORK_MANAGER_RX_FAIL Failed to receive messages from 
     *                                    clients.
     *          E_RX_AND_NTPD_FAIL        Failed to rx msg and kill ntpd.                          
     *          E_CLIENT_FAILED_TO_SYNC   One or more clients failed to sync.
     *          E_SYNC_AND_NTPD_FAIL      One or more clients failed to sync
     *                                    and failed to kill ntpd.
     *          E_FAILED_TO_START_NTPD    Failed to start ntp daemon.
     *          E_FAILED_TO_STOP_NTPD     Failed to stop ntp daemon.
     */
    Error_t syncServer (std::shared_ptr<NetworkManager>& kPNm, 
                        std::vector<Node_t> kClientNodes);

    /**
     * Wait for a message from the server node that it is ready, then sync to
     * the server and send a success or fail message back to the server.
     *
     * WARNING: This method blocks until a message is received from the server.
     *
     * @param   kPNm                               Pointer to Network Manager.
     * @param   kServerNode                        Server node.
     * @param   kServerNodeIP                      Server node IP.
     *
     * @ret     E_SUCCESS                          Successfully synchronized 
     *                                             with server.
     *          E_NETWORK_MANAGER_NULL             Network Manager pointer null.
     *          E_INVALID_SERVER_MSG               Server message invalid.
     *          E_NETWORK_MANAGER_RX_FAIL          Failed to receive message
     *                                             from server.
     *          E_NETWORK_MANAGER_TX_FAIL          Failed to send sync success 
     *                                             message to server.
     *          E_CLIENT_FAILED_TO_SYNC            Client failed to sync.
     *          E_CLIENT_FAILED_TO_SYNC_AND_TX_MSG Client failed to sync & tx
     *                                             message to server.
     *          E_SYNCD_OFFSET_OVER_MAX            Sync'd but initial clock 
     *                                             offset between client and
     *                                             server above max allowed.
     */
    Error_t syncClient (std::shared_ptr<NetworkManager>& kPNm, 
                        Node_t kServerNode,
                        NetworkManager::IP_t kServerNodeIp);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF CLOCKSYNC
     *
     * Messages to send between server and clients to facilitate clock 
     * synchronization.
     */
    enum Msg_t : uint8_t
    {
        SERVER_READY,
        CLIENT_SYNC_SUCCESS,
        CLIENT_SYNC_FAIL,

        LAST
    };
};

# endif
