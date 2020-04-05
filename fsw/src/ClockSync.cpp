#include <cstring>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <cmath>

#include "ClockSync.hpp"

/**
 * Maximum starting clock offset between client and server. Value obtained
 * by synchronizing the flight network 10 times and taking 1.5 * maximum offset.
 */
static const float MAX_STARTING_OFFSET_SECONDS = 0.000040;

/**
 * Temporary file to store ntpdate output. This is used to verify offset after
 * sync is acceptable.
 */
static const std::string TMP_SYNC_FILE_PATH = "/tmp/fsw_clock_sync.output";

/**
 * Starts or stops NTP daemon. Daemon runs on CPU 1.
 *
 * @param   kEnable                  If set to true, daemon is started.
 *                                   If false, daemon is stopped.
 *
 * @ret     E_SUCCESS                Successfully set ClockSync.
 *          E_FAILED_TO_STOP_NTPD    Failed to stop NTP daemon.
 *          E_FAILED_TO_START_NTPD   Failed to start NTP daemon.
 */
static Error_t setNtpDaemonStatus (bool kEnable);

/**
 * Runs ntpdate in debug mode to read offset from server and verify it is less
 * than MAX_STARTING_OFFSET_SECONDS.
 *
 * @param   kServerNodeIP           IP of server node.
 *
 * @ret     E_SUCCESS               Offset verified to be below max.
 *          E_SYNCD_OFFSET_OVER_MAX Offset above max.
 */
static Error_t verifyClientSynced (NetworkManager::IP_t kServerNodeIP);

Error_t ClockSync::syncServer (std::shared_ptr<NetworkManager>& kPNm, 
                               std::vector<Node_t> kClientNodes)
{
    // 1) Verify params.
    if (kPNm == nullptr)
    {
        return E_NETWORK_MANAGER_NULL;
    }

    if (kClientNodes.empty () == true)
    {
        return E_NO_CLIENTS;
    }

    // 2) Start ntpd. 
    Error_t ret = setNtpDaemonStatus (true);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // 3) Send notification to clients and create rx buffers.
    std::vector<std::vector<uint8_t>> rxMsgs;
    for (Node_t client : kClientNodes)
    {
        rxMsgs.push_back ({ClockSync::Msg_t::CLIENT_SYNC_FAIL});
        std::vector<uint8_t> sendMsg = {ClockSync::Msg_t::SERVER_READY};
        ret = kPNm->send (client, sendMsg);
        if (ret != E_SUCCESS)
        {
            return E_NETWORK_MANAGER_TX_FAIL;
        }
    }

    // 4) Wait for clients to send success/fail message.
    for (uint8_t nodeIdx = 0; nodeIdx < kClientNodes.size (); nodeIdx++)
    {
        ret = kPNm->recvBlock (kClientNodes[nodeIdx], rxMsgs[nodeIdx]);
        if (ret != E_SUCCESS)
        {
            // Attempt to stop daemon.
            ret = setNtpDaemonStatus (false);
            if (ret != E_SUCCESS)
            {
                return E_RX_AND_NTPD_FAIL;
            }

            return E_NETWORK_MANAGER_RX_FAIL;
        }
    }

    // 5) Check if all clients successfully sync'd.
    for (std::vector<uint8_t> rxMsg : rxMsgs)
    {
        // Message is 1 byte long.
        std::vector<uint8_t> successMsg = 
            {ClockSync::Msg_t::CLIENT_SYNC_SUCCESS};
        if (rxMsg != successMsg)
        {
            // Attempt to stop daemon.
            ret = setNtpDaemonStatus (false);
            if (ret != E_SUCCESS)
            {
                return E_SYNC_AND_NTPD_FAIL;
            }

            return E_CLIENT_FAILED_TO_SYNC;
        }
    }

    // 6) Stop ntpd.
    ret = setNtpDaemonStatus (false);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    return E_SUCCESS;
}

Error_t ClockSync::syncClient (std::shared_ptr<NetworkManager>& kPNm, 
                               Node_t kServerNode,
                               NetworkManager::IP_t kServerNodeIP)
{
    // 1) Verify params.
    if (kPNm == nullptr)
    {
        return E_NETWORK_MANAGER_NULL;
    }

    // 2) Wait for server ready message.
    std::vector<uint8_t> rxMsg (1);
    std::vector<uint8_t> expectedRxMsg = {ClockSync::Msg_t::SERVER_READY};
    Error_t ret = kPNm->recvBlock (kServerNode, rxMsg);
    if (ret != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_RX_FAIL;
    }
    if (rxMsg != expectedRxMsg)
    {
        return E_INVALID_SERVER_MSG;
    }

    // 3) Attempt to sync to server. -b flag forces the time sync even if the
    //    offset is large.
    std::string ntpdateCmd = "ntpdate -b " + kServerNodeIP + " > /dev/null 2>&1";
    int32_t ntpdateRet = std::system (ntpdateCmd.c_str ());

    // 4) If failed to sync or post-sync offset over max, send fail status to 
    //    server.
    ret = verifyClientSynced (kServerNodeIP);
    if (ntpdateRet != 0 || ret != E_SUCCESS)
    {
        std::vector<uint8_t> txMsg = {ClockSync::Msg_t::CLIENT_SYNC_FAIL};
        ret = kPNm->send (kServerNode, txMsg);
        if (ret != E_SUCCESS)
        {
            return E_CLIENT_FAILED_TO_SYNC_AND_TX_MSG;
        }

        return E_CLIENT_FAILED_TO_SYNC;
    }

    // 5) Otherwise, send success status.
    std::vector<uint8_t> txMsg = {ClockSync::Msg_t::CLIENT_SYNC_SUCCESS};
    ret = kPNm->send (kServerNode, txMsg);
    if (ret != E_SUCCESS)
    {
        return E_NETWORK_MANAGER_TX_FAIL;
    }

    return E_SUCCESS;
}

Error_t setNtpDaemonStatus (bool kEnable)
{
    // 1) Start or stop the daemon as specified by kEnable. 
    if (kEnable == true)
    {
        // Start daemon. If it is already running, the command will fail 
        // silently without impacting the existing daemon.
        if (std::system ("/etc/init.d/ntpd start > /dev/null 2>&1") != 0)
        {
            return E_FAILED_TO_START_NTPD;
        }
    }
    else 
    {
        // Stop daemon. Ignore system call failure, as this could mean the 
        // daemon was not running, which is ok.
        std::system ("/etc/init.d/ntpd stop > /dev/null 2>&1");

        // Sleep for 1 second to allow process to receive TERM signal.
        sleep (1);
    }

    // 2) Get status of daemon. pidof returns 0 if at least 1 program found with 
    //    the requested name.
    bool daemonRunning = false;
    if (std::system ("pidof -x /usr/sbin/ntpd > /dev/null 2>&1") == 0)
    {
        daemonRunning = true;
    }

    // 3) If daemon is running and this is unexpected, return error.
    if (daemonRunning == true && kEnable == false)
    {
        return E_FAILED_TO_STOP_NTPD;
    }

    // 4) If daemon is not running and this is unexpected, return error.
    if (daemonRunning == false && kEnable == true)
    {
        return E_FAILED_TO_START_NTPD;
    }

    return E_SUCCESS;
}

Error_t verifyClientSynced (NetworkManager::IP_t kServerNodeIP)
{
    // 1) Run ntpdate in debug mode to read current offset. Write output to
    //    TMP_SYNC_FILE_PATH.
    std::string ntpdateDebugCmd = "output=$(ntpdate -d " + kServerNodeIP +
                                  " 2>&1) && echo $output | awk '{print $119}' > "
                                  + TMP_SYNC_FILE_PATH;
    int32_t ntpdateDebugRet = std::system (ntpdateDebugCmd.c_str ());

    // 2) Handle ntpdate failure.
    if (ntpdateDebugRet != 0)
    {
        return E_CLIENT_FAILED_TO_SYNC;
    }

    // 3) Read TMP_SYNC_FILE_PATH, which now contains the offset.
    std::ifstream tmpFile (TMP_SYNC_FILE_PATH, std::ifstream::in);
    std::string offsetStr;
    tmpFile >> offsetStr;

    // 4) Verify offset.
    float offsetFlt = std::stof (offsetStr);
    if (std::abs (offsetFlt) > MAX_STARTING_OFFSET_SECONDS)
    {
        return E_SYNCD_OFFSET_OVER_MAX;
    }

    return E_SUCCESS;
}
