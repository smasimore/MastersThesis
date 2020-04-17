/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "DeviceNode.hpp"
#include "ProfilePlatformRxnTime_DeviceNode.hpp"

/********************************* GLOBALS ************************************/

#if DEVICE_NODE_TO_COMPILE == DEVICE_NODE0
/**
 * Element Sensor will write to. Controllers will read this element and set
 * corresponding elements that will be read by Actuator.
 */
static const DataVectorElement_t SENSOR_ELEMENT = DV_ELEM_TEST387;

/**
 * Elements Actuator will read.
 */
static const DataVectorElement_t CN_ACTUATOR_ELEMENT = DV_ELEM_TEST2;
static const DataVectorElement_t DN_ACTUATOR_ELEMENT = DV_ELEM_TEST388;

/**
 * Controller mode element.
 */
static const DataVectorElement_t CTRL_ELEMENT = DV_ELEM_RXN_TIME_DN0_CTRL_MODE;

#elif  DEVICE_NODE_TO_COMPILE == DEVICE_NODE1
static const DataVectorElement_t SENSOR_ELEMENT = DV_ELEM_TEST513;
static const DataVectorElement_t CN_ACTUATOR_ELEMENT = DV_ELEM_TEST131;
static const DataVectorElement_t DN_ACTUATOR_ELEMENT = DV_ELEM_TEST514;
static const DataVectorElement_t CTRL_ELEMENT = DV_ELEM_RXN_TIME_DN1_CTRL_MODE;
#else
static const DataVectorElement_t SENSOR_ELEMENT = DV_ELEM_TEST639;
static const DataVectorElement_t CN_ACTUATOR_ELEMENT = DV_ELEM_TEST260;
static const DataVectorElement_t DN_ACTUATOR_ELEMENT = DV_ELEM_TEST640;
static const DataVectorElement_t CTRL_ELEMENT = DV_ELEM_RXN_TIME_DN2_CTRL_MODE;
#endif

/**
 * Value Sensor writes to element. Set by Actuator and read by Sensor.
 */
static uint64_t gValToWrite = 0;

/**
 * Start time to calculate reaction time. Set by Sensor and read by Actuator.
 */
static Time::TimeNs_t gStartTimeNs = 0;

/******************************* CONTROLLERS **********************************/

/**
 * Controller to read element set by Sensor Device and write the value to
 * corresponding elements to be read by Actuator Device.
 */
class ProfileDnRxnTimeController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Copy SENSOR_ELEMENT value to DN_ACTUATOR_ELEMENT.
         */
        Error_t runEnabled ()
        {
            uint64_t val = 0;

            // Read value.
            if (mPDataVector->read (SENSOR_ELEMENT, val) != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }

            // Write value.
            if (mPDataVector->write (DN_ACTUATOR_ELEMENT, val) != E_SUCCESS)
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
        ProfileDnRxnTimeController (Config_t kConfig,
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

/********************************* DEVICES ************************************/

/*
 * Write value for Control Node and Device Node Controllers to read and set
 * start time for calculating reaction time in ProfileRxnTimeActuator.
 */
class ProfileRxnTimeSensor final : public Device
{

    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Set element for Control Node and Device Node to read.
         */
        virtual Error_t run ()
        {
            // Store previous value written so we know when to reset the 
            // reaction time start time.
            static uint32_t prevValWritten = 0;

            // Get Time Module.
            static Time* pTime = nullptr;
            if (pTime == nullptr)
            {
                if (Time::getInstance (pTime) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_INIT_TIME);
                }
            }

            // If val to written has been incremented by Actuator, write new
            // value and set reaction time start time.
            if (gValToWrite != prevValWritten)
            {
                if (pTime->getTimeNs (gStartTimeNs) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_GET_TIME);
                }

                if (mPDataVector->write (SENSOR_ELEMENT, gValToWrite) 
                        != E_SUCCESS)
                {
                    pthread_exit ((void*) E_DATA_VECTOR_WRITE);
                }
            }

            prevValWritten = gValToWrite;

            return E_SUCCESS;
        }

        /**
         * Constructor.
         */
        ProfileRxnTimeSensor (NiFpga_Session& kSession, 
                              std::shared_ptr<DataVector> kPDataVector,
                              Config_t& kConfig,
                              Error_t& kRet) :
            Device (kSession, kPDataVector) 
        {
            kRet = E_SUCCESS;
        }
};

/*
 * Read value from Control Node and Device Node Controllers. If it equals the
 * value set by the Sensor, calculate reaction time.
 */
class ProfileRxnTimeActuator final : public Device
{

    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Set element for Control Node and Device Node to read.
         */
        virtual Error_t run ()
        {
            // Keep track of what run we're on. Since Controllers running on the
            // Device Nodes can react faster, we need to keep track of the CN
            // and DN runs separately.
            static uint32_t cnRun = 1;
            static uint32_t dnRun = 1;
            static bool complete = false;

            // Init buffers.
            static std::vector<Time::TimeNs_t> cnBuf (NUM_RUNS);
            static std::vector<Time::TimeNs_t> dnBuf (NUM_RUNS);
  
            if (complete == true)
            {
                return E_SUCCESS;
            }

            // The first time we run, increment value to write. This makes sure
            // we don't consider the 0 case a valid run.
            if (gValToWrite == 0)
            {
                gValToWrite = 1;
            }

            // Initialize Time Module.
            static Time* pTime = nullptr;
            if (pTime == nullptr)
            {
                if (Time::getInstance (pTime) != E_SUCCESS)
                {
                    pthread_exit ((void*) E_FAILED_TO_INIT_TIME);
                }
            }

            // Check elements set by Control Node and Device Node Controllers.
            // If they are equal to gValToWrite, calculate reaction time. 
            uint64_t cnVal = 0;
            uint64_t dnVal = 0;
            if (mPDataVector->read (CN_ACTUATOR_ELEMENT, cnVal) != E_SUCCESS ||
                mPDataVector->read (DN_ACTUATOR_ELEMENT, dnVal) != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }

            // Get current time.
            Time::TimeNs_t endTimeNs = 0;
            if (pTime->getTimeNs (endTimeNs) != E_SUCCESS)
            {
                pthread_exit ((void*) E_FAILED_TO_GET_TIME);
            }

            if (dnVal == gValToWrite && dnVal == dnRun)
            {
                // Calculate rxn time.
                dnBuf[gValToWrite - 1] = endTimeNs - gStartTimeNs; 
                
                // Increment dnRun.
                dnRun++;
            }

            if (cnVal == gValToWrite && cnVal == cnRun)
            {
                // Calculate rxn time.
                cnBuf[gValToWrite - 1] = endTimeNs - gStartTimeNs; 
                
                // Increment cnRun.
                cnRun++;
            }

            // If both cnRun and dnRun have incremented, we are done with this
            // run. Increment gValToWrite to move on to next run.
            if (cnRun != gValToWrite && dnRun != gValToWrite)
            {
                std::cout << cnBuf[gValToWrite - 1] << ", " << 
                    dnBuf[gValToWrite - 1] << std::endl;
                gValToWrite++;
            }

            // After NUM_RUNS, print results and exit thread.
            if (gValToWrite - 1 == NUM_RUNS)
            {
                ProfileHelpers::printVectorStats (cnBuf, 
                    "---- CN Results ----");
                ProfileHelpers::printVectorStats (dnBuf, 
                    "---- DN Results ----");

                complete = true;
            }

            return E_SUCCESS;
        }

        /**
         * Constructor.
         */
        ProfileRxnTimeActuator (NiFpga_Session& kSession, 
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
    // Sensors.
    std::unique_ptr<ProfileRxnTimeSensor> pSensor;
    ProfileRxnTimeSensor::Config_t sensorConfig = {};
    Error_t ret = Device::createNew (kFpgaSession, kPDv, sensorConfig,pSensor);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Add sensor device.
    kPSensorDevs.push_back (std::move (pSensor));

    // Controllers.
    std::unique_ptr<ProfileDnRxnTimeController> pCtrlr;
    ProfileDnRxnTimeController::Config_t ctrlConfig = {};
    ret = Controller::createNew (ctrlConfig, kPDv, CTRL_ELEMENT, pCtrlr);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Add controller to return vector. std::move necessary since controller
    // stored in unique_ptr. Move transfers ownership of the object.
    kPCtrls.push_back (std::move (pCtrlr));

    // Actuators.
    std::unique_ptr<ProfileRxnTimeActuator> pActuator;
    ProfileRxnTimeActuator::Config_t actuatorConfig = {};
    ret = Device::createNew (kFpgaSession, kPDv, actuatorConfig, pActuator);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Add actuator device.
    kPActuatorDevs.push_back (std::move (pActuator));
    
    return E_SUCCESS;
}

/*********************************** MAIN *************************************/

void ProfilePlatformRxnTime_DeviceNode::main (int, char**)
{
    DeviceNode::entry (
            ProfilePlatform_Config::mDnNmConfig, 
            ProfilePlatform_Config::mDnDvConfig,  
            (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevs,
            false);
}
