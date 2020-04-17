/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "DeviceNode.hpp"
#include "ProfilePlatformJitter_DeviceNode.hpp"

/********************************* DEVICES ************************************/

/**
 * Manage jitter test for Device Nodes. After NUM_RUNS prints results.
 */
class ProfileJitterDevice final : public Device
{

    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Calculate jitter.
         */
        virtual Error_t run ()
        {
            static const int64_t CN_LOOP_PERIOD_NS = 10 * Time::NS_IN_MS;
            static std::vector<int64_t> jitterBuf (NUM_RUNS);
            static uint32_t jitterIdx = 0;
            static Time* pTime = nullptr;
            static Time::TimeNs_t prevTimeNs = 0;
            Time::TimeNs_t currTimeNs = 0;

            // 1) The first time Device runs, get Time Module and do not set 
            //    curr time, since it is not accurate (had to wait until Time 
            //    Module grabbed).
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
            if (jitterIdx  == NUM_RUNS)
            {
                ProfileHelpers::printVectorStats (
                        jitterBuf, "--- Device Node Results ---");
                pthread_exit ((void*) E_SUCCESS);
            }

            return E_SUCCESS;
        }

        /**
         * Constructor.
         */
        ProfileJitterDevice (NiFpga_Session& kSession, 
                             std::shared_ptr<DataVector> kPDataVector,
                             Config_t& kConfig,
                             Error_t& kRet) :
            Device (kSession, kPDataVector) 
        {
            kRet = E_SUCCESS;
        }
};

/********************************* CONFIGS ************************************/

/**
 * Controller and Device initialization function.
 */
static Error_t initializeCtrlsAndDevs (
                         std::shared_ptr<DataVector> kPDv,                       
                         NiFpga_Session& kFpgaSession,                           
                         std::vector<std::unique_ptr<Controller>>& kPCtrls,         
                         std::vector<std::unique_ptr<Device>>& kPSensorDevs,        
                         std::vector<std::unique_ptr<Device>>& kPActuatorDevs)
{
    std::unique_ptr<ProfileJitterDevice> pDev;
    ProfileJitterDevice::Config_t devConfig = {};
    Error_t ret = Device::createNew (kFpgaSession, kPDv, devConfig,pDev);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Add as a sensor device so that it runs at top of loop.
    kPSensorDevs.push_back (std::move (pDev));
    
    return E_SUCCESS;
}

/*********************************** MAIN *************************************/

void ProfilePlatformJitter_DeviceNode::main (int, char**)
{
    DeviceNode::entry (
            ProfilePlatform_Config::mDnNmConfig, 
            ProfilePlatform_Config::mDnDvConfig,  
            (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevs,
            false);
}
