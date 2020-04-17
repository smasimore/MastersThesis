/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "Time.hpp"
#include "ControlNode.hpp"
#include "ProfilePlatformJitter_ControlNode.hpp"

/******************************* CONTROLLERS **********************************/

/**
 * Controller to manage jitter test. Checks if # of Control Node loops has 
 * hit NUM_RUNS. If yes, print results.
 */
class ProfileJitterController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Calculate jitter.
         */
        Error_t runEnabled ()
        {
            static const int64_t CN_LOOP_PERIOD_NS = 10 * Time::NS_IN_MS;
            static std::vector<int64_t> jitterBuf (NUM_RUNS);
            static uint32_t jitterIdx = 0;
            static Time* pTime = nullptr;
            static Time::TimeNs_t prevTimeNs = 0;
            Time::TimeNs_t currTimeNs = 0;

            // 1) The first time Controller runs, get Time Module and do not
            //    set curr time, since it is not accurate (had to wait until
            //    Time Module grabbed).
            if (pTime == nullptr)
            {
                if (Time::getInstance (pTime) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_INIT_TIME);
                }
            }

            // 2) Get current time.
            else
            {
                if (pTime->getTimeNs (currTimeNs) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_GET_TIME);
                }
            }

            // 3) If this isn't the first or second run, store jitter. 
            if (prevTimeNs != 0)
            {
                int64_t jitterNs = CN_LOOP_PERIOD_NS - 
                                   (int64_t) (currTimeNs - prevTimeNs);
                jitterBuf[jitterIdx] = jitterNs; 
                std::cout << jitterNs << std::endl;

                jitterIdx++;
            }

            // 4) Save curr as prev to use next loop.
            prevTimeNs = currTimeNs;

            // 5) If NUM_RUN have occurred, print results and exit. 
            if (jitterIdx == NUM_RUNS)
            {
                uint32_t numCommsDeadlineMisses = 0;
                uint32_t numLoopDeadlineMisses = 0;
                if (mPDataVector->read (DV_ELEM_CN_COMMS_DEADLINE_MISS_COUNT, 
                                        numCommsDeadlineMisses) != E_SUCCESS ||
                    mPDataVector->read (DV_ELEM_CN_LOOP_DEADLINE_MISS_COUNT, 
                                        numLoopDeadlineMisses) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_DATA_VECTOR_READ);
                }
                ProfileHelpers::printVectorStats (jitterBuf, "--- Results ---");
                std::cout << "# Comms Deadline Misses: " << 
                    numCommsDeadlineMisses << std::endl;
                std::cout << "# Loop Deadline Misses: " << numLoopDeadlineMisses 
                    << std::endl;
                pthread_exit ((void*) E_SUCCESS);
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
        ProfileJitterController (Config_t kConfig,
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
    std::unique_ptr<ProfileJitterController> pCtrlr;
    ProfileJitterController::Config_t ctrlConfig = {};
    Error_t ret = Controller::createNew (ctrlConfig, kPDv, 
                                         DV_ELEM_JITTER_CTRL_MODE, 
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
         {ACT_CREATE_UINT8  ( DV_ELEM_JITTER_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    {}},
};

/*********************************** MAIN *************************************/

void ProfilePlatformJitter_ControlNode::main (int, char**)
{
    ControlNode::entry (
            ProfilePlatform_Config::mCnNmConfig, 
            ProfilePlatform_Config::mCnDvConfig, 
            ProfilePlatform_Config::mChConfig, 
            gSmConfig, 
            (ControlNode::fInitializeControllers_t) initializeControllers);
}
