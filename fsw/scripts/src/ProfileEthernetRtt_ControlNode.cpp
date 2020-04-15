/**
 * See ProfileEthernetRtt_Config.hpp for instructions on running test.
 */

#include <iostream>
#include <algorithm>
#include <unistd.h>

#include "ThreadManager.hpp"
#include "ScriptHelpers.hpp"
#include "ProfileEthernetRtt_ControlNode.hpp"

/**
 * If a serial or parallel measurement takes over this time, filter it out as it
 * is a spike. See ProfileEthernetRtt_Config.hpp for rationale.
 */
static const Time::TimeNs_t MAX_ELAPSED_NS = 10 * Time::NS_IN_MS;

/**
 * Global pointer to Network Manager.
 */
static std::shared_ptr<NetworkManager> gPNm = nullptr;

/***************************** PRINT FUNCTIONS ********************************/

/**
 * If the elapsed time is over MAX_ELAPSED_MS, print granular timeline of what
 * occurred during communications.
 *
 * @param  kElapsedNs    Round trip time for network comms.
 * @param  kReg0RecvBuf  DN0's response buffer.
 * @param  kStartNs      Timestamp of round trip start.
 * @param  kSent0Ns      Timestamp of when CN finished sending buffer to DN0.
 * @param  kRecvd0Ns     Timestamp of when CN finished receiving buffer from 
 *                       DN0.
 */
static void printDebug (Time::TimeNs_t kElapsedNs,
                        std::vector<uint8_t>& kReg0RecvBuf,
                        Time::TimeNs_t& kStartNs, 
                        Time::TimeNs_t& kSent0Ns, 
                        Time::TimeNs_t& kRecvd0Ns)
{
    static const uint8_t MAX_ELAPSED_MS  = 2;
    static bool headerPrinted            = false;
    static bool print                    = false;
    static Time::TimeNs_t lastElapsedNs  = 0;
    static Time::TimeNs_t lastStartNs    = 0;
    static Time::TimeNs_t lastSentNs     = 0;
    static Time::TimeNs_t lastRecvdNs    = 0;
    static Time::TimeNs_t lastDn0RecvdNs = 0;

    if (headerPrinted == false)
    {
        std::cout << "elapsed,start,cn_sent,dn_recvd,dn_sent,cn_recvd" 
            << std::endl; 
        headerPrinted = true;
    }

    // Device Nodes send the time it took to send a response in the buffer of 
    // the NEXT run (since the act of sending the previous buffer is what was
    // being measured). If previous run took over MAX_ELAPSED_MS, print the 
    // granular timeline data.
    if (print == true)
    {
        // Get send timestamp from Device Node.
        Time::TimeNs_t dn0PrevSentNs = 0;
        std::memcpy (&dn0PrevSentNs, &kReg0RecvBuf[0], 
                     sizeof (Time::TimeNs_t));
        std::cout << lastElapsedNs << "," << "0," << lastSentNs - lastStartNs
            << "," << lastDn0RecvdNs - lastSentNs << ","
            << dn0PrevSentNs - lastSentNs << "," << lastRecvdNs - lastStartNs
            << std::endl;

        print = false;
    }

    // If run took over MAX_ELAPSED_MS, save timeline details to be printed next
    // run.
    if (kElapsedNs > MAX_ELAPSED_MS * Time::NS_IN_MS)
    {
        print = true;

        lastElapsedNs = kElapsedNs;
        lastStartNs   = kStartNs;
        lastSentNs    = kSent0Ns;
        lastRecvdNs   = kRecvd0Ns;

        // Get Device Node recv timestamps.
        std::memcpy (&lastDn0RecvdNs,
                     &kReg0RecvBuf[sizeof (Time::TimeNs_t)], 
                     sizeof (Time::TimeNs_t));
    }
}

/**************************** MEASURE FUNCTIONS *******************************/

/**
 * Measure RTT for buffer to be sent from CN --> DN0 and for DN0 to respond with
 * a buffer to CN. Print granular timeline if over MAX_ELAPSED_MS.
 *
 * @param  kReg0SendBuf  Buffer to send to DN0.
 * @param  kReg0RecvBuf  Buffer to store DN0's response.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureCommsTimeDebug (std::vector<uint8_t>& kReg0SendBuf,
                                             std::vector<uint8_t>& kReg0RecvBuf)
{
    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Send "Region" to DN0 and wait for response "Region".
    Errors::exitOnError (gPNm->send (NODE_DEVICE0, kReg0SendBuf), "Sent err");
    Time::TimeNs_t sent0Ns = ProfileHelpers::getTimeNs ();
    Errors::exitOnError (gPNm->recvBlock (NODE_DEVICE0, kReg0RecvBuf), 
                         "Rx err");
    Time::TimeNs_t recvd0Ns = ProfileHelpers::getTimeNs ();

    // Calculate elapsed and print debug info.
    Time::TimeNs_t elapsed = recvd0Ns - startNs;
    printDebug (elapsed, kReg0RecvBuf, startNs, sent0Ns, recvd0Ns);

    return elapsed;
}

/**
 * Measure RTT for full flight network comms using a parallel implementation. 
 * All buffers sent from CN and then CN calls recvMult to receive in parallel.
 *
 * @param  kReg0SendBuf  Buffer to send to DN0.
 * @param  kReg1SendBuf  Buffer to send to DN1.
 * @param  kReg2SendBuf  Buffer to send to DN2.
 * @param  kRegRecvBufs  Vector of buffers to store DN responses.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureCommsTimeParallel (
                                std::vector<uint8_t>& kReg0SendBuf,
                                std::vector<uint8_t>& kReg1SendBuf,
                                std::vector<uint8_t>& kReg2SendBuf,
                                std::vector<std::vector<uint8_t>>& kRegRecvBufs,
                                std::vector<uint8_t>& kDvBuf)
{
    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Send "Data Vector" to Ground and "Regions" to Device Nodes.
    Errors::exitOnError (gPNm->send (NODE_GROUND,  kDvBuf),       "Send err");
    Errors::exitOnError (gPNm->send (NODE_DEVICE0, kReg0SendBuf), "Send err");
    Errors::exitOnError (gPNm->send (NODE_DEVICE1, kReg1SendBuf), "Send err");
    Errors::exitOnError (gPNm->send (NODE_DEVICE2, kReg2SendBuf), "Send err");

    std::vector<uint32_t> recvdMsgs (3);
    Errors::exitOnError (gPNm->recvMult (
                                     NetworkManager::MAX_TIMEOUT_NS,
                                     {NODE_DEVICE0, NODE_DEVICE1, NODE_DEVICE2},
                                     kRegRecvBufs, recvdMsgs),
                         "recvMult err");

    Time::TimeNs_t endNs = ProfileHelpers::getTimeNs ();

    return endNs - startNs;
}

/**
 * Measure RTT for full flight network comms using a serial implementation. 
 * After each buffer sent from CN to a DN, CN waits for response.
 *
 * @param  kReg0SendBuf  Buffer to send to DN0.
 * @param  kReg1SendBuf  Buffer to send to DN1.
 * @param  kReg2SendBuf  Buffer to send to DN2.
 * @param  kReg0RecvBuf  Buffer to store DN0 response.
 * @param  kReg1RecvBuf  Buffer to store DN1 response.
 * @param  kReg2RecvBuf  Buffer to store DN2 response.
 * @param  kDvBuf        Buffer to send to Ground.
 *
 * @ret    Elapsed time in nanoseconds.
 */
static Time::TimeNs_t measureCommsTimeSerial (
                                        std::vector<uint8_t>& kReg0SendBuf,
                                        std::vector<uint8_t>& kReg1SendBuf,
                                        std::vector<uint8_t>& kReg2SendBuf,
                                        std::vector<uint8_t>& kReg0RecvBuf,
                                        std::vector<uint8_t>& kReg1RecvBuf,
                                        std::vector<uint8_t>& kReg2RecvBuf,
                                        std::vector<uint8_t>& kDvBuf)
{
    // Start time.
    Time::TimeNs_t startNs = ProfileHelpers::getTimeNs ();

    // Send "Data Vector" to Ground. No response expected.
    Errors::exitOnError (gPNm->send (NODE_GROUND,  kDvBuf),       "Send err");

    // Send "Region" to DN0 and wait for response "Region".
    Errors::exitOnError (gPNm->send (NODE_DEVICE0, kReg0SendBuf), "Send err");
    Errors::exitOnError (gPNm->recvBlock (NODE_DEVICE0, kReg0RecvBuf), 
			             "Rx err");

    // Send "Region" to DN1 and wait for response "Region".
    Errors::exitOnError (gPNm->send (NODE_DEVICE1, kReg1SendBuf), "Send err");
    Errors::exitOnError (gPNm->recvBlock (NODE_DEVICE1, kReg1RecvBuf), 
			             "Rx err");

    // Send "Region" to DN2 and wait for response "Region".
    Errors::exitOnError (gPNm->send (NODE_DEVICE2, kReg2SendBuf), "Send err");
    Errors::exitOnError (gPNm->recvBlock (NODE_DEVICE2, kReg2RecvBuf), 
			             "Rx err");
    Time::TimeNs_t recvd2Ns = ProfileHelpers::getTimeNs ();

    Time::TimeNs_t elapsed = recvd2Ns - startNs;

    return elapsed;
}

/*********************************** MAIN *************************************/

void ProfileEthernetRtt_ControlNode::main (int, char**)
{
    // 1) Init thread scheduling and priority.
    ProfileHelpers::setThreadPriAndAffinity ();

    // 2) Init Data Vector.
    std::shared_ptr<DataVector> pDv = nullptr;
    Errors::exitOnError (DataVector::createNew (
                             ProfileEthernetRtt_Config::mDvConfig,
                             pDv), "DV init");

    // 3) Init Network Manager.
    NetworkManager::Config_t nmConfig = 
    {
        ProfileEthernetRtt_Config::mNodes,
        ProfileEthernetRtt_Config::mChannels,
        NODE_CONTROL,
        DV_ELEM_TEST0,
        DV_ELEM_TEST1,
    };
    Errors::exitOnError (NetworkManager::createNew (nmConfig, pDv, gPNm), 
                         "NM init");

    // 4) Synchronize clocks to be able to compare flight computer timestamps.
    Errors::exitOnError (ClockSync::syncServer (gPNm, {NODE_DEVICE0, 
                                                       NODE_DEVICE1, 
                                                       NODE_DEVICE2}),
                         "ClockSync");

    // 5) Init buffers.
    static std::vector<uint8_t> reg0SendBuf (REGION_SIZE_BYTES);
    static std::vector<uint8_t> reg1SendBuf (REGION_SIZE_BYTES);
    static std::vector<uint8_t> reg2SendBuf (REGION_SIZE_BYTES);
    static std::vector<uint8_t> dvBuf       (REGION_SIZE_BYTES * 7);
    static std::vector<std::vector<uint8_t>> regRecvBufs (3);
    regRecvBufs[0].resize (REGION_SIZE_BYTES);
    regRecvBufs[1].resize (REGION_SIZE_BYTES);
    regRecvBufs[2].resize (REGION_SIZE_BYTES);

    // 6) Randomly fill buffers.
    std::generate (reg0SendBuf.begin(), reg0SendBuf.end(), 
                   []() {return rand() % 100;});
    std::generate (reg1SendBuf.begin(), reg1SendBuf.end(), 
                   []() {return rand() % 100;});
    std::generate (reg2SendBuf.begin(), reg2SendBuf.end(), 
                   []() {return rand() % 100;});
    std::generate (regRecvBufs[0].begin(), regRecvBufs[0].end(), 
                   []() {return rand() % 100;});
    std::generate (regRecvBufs[1].begin(), regRecvBufs[1].end(), 
                   []() {return rand() % 100;});
    std::generate (regRecvBufs[2].begin(), regRecvBufs[2].end(), 
                   []() {return rand() % 100;});

    std::cout << "------ Results ------" << std::endl;
    std::cout << "Region Size: " << REGION_SIZE_BYTES << std::endl;
    std::cout << "# of Debug Runs: " << NUM_DEBUG_RUNS << std::endl;
    std::cout << "# of Parallel Runs: " << NUM_PARALLEL_RUNS << std::endl;
    std::cout << "# of Serial Runs: " << NUM_SERIAL_RUNS << std::endl;
    std::cout << "# of Stress Parallel Runs: " << NUM_STRESS_PARALLEL_RUNS 
        << std::endl;
    std::cout << "# of Stress Serial Runs: " << NUM_STRESS_SERIAL_RUNS 
        << std::endl;

    // 7) Init result buffers.
    static std::vector<Time::TimeNs_t> resultsDebBuf  (NUM_DEBUG_RUNS,    0);
    static std::vector<Time::TimeNs_t> resultsParBuf  (NUM_PARALLEL_RUNS, 0);
    static std::vector<Time::TimeNs_t> resultsSerBuf  (NUM_SERIAL_RUNS,   0);

    // 8) Run Debug configuration.
    std::string rttMsg;
    if (NUM_DEBUG_RUNS > 0)
    {
        for (uint32_t i = 0; i < NUM_DEBUG_RUNS; i++)
        {
            resultsDebBuf[i] = measureCommsTimeDebug (reg0SendBuf,
                                                      regRecvBufs[0]);
        }
        rttMsg = "Debug Mode";
        ProfileHelpers::printVectorStats (resultsDebBuf,  rttMsg);
    }

    // 9) Run Parallel configuration. Filter out spikes.
    if (NUM_PARALLEL_RUNS > 0)
    {
        for (uint32_t i = 0; i < NUM_PARALLEL_RUNS; i++)
        {
            do
            {
                resultsParBuf[i]  = measureCommsTimeParallel (reg0SendBuf,
                                                              reg1SendBuf,
                                                              reg2SendBuf,
                                                              regRecvBufs,
                                                              dvBuf);
            }
            while (resultsParBuf[i] > MAX_ELAPSED_NS);
        }
        rttMsg = "Parallel Configuration";
        ProfileHelpers::printVectorStats (resultsParBuf,  rttMsg);
    }

    // 10) Run Serial configuration.
    if (NUM_SERIAL_RUNS > 0)
    {
        for (uint32_t i = 0; i < NUM_SERIAL_RUNS; i++)
        {
            do
            {
                resultsSerBuf[i]  = measureCommsTimeSerial (reg0SendBuf,
                                                            reg1SendBuf,
                                                            reg2SendBuf,
                                                            regRecvBufs[0],
                                                            regRecvBufs[1],
                                                            regRecvBufs[2],
                                                            dvBuf);
            }
            while (resultsParBuf[i] > MAX_ELAPSED_NS);
        }
        rttMsg = "\nSerial Configuration";
        ProfileHelpers::printVectorStats (resultsSerBuf,  rttMsg);
    }

    // 11) Run stress testing of Parallel configuration.
    uint32_t numOver2ms = 0;
    uint32_t numOver100ms = 0;
    uint32_t numOver1000ms = 0;
    if (NUM_STRESS_PARALLEL_RUNS > 0)
    {
        std::cout << "\nStress Parallel Configuration" << std::endl;
        for (uint32_t i = 0; i < NUM_STRESS_PARALLEL_RUNS; i++)
        {
            Time::TimeNs_t elapsed = measureCommsTimeParallel (reg0SendBuf,
                                                               reg1SendBuf,
                                                               reg2SendBuf,
                                                               regRecvBufs,
                                                               dvBuf);
            if (elapsed > 2 * Time::NS_IN_MS)
            {
                std::cout << "Run: " << i << " Elapsed: " << elapsed <<
                    std::endl;
                numOver2ms++;
            }
            if (elapsed > 100 * Time::NS_IN_MS)
            {
                numOver100ms++;
            }
            if (elapsed > 1000 * Time::NS_IN_MS)
            {
                numOver1000ms++;
            }
        }
        std::cout << "Num Over 2ms:    " << numOver2ms << std::endl;
        std::cout << "Num Over 100ms:  " << numOver100ms << std::endl;
        std::cout << "Num Over 1000ms: " << numOver1000ms << std::endl;
    }

    // 12) Run stress testing of Serial configuration.
    if (NUM_STRESS_SERIAL_RUNS > 0)
    {
        numOver2ms = 0;
        numOver100ms = 0;
        numOver1000ms = 0;
        std::cout << "\nStress Serial Configuration" << std::endl;
        for (uint32_t i = 0; i < NUM_STRESS_SERIAL_RUNS; i++)
        {
            Time::TimeNs_t elapsed = measureCommsTimeSerial (reg0SendBuf,
                                                             reg1SendBuf,
                                                             reg2SendBuf,
                                                             regRecvBufs[0],
                                                             regRecvBufs[1],
                                                             regRecvBufs[2],
                                                             dvBuf);
            if (elapsed > 2 * Time::NS_IN_MS)
            {
                std::cout << "Run: " << i << " Elapsed: " << elapsed <<
                    std::endl;
                numOver2ms++;
            }
            if (elapsed > 100 * Time::NS_IN_MS)
            {
                numOver100ms++;
            }
            if (elapsed > 1000 * Time::NS_IN_MS)
            {
                numOver1000ms++;
            }
        }
        std::cout << "Num Over 2ms:    " << numOver2ms << std::endl;
        std::cout << "Num Over 100ms:  " << numOver100ms << std::endl;
        std::cout << "Num Over 1000ms: " << numOver1000ms << std::endl;
    }
}
