/**
 * Measure flight network communication time for parallel and serial 
 * implementations. The purpose of this profiling script is to better understand 
 * the average and maximum amount of time it takes to complete one round of
 * flight network communications. A round is defined as:
 *
 *     1) Control Node sends entire Data Vector to Ground. Data Vector is
 *        represented as a byte buffer of size 7 x the Region size. 7 accounts 
 *        for the 7 Control Node Data Vector regions.
 *     2) Control Node sends a Data Vector Region to each of the 3 Device Nodes.
 *        A Region is represented by a byte buffer of sizes defined by the
 *        REGION_SIZE_BYTES macro in ProfileEthernetRtt_Config.
 *     3) Control Node receives a Region in response from each of the 3 Device
 *        Nodes.
 *
 * Due to a known issue with the Zynq-7000 series Gigabit Ethernet Controller,
 * there are periodic spikes in the time it takes to receive a message. This is
 * because a message can get stuck in the RX FIFO queue, and will only get
 * unstuck when another Ethernet frame comes in. In our flight software, the
 * Control Node will timeout when this happens and the stuck message will come
 * in the next loop. In these profiling scripts, we filter out these spikes as
 * the purpose of this script is to measure the time it takes for the serial vs.
 * parallel comms to complete to inform the Control Node timeout constants.
 *
 * The experiment has 5 different configurations, which can be turned off by 
 * setting their NUM_x_RUNS macro to 0. Configurations 2-5 can be run at the
 * same time, and configuration 1 must be run alone.
 *
 *     #1 NUM_DEBUG_RUNS: Measures time it takes for Control Node to send a 
 *        Region to Device Node 0 and for DN0 to respond. If takes over 2ms,
 *        prints out a detailed timeline to be able to investigate where the
 *        delay occurred.
 *
 *     #2 NUM_PARALLEL_RUNS: Measures time to do a complete round of flight
 *        network communications, with all sends occurring first and then a call
 *        to recvMult to receive the responses. Spikes are filtered out as the
 *        purpose of this configuration is to measure the time it takes for the 
 *        nominal parallel comms to take in comparing the serial vs. parallel 
 *        implementations. Max 10,000 runs. 
 *
 *     #3 NUM_SERIAL_RUNS: Measures time to do a complete round of flight
 *        network communications, with each recv call occurring right after the 
 *        relevant send call (besides to Ground, which expects no response). 
 *        Spikes are filtered out as the purpose of this configuration is to 
 *        measure the time it takes for the nominal serial comms to take in 
 *        comparing the serial vs. parallel implementations. Max 10,000 runs. 
 *
 *     #4 NUM_STRESS_PARALLEL_RUNS: Stress testing of the parallel design. 
 *        Because each run time is not stored like in #2, can run the complete
 *        round of flight comms using the parallel design any number of times
 *        (e.g. 1 million runs). Counts number of times the round-trip time 
 *        (RTT) is over 2ms, 100ms, and 1000ms.
 *
 *     #5 NUM_STRESS_SERIAL_RUNS: Stress testing of the serial design. Because 
 *        each run time is not stored like in #3, can run the complete round of 
 *        flight comms using the serial design any number of times (e.g. 1 
 *        million runs). Counts number of times the round-trip time (RTT) is 
 *        over 2ms, 100ms, and 1000ms.
 *
 * NOTES
 *
 *     #1 Clock synchronization is done so that timestamps collected on 
 *        different nodes can be compared (with error up to +/- 100us). If the
 *        clocks haven't been synchronized in a while, this step can fail. 
 *        Rerun script to resolve.
 *
 *                       ---- HARDWARE SETUP ---- 
 * 
 * 1) Connect four sbRIO's to the switch.
 * 2) Set the IP and REGION_SIZE_BYTES macros.
 * 3) Compile Control Node binary.
 * 4) Compile Device Node binaries, with relevant device node set in 
 *    DEVICE_NODE_TO_COMPILE macro.
 * 5) Start Device Node binaries.
 * 6) Start Control Node binary.
 *
 */

#ifndef PROFILE_ETHERNET_RTT_CONFIG_HPP
#define PROFILE_ETHERNET_RTT_CONFIG_HPP

#include "NetworkManager.hpp"
#include "DataVector.hpp"
#include "ProfileHelpers.hpp"
#include "ScriptHelpers.hpp"
#include "ClockSync.hpp"

#define DEVICE_NODE_TO_COMPILE   NODE_DEVICE0
#define NUM_DEBUG_RUNS           0
#define NUM_PARALLEL_RUNS        10000
#define NUM_SERIAL_RUNS          10000
#define NUM_STRESS_PARALLEL_RUNS 0
#define NUM_STRESS_SERIAL_RUNS   0
#define REGION_SIZE_BYTES        1024
#define DEVICE_NODE0_IP          "10.0.0.1"
#define DEVICE_NODE1_IP          "10.0.0.2"
#define DEVICE_NODE2_IP          "10.0.0.3"
#define CONTROL_NODE_IP          "10.0.0.4"
#define GROUND_NODE_IP           "10.0.0.99"

namespace ProfileEthernetRtt_Config
{

    /**
     * Generic Data Vector config to satisfy Network Manager init.
     */
    extern DataVector::Config_t mDvConfig;

    /**
     * Nodes used to initialize Network Manager.
     */
    extern std::unordered_map<Node_t, 
                              NetworkManager::IP_t, 
                              EnumClassHash> mNodes;

    /**
     * Channels used to initialize Network Manager.
     */
    extern std::vector<NetworkManager::ChannelConfig_t> mChannels;
}

# endif
