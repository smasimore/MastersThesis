/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "Time.hpp"
#include "ControlNode.hpp"
#include "ProfilePlatformOverhead_ControlNode.hpp"

/**
 * Wall spin time to start with.
 */
static const uint64_t INITIAL_WALL_TIME_TO_SPIN_NS = 5 * Time::NS_IN_MS;

/**
 * Amount to increment wall spin time after a deadline is not missed.
 */
static const uint64_t WALL_TIME_TO_SPIN_INC_NS = 25 * Time::NS_IN_US;

/**
 * It takes a few loops for DN to get new spin time and for CN Controller to 
 * detect a deadline miss, so run system for multiple loops per spin time.
 */
static const uint64_t NUM_LOOPS_PER_TIME_NS = 5;

/******************************* CONTROLLERS **********************************/

/**
 * Controller to manage overhead measurements. Increases wall spin time until a 
 * deadline miss is detected and then prints wall spin time and CPU process time
 * that last ran without a deadline miss. 
 */
class ProfileCnOverheadController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Phases of script.
         */
        enum Phase_t : uint8_t
        {
            DN0,
            DN1,
            DN2,
            CN
        };

        /**
         * Goes through 4 phases: DN0, DN1, DN2, and then CN overhead 
         * measurement phases.
         */
        Error_t runEnabled ()
        {
            Error_t ret = E_SUCCESS;
            switch (mPhase)
            {
                case DN0:
                    ret = measureDeviceOverhead (DV_ELEM_DN0_RX_MISS_COUNT,
                                                 DV_ELEM_TEST2,
                                                 DV_ELEM_TEST387);
                    break;
                case DN1:
                    ret = measureDeviceOverhead (DV_ELEM_DN1_RX_MISS_COUNT,
                                                 DV_ELEM_TEST131,
                                                 DV_ELEM_TEST513);
                    break;
                case DN2:
                    ret = measureDeviceOverhead (DV_ELEM_DN2_RX_MISS_COUNT,
                                                 DV_ELEM_TEST260,
                                                 DV_ELEM_TEST639);
                    break;
                case CN:
                    ret = measureControlOverhead ();
                    break;
            };

            // Handle error.
            if (ret != E_SUCCESS)
            {
                pthread_exit ((void*) ret);
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
        ProfileCnOverheadController (Config_t kConfig,
                                std::shared_ptr<DataVector> kPDv,
                                DataVectorElement_t kDvModeElem) :
            Controller (kPDv, kDvModeElem),
            mConfig (kConfig) {}
    
    private:

        /**
         * Unused.
         */
        Config_t mConfig;
       
        /**
         * Phase of experiment.
         */
        Phase_t mPhase = DN0;

        /**
         * Measure Platform overhead on Device Node by incrementing spin time
         * until a deadline miss is detected.
         *
         * @param  kMissedMsgsElem      DV element containing number of missed 
         *                              msgs for Device Node.
         * @param  kWallTimeToSpinElem  DV element to save wall time to spin in
         *                              for Device Node controller to read.
         * @param  kProcTimeSpunElem    DV element for Device Node controller to 
         *                              save CPU process time to.
         *
         * @ret    E_SUCCESS            Success.
         *         E_DATA_VECTOR_READ   Failed to read Data Vector.
         *         E_DATA_VECTOR_WRITE  Failed to write Data Vector.
         *
         */
        Error_t measureDeviceOverhead (DataVectorElement_t kMissedMsgsElem,
                                       DataVectorElement_t kWallTimeToSpinElem,
                                       DataVectorElement_t kProcTimeSpunElem)
        {
            static uint8_t numLoops = 0;
            static uint64_t wallTimeToSpinNs = INITIAL_WALL_TIME_TO_SPIN_NS;
            static uint32_t prevNumMissedMsgs = 0;
            static uint32_t prevNumRxdMsgs = 0;
            static bool prevMsgMissed = false;
            static uint64_t prevCpuProcTimeNs = 0;

            // Check if DN missed its deadline. If we missed the DN's previous
            // msg and only received 3 total messages this round, it means the
            // DN is taking longer than 10ms to complete its loop. We can't 
            // solely look at a missed msg, because this can occur due to a msg
            // getting "stuck" in the NIC's receive queue.
            uint32_t numMissedMsgs = 0;
            uint32_t numRxdMsgs = 0;
            if (mPDataVector->read (kMissedMsgsElem, numMissedMsgs) 
                    != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_CN_MSG_RX_COUNT, numRxdMsgs) 
                    != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            // If DN missed its deadline, print results, move on to the next 
            // phase, and reset static vars.
            if (prevMsgMissed && (numRxdMsgs - prevNumRxdMsgs == 3))
            {

                // Print results.
                std::cout << "---- Device Node " << (int) mPhase << " ----" <<
                    std::endl;
                std::cout << "Spin Wall Time (ns): " << wallTimeToSpinNs - 
                    WALL_TIME_TO_SPIN_INC_NS << std::endl;
                std::cout << "Spin CPU Process Time (ns): " << prevCpuProcTimeNs
                    << std::endl;

                // Update phase and disable/enable controllers.
                Mode_t dn0Ctrl = MODE_SAFED;
                Mode_t dn1Ctrl = MODE_SAFED;
                Mode_t dn2Ctrl = MODE_SAFED;
                switch (mPhase)
                {
                    case DN0:
                        mPhase = DN1;
                        dn1Ctrl = MODE_ENABLED;
                        break;
                    case DN1:
                        mPhase = DN2;
                        dn2Ctrl = MODE_ENABLED;
                        break;
                    case DN2:
                        mPhase = CN;
                        break;
                    default:
                        // Should never get here.
                        return E_INVALID_ENUM;
                }
                if (mPDataVector->write (DV_ELEM_OVERHEAD_DN0_CTRL_MODE,
                                         (uint8_t) dn0Ctrl) != E_SUCCESS ||
                    mPDataVector->write (DV_ELEM_OVERHEAD_DN1_CTRL_MODE,
                                         (uint8_t) dn1Ctrl) != E_SUCCESS ||
                    mPDataVector->write (DV_ELEM_OVERHEAD_DN2_CTRL_MODE,
                                         (uint8_t) dn2Ctrl) != E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }

                // Update relevant static vars.
                prevMsgMissed = false;
                wallTimeToSpinNs = INITIAL_WALL_TIME_TO_SPIN_NS;
            }

            else
            {
                // Set prev vars.
                prevMsgMissed = prevNumMissedMsgs != numMissedMsgs;
                prevNumMissedMsgs = numMissedMsgs;
                prevNumRxdMsgs = numRxdMsgs;
                
                // If we've looped NUM_LOOPS_PER_TIME_NS, increase wall time.
                if (numLoops == NUM_LOOPS_PER_TIME_NS)
                {
                    // Save recorded CPU process time. This will be used if the 
                    // next spin time value results in a miss.
                     if (mPDataVector->read (kProcTimeSpunElem, 
                                             prevCpuProcTimeNs) != E_SUCCESS)
                    {
                        return E_DATA_VECTOR_READ;
                    }

                    wallTimeToSpinNs += WALL_TIME_TO_SPIN_INC_NS;
                    if (mPDataVector->write (kWallTimeToSpinElem, 
                                             wallTimeToSpinNs) != E_SUCCESS)
                    {
                        return E_DATA_VECTOR_WRITE;
                    }

                    // Reset loop counter.
                    numLoops = 0;
                }

                numLoops++;
            }

            return E_SUCCESS;
        }

        /**
         * Measure Platform overhead on Control Node by incrementing spin time
         * until a deadline miss is detected.
         *
         * @ret    E_SUCCESS             Success.
         *         E_FAILED_TO_INIT_TIME Failed to init Time Module.
         *         E_FAILED_TO_GET_TIME  Failed to read time.
         *         E_DATA_VECTOR_READ    Failed to read Data Vector.
         *         E_DATA_VECTOR_WRITE   Failed to write Data Vector.
         *
         */
        Error_t measureControlOverhead ()
        {
            static uint8_t numLoops = 0;
            static uint64_t wallTimeToSpinNs = INITIAL_WALL_TIME_TO_SPIN_NS;
            static uint64_t prevCpuProcTimeNs = 0;

            // Get Time Module.
            static Time* pTime = nullptr;
            if (pTime == nullptr)
            {
                if (Time::getInstance (pTime) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_INIT_TIME);
                }
            }

            // Get wall start time.
            Time::TimeNs_t wallStartTimeNs = 0;
            if (pTime->getTimeNs (wallStartTimeNs) != E_SUCCESS)
            {
                pthread_exit ((void*) E_FAILED_TO_GET_TIME);
            }

            // Get CPU process start time.
            struct timespec cpuProcessStartTimeTs;
            if (clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &cpuProcessStartTimeTs) 
                    != 0)
            {
                pthread_exit ((void*) E_FAILED_TO_GET_TIME);
            }
            uint64_t cpuProcessStartTimeNs = 
                Time::NS_IN_S * cpuProcessStartTimeTs.tv_sec + 
                cpuProcessStartTimeTs.tv_nsec;

            // Check if missed loop deadline.
            uint32_t numMissedLoopDeadlines = 0;
            if (mPDataVector->read (DV_ELEM_CN_LOOP_DEADLINE_MISS_COUNT,
                                    numMissedLoopDeadlines) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            // If DN missed its deadline, print results, move on to the next 
            // phase, and reset static vars.
            if (numMissedLoopDeadlines > 0)
            {
                // Print results.
                std::cout << "---- Control Node ----" << std::endl;
                std::cout << "Spin Wall Time (ns): " << wallTimeToSpinNs -
                    WALL_TIME_TO_SPIN_INC_NS << std::endl;
                std::cout << "Spin CPU Process Time (ns): " << prevCpuProcTimeNs
                    << std::endl;

                // Exit thread.
                pthread_exit ((void*) E_SUCCESS);
            }

            // Otherwise, spin.
            else
            {
                // Spin.
                Time::TimeNs_t currWallTimeNs = 0;
                while (currWallTimeNs < wallStartTimeNs + wallTimeToSpinNs)
                {
                    if (pTime->getTimeNs (currWallTimeNs) != E_SUCCESS)
                    {
                        return E_FAILED_TO_GET_TIME;
                    }
                }

                // Get CPU process end time.
                struct timespec cpuProcessEndTimeTs;
                if (clock_gettime (CLOCK_PROCESS_CPUTIME_ID, 
                                   &cpuProcessEndTimeTs) != 0)
                {
                    pthread_exit ((void*) E_FAILED_TO_GET_TIME);
                }
                uint64_t cpuProcessEndTimeNs = 
                    Time::NS_IN_S * cpuProcessEndTimeTs.tv_sec + 
                    cpuProcessEndTimeTs.tv_nsec;

                // Increment number of loops.
                numLoops++;

                // If we've looped NUM_LOOPS_PER_TIME_NS, save the CPU process
                // time and increase wall time.
                if (numLoops == NUM_LOOPS_PER_TIME_NS)
                {
                    prevCpuProcTimeNs = cpuProcessEndTimeNs - 
                                        cpuProcessStartTimeNs;
                    wallTimeToSpinNs += WALL_TIME_TO_SPIN_INC_NS;

                    // Reset loop counter.
                    numLoops = 0;
                }
            }

            return E_SUCCESS;
        }
};

/********************************* CONFIGS ************************************/

/**
 * Controller initialization function.
 */
static Error_t initializeControllers (
                      std::shared_ptr<DataVector> kPDv,
                      std::vector<std::unique_ptr<Controller>>& kPCtlrsVecRet)
{
    std::unique_ptr<ProfileCnOverheadController> pCtrlr;
    ProfileCnOverheadController::Config_t ctrlConfig = {};
    Error_t ret = Controller::createNew (ctrlConfig, kPDv, 
                                         DV_ELEM_OVERHEAD_CN_CTRL_MODE, 
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
    // Initial state. Enables CN and DN0 overhead controllers. The rest of the
    // overhead controller modes set by CN controller.
    //
    // ID
    {STATE_A,
    // 
    // ACTIONS
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_OVERHEAD_CN_CTRL_MODE,   MODE_ENABLED ),
          ACT_CREATE_UINT8  ( DV_ELEM_OVERHEAD_DN0_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    {}},
};

/*********************************** MAIN *************************************/

void ProfilePlatformOverhead_ControlNode::main (int, char**)
{
    ControlNode::entry (
            ProfilePlatform_Config::mCnNmConfig, 
            ProfilePlatform_Config::mCnDvConfig, 
            ProfilePlatform_Config::mChConfig, 
            gSmConfig, 
            (ControlNode::fInitializeControllers_t) initializeControllers);
}
