/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "Time.hpp"
#include "ControlNode.hpp"
#include "ProfilePlatformComms_ControlNode.hpp"

/******************************* CONTROLLERS **********************************/

/**
 * Controller to manage comms test. Checks if # of Control Node loops has hit 
 * NUM_RUNS. If yes, print message miss rate, out-of-order rate, and # of 
 * dropped messages.
 */
class ProfileCommsController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Manage comms test.
         */
        Error_t runEnabled ()
        {
            // 1) Read current Control Node loop count.
            uint32_t numCnLoops = 0;
            if (mPDataVector->read (DV_ELEM_CN_LOOP_COUNT, numCnLoops) 
                    != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }

            // 2) Read message miss counters and print if we missed a message.
            static uint32_t prevNumDn0RxMisses = 0;
            static uint32_t prevNumDn1RxMisses = 0;
            static uint32_t prevNumDn2RxMisses = 0;
            uint32_t numDn0RxMisses = 0;
            uint32_t numDn1RxMisses = 0;
            uint32_t numDn2RxMisses = 0;
            if (mPDataVector->read (DV_ELEM_DN0_RX_MISS_COUNT, 
                                    numDn0RxMisses) != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_DN1_RX_MISS_COUNT, 
                                    numDn1RxMisses) != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_DN2_RX_MISS_COUNT, 
                                    numDn2RxMisses) != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }
            if (numDn0RxMisses > prevNumDn0RxMisses)
            {
                std::cout << "DN0 Rx Miss: run " << numCnLoops << std::endl;
                prevNumDn0RxMisses = numDn0RxMisses;
            }
            if (numDn1RxMisses > prevNumDn1RxMisses)
            {
                std::cout << "DN1 Rx Miss: run " << numCnLoops << std::endl;
                prevNumDn1RxMisses = numDn1RxMisses;
            }
            if (numDn2RxMisses > prevNumDn2RxMisses)
            {
                std::cout << "DN2 Rx Miss: run " << numCnLoops << std::endl;
                prevNumDn2RxMisses = numDn2RxMisses;
            }

            // 3) Read # of msgs rx'd to determine if we received more than the
            //    expected 3 this loop (1 per DN). After each miss, the next run
            //    we expect to receive an additional msg. Initialize to 3 to 
            //    account for the 3 clock sync messages.
            static uint32_t prevNumRxMsgs = 3;
            uint32_t numRxMsgs = 0;
            if (mPDataVector->read (DV_ELEM_CN_MSG_RX_COUNT, numRxMsgs) 
                    != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }
            if (numRxMsgs - prevNumRxMsgs > 3)
            {
                std::cout << "More than 3 (" << numRxMsgs - prevNumRxMsgs << ")"
                    << " msgs recvd: run " << numCnLoops << std::endl;
            }
            prevNumRxMsgs = numRxMsgs;

            // 4) Read loop counters to determine if we received an out-of-order
            //    message.
            static uint32_t numOutOfOrderMsgs = 0;
            static uint32_t prevDn0LoopCnt = 0;
            static uint32_t prevDn1LoopCnt = 0;
            static uint32_t prevDn2LoopCnt = 0;
            uint32_t dn0LoopCnt = 0;
            uint32_t dn1LoopCnt = 0;
            uint32_t dn2LoopCnt = 0;
            if (mPDataVector->read (DV_ELEM_DN0_LOOP_COUNT, dn0LoopCnt) 
                    != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_DN1_LOOP_COUNT, dn1LoopCnt) 
                    != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_DN2_LOOP_COUNT, dn2LoopCnt) 
                    != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }
            if (dn0LoopCnt < prevDn0LoopCnt)
            {
                std::cout << "DN0 loop cnt decreased: run " << numCnLoops << 
                    std::endl;
                prevDn0LoopCnt = dn0LoopCnt;
                numOutOfOrderMsgs++;
            }
            if (dn1LoopCnt < prevDn1LoopCnt)
            {
                std::cout << "DN1 loop cnt decreased: run " << numCnLoops << 
                    std::endl;
                prevDn1LoopCnt = dn1LoopCnt;
                numOutOfOrderMsgs++;
            }
            if (dn2LoopCnt < prevDn2LoopCnt)
            {
                std::cout << "DN2 loop cnt decreased: run " << numCnLoops << 
                    std::endl;
                prevDn2LoopCnt = dn2LoopCnt;
                numOutOfOrderMsgs++;
            }

            // 5) Print stats every 100,000 loops. If have run NUM_RUNS times, 
            //    print test results and exit.
            if (numCnLoops % 100000 == 0 || numCnLoops == NUM_RUNS - 1)
            {
                uint32_t numTxMsgs = 0;
                uint32_t numCommsDeadlineMisses = 0;
                uint32_t numLoopDeadlineMisses = 0;
                uint32_t numDn0TxMsgs = 0;
                uint32_t numDn0RxMsgs = 0;
                uint32_t numDn1TxMsgs = 0;
                uint32_t numDn1RxMsgs = 0;
                uint32_t numDn2TxMsgs = 0;
                uint32_t numDn2RxMsgs = 0;
                if (mPDataVector->read (DV_ELEM_CN_MSG_TX_COUNT, numTxMsgs) 
                        != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_CN_COMMS_DEADLINE_MISS_COUNT, 
                                        numCommsDeadlineMisses) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_CN_LOOP_DEADLINE_MISS_COUNT, 
                                        numLoopDeadlineMisses) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_DN0_MSG_TX_COUNT, 
                                        numDn0TxMsgs) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_DN0_MSG_RX_COUNT, 
                                        numDn0RxMsgs) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_DN1_MSG_TX_COUNT, 
                                        numDn1TxMsgs) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_DN1_MSG_RX_COUNT, 
                                        numDn1RxMsgs) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_DN2_MSG_TX_COUNT, 
                                        numDn2TxMsgs) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_DN2_MSG_RX_COUNT, 
                                        numDn2RxMsgs) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_DATA_VECTOR_READ);
                }

                // Calculate msg missed rate.
                uint32_t numMissedMsgs = numDn0RxMisses + numDn1RxMisses + 
                                         numDn2RxMisses;
                float msgMissRate = numMissedMsgs / (float) numRxMsgs;

                // Calculated msg received out-of-order rate.
                float msgOutOfOrderRate = numOutOfOrderMsgs / (float) numRxMsgs;

                // Calculate msg drop rate. (NUM_RUNS + 1) * 3 is the number of 
                // messages we would expect to receive (3 clock sync msgs + 3 
                // Device Node msgs per loop) if there were no skips or drops.
                uint32_t numExpectedMsgs = ((NUM_RUNS + 1) * 3);
                float msgDropRate = (numExpectedMsgs - numRxMsgs) / 
                                    (float) numRxMsgs;
                    
                // Print raw data.
                std::cout << "---- Raw Data ----" << std::endl;
                std::cout << "# TX Msgs: " << numTxMsgs << std::endl;
                std::cout << "# RX Msgs: " << numRxMsgs << std::endl;
                std::cout << "# Comms Deadline Misses: " << 
                    numCommsDeadlineMisses << std::endl;
                std::cout << "# Loop Deadline Misses: " << numLoopDeadlineMisses 
                    << std::endl;
                std::cout << "# DN0 RX Misses: " << numDn0RxMisses << std::endl;
                std::cout << "# DN1 RX Misses: " << numDn1RxMisses << std::endl;
                std::cout << "# DN2 RX Misses: " << numDn2RxMisses << std::endl;
                std::cout << "# Out of Order Msgs: " << numOutOfOrderMsgs << 
                    std::endl;
                std::cout << "# CN Loops: " << numCnLoops << std::endl;
                std::cout << "# DN0 Loops: " << dn0LoopCnt << std::endl;
                std::cout << "# DN1 Loops: " << dn1LoopCnt << std::endl;
                std::cout << "# DN2 Loops: " << dn2LoopCnt << std::endl;
                std::cout << "# DN0 TX Msgs: " << numDn0TxMsgs << std::endl;
                std::cout << "# DN0 RX Msgs: " << numDn0RxMsgs << std::endl;
                std::cout << "# DN1 TX Msgs: " << numDn1TxMsgs << std::endl;
                std::cout << "# DN1 RX Msgs: " << numDn1RxMsgs << std::endl;
                std::cout << "# DN2 TX Msgs: " << numDn2TxMsgs << std::endl;
                std::cout << "# DN2 RX Msgs: " << numDn2RxMsgs << std::endl;

                if (numCnLoops == NUM_RUNS - 1)
                {
                    // Print rates.
                    std::cout << "---- Test Results ----" << std::endl;
                    std::cout << "Msg Miss Rate: " << msgMissRate << std::endl;
                    std::cout << "Msg Out-Of-Order Rate: " << msgOutOfOrderRate 
                        << std::endl;
                    std::cout << "Msg Drop Rate: " << msgDropRate << std::endl;

                    // Exit.
                    pthread_exit ((void*) E_SUCCESS);
                }
            }

            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t runSafed ()
        {
            return E_SUCCESS;
        }

        /**
         * Do nothing.
         */
        Error_t verifyConfig ()
        {
            return E_SUCCESS;
        }

        /**
         * Constructor.
         */
        ProfileCommsController (Config_t kConfig,
                                std::shared_ptr<DataVector> kPDv,
                                DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem),
            mConfig (kConfig) {}
    
    private:

        /**
         * Unused.
         */
        Config_t mConfig;
};

/********************************* CONFIGS ************************************/

/**
 * Controller initialization function.
 */
static Error_t initializeControllers (
                      std::shared_ptr<DataVector> kPDv,
                      std::vector<std::unique_ptr<Controller>>& kPCtlrsVecRet)
{
    std::unique_ptr<ProfileCommsController> pCtrlr;
    ProfileCommsController::Config_t ctrlConfig = {};
    Error_t ret = Controller::createNew (ctrlConfig, kPDv, 
                                         DV_ELEM_COMMS_CTRL_MODE, 
                                         pCtrlr);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Add controller to return vector. std::move necessary since controller
    // stored in unique_ptr. Move transfers ownership of the object.
    kPCtlrsVecRet.push_back (std::move (pCtrlr));

    return E_SUCCESS;
}

/**
 * State Machine config. 
 */
static StateMachine::Config_t gSmConfig =
{
    //////////////////////////////// STATE_A ///////////////////////////////////
    //
    // Initial state. Loops until Controller forces the thread to exit.
    //
    // ID
    {STATE_A,
    // 
    // ACTIONS
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_COMMS_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    {}},
};

/*********************************** MAIN *************************************/

void ProfilePlatformComms_ControlNode::main (int, char**)
{
    ControlNode::entry (
            ProfilePlatform_Config::mCnNmConfig, 
            ProfilePlatform_Config::mCnDvConfig, 
            ProfilePlatform_Config::mChConfig, 
            gSmConfig, 
            (ControlNode::fInitializeControllers_t) initializeControllers);
}
