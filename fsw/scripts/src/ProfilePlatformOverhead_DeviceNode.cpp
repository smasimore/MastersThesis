/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "DeviceNode.hpp"
#include "ProfilePlatformOverhead_DeviceNode.hpp"

/********************************* GLOBALS ************************************/

#if DEVICE_NODE_TO_COMPILE == DEVICE_NODE0
/**
 * Element containing amount of wall time to spin for.
 */
static const DataVectorElement_t WALL_TIME_SPIN_ELEMENT = DV_ELEM_TEST2;

/**
 * Element to store CPU process time in.
 */
static const DataVectorElement_t PROC_TIME_SPIN_ELEMENT = DV_ELEM_TEST387;

/**
 * Controller mode element.
 */
static const DataVectorElement_t CTRL_ELEMENT = DV_ELEM_OVERHEAD_DN0_CTRL_MODE;

#elif  DEVICE_NODE_TO_COMPILE == DEVICE_NODE1
static const DataVectorElement_t WALL_TIME_SPIN_ELEMENT = DV_ELEM_TEST131;
static const DataVectorElement_t PROC_TIME_SPIN_ELEMENT = DV_ELEM_TEST513;
static const DataVectorElement_t CTRL_ELEMENT = DV_ELEM_OVERHEAD_DN1_CTRL_MODE;
#else
static const DataVectorElement_t WALL_TIME_SPIN_ELEMENT = DV_ELEM_TEST260;
static const DataVectorElement_t PROC_TIME_SPIN_ELEMENT = DV_ELEM_TEST639;
static const DataVectorElement_t CTRL_ELEMENT = DV_ELEM_OVERHEAD_DN2_CTRL_MODE;
#endif

/******************************* CONTROLLERS **********************************/

/**
 * Controller to measure Device Node Platform CPU overhead.
 */
class ProfileDnOverheadController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Measure CPU time not used by Platform.
         */
        Error_t runEnabled ()
        {
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

            // Read wall time to spin for.
            uint64_t wallTimeToSpinNs = 0;
            if (mPDataVector->read (WALL_TIME_SPIN_ELEMENT, wallTimeToSpinNs) 
                    != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }

            // Spin.
            Time::TimeNs_t currWallTimeNs = 0;
            while (currWallTimeNs < wallStartTimeNs + wallTimeToSpinNs)
            {
                if (pTime->getTimeNs (currWallTimeNs) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_GET_TIME);
                }
            }

            // Get CPU process end time.
            struct timespec cpuProcessEndTimeTs;
            if (clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &cpuProcessEndTimeTs) 
                    != 0)
            {
                pthread_exit ((void*) E_FAILED_TO_GET_TIME);
            }
            uint64_t cpuProcessEndTimeNs = 
                Time::NS_IN_S * cpuProcessEndTimeTs.tv_sec + 
                cpuProcessEndTimeTs.tv_nsec;

            // Write CPU process time elapsed.
            uint64_t cpuProcessTimeElapsedNs = cpuProcessEndTimeNs - 
                                               cpuProcessStartTimeNs;
            if (mPDataVector->write (PROC_TIME_SPIN_ELEMENT, 
                                     cpuProcessTimeElapsedNs) != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_WRITE);
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
        ProfileDnOverheadController (Config_t kConfig,
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
 * Controller and Device initialization function.
 */
static Error_t initializeCtrlsAndDevs (
                         std::shared_ptr<DataVector> kPDv,                       
                         NiFpga_Session& kFpgaSession,                           
                         std::vector<std::unique_ptr<Controller>>& kPCtrls,         
                         std::vector<std::unique_ptr<Device>>& kPSensorDevs,        
                         std::vector<std::unique_ptr<Device>>& kPActuatorDevs)
{
    // Controllers.
    std::unique_ptr<ProfileDnOverheadController> pCtrlr;
    ProfileDnOverheadController::Config_t ctrlConfig = {};
    Error_t ret = Controller::createNew (ctrlConfig, kPDv, CTRL_ELEMENT, 
                                         pCtrlr);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Add controller to return vector. std::move necessary since controller
    // stored in unique_ptr. Move transfers ownership of the object.
    kPCtrls.push_back (std::move (pCtrlr));
    
    return E_SUCCESS;
}

/*********************************** MAIN *************************************/

void ProfilePlatformOverhead_DeviceNode::main (int, char**)
{
    DeviceNode::entry (
            ProfilePlatform_Config::mDnNmConfig, 
            ProfilePlatform_Config::mDnDvConfig,  
            (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevs,
            false);
}
