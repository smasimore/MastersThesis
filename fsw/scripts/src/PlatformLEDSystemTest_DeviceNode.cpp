/**
 * See PlatformLEDSystemTest_Config.hpp for instructions on running test.
 */

#include <iostream>

#include "DeviceNode.hpp"
#include "DigitalOutDevice.hpp"
#include "PlatformLEDSystemTest_DeviceNode.hpp"

/******************************* CONTROLLERS **********************************/

/**
 * Controller to flash an LED.
 */
class FlashLEDController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Flash LED every half a second.
         */
        Error_t runEnabled ()
        {
            // Get Time Module.
            static Time* pTime = nullptr;
            if (pTime == nullptr)
            {
                if (Time::getInstance (pTime) != E_SUCCESS)
                {
                    return E_FAILED_TO_INIT_TIME;
                }
            }

            // Get time.
            Time::TimeNs_t currTimeNs = 0;
            if (pTime->getTimeNs (currTimeNs) != E_SUCCESS)
            {
                return E_FAILED_TO_GET_TIME;
            }

            // Every half second flip LED control value.
            uint64_t currTimeMs = currTimeNs / Time::NS_IN_MS;
            if (currTimeMs % (1 * Time::MS_IN_S) >= 500)
            {
                // Set high.
                if (mPDataVector->write (DV_ELEM_DN_FLASH_LED_CONTROL_VAL,
                                         true) != E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }
            }
            else
            {
                // Set low
                if (mPDataVector->write (DV_ELEM_DN_FLASH_LED_CONTROL_VAL,
                                         false) != E_SUCCESS)
                {
                    return E_DATA_VECTOR_WRITE;
                }
            }

            return E_SUCCESS;
        }

        /**
         * Set LED to low.
         */
        Error_t runSafed ()
        {
            // Set low
            if (mPDataVector->write (DV_ELEM_DN_FLASH_LED_CONTROL_VAL,
                                     false) != E_SUCCESS)
            {
                return E_DATA_VECTOR_WRITE;
            }
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
        FlashLEDController (Config_t kConfig,
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
#if DEVICE_NODE_TO_COMPILE == DEVICE_NODE0
    std::unique_ptr<DigitalOutDevice> pStateADev = nullptr;
    std::unique_ptr<DigitalOutDevice> pStateBDev = nullptr;
    std::unique_ptr<DigitalOutDevice> pStateCDev = nullptr;
    std::unique_ptr<DigitalOutDevice> pStateDDev = nullptr;
    std::unique_ptr<DigitalOutDevice> pStateEDev = nullptr;
    DigitalOutDevice::Config_t stateADevConfig = 
    {
        DV_ELEM_STATE_A_LED_CONTROL_VAL,
        DV_ELEM_STATE_A_LED_FEEDBACK_VAL,
        5,
    };
    DigitalOutDevice::Config_t stateBDevConfig = 
    {
        DV_ELEM_STATE_B_LED_CONTROL_VAL,
        DV_ELEM_STATE_B_LED_FEEDBACK_VAL,
        7,
    };
    DigitalOutDevice::Config_t stateCDevConfig = 
    {
        DV_ELEM_STATE_C_LED_CONTROL_VAL,
        DV_ELEM_STATE_C_LED_FEEDBACK_VAL,
        9,
    };
    DigitalOutDevice::Config_t stateDDevConfig = 
    {
        DV_ELEM_STATE_D_LED_CONTROL_VAL,
        DV_ELEM_STATE_D_LED_FEEDBACK_VAL,
        11,
    };
    DigitalOutDevice::Config_t stateEDevConfig = 
    {
        DV_ELEM_STATE_E_LED_CONTROL_VAL,
        DV_ELEM_STATE_E_LED_FEEDBACK_VAL,
        13,
    };
    Error_t ret = Device::createNew (kFpgaSession, kPDv, stateADevConfig, 
                                     pStateADev);
    if (ret != E_SUCCESS)
    {
        std::cout << "State A Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, stateBDevConfig, 
                             pStateBDev);
    if (ret != E_SUCCESS)
    {
        std::cout << "State B Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, stateCDevConfig, 
                             pStateCDev);
    if (ret != E_SUCCESS)
    {
        std::cout << "State C Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, stateDDevConfig, 
                             pStateDDev);
    if (ret != E_SUCCESS)
    {
        std::cout << "State D Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, stateEDevConfig, 
                             pStateEDev);
    if (ret != E_SUCCESS)
    {
        std::cout << "State E Device failed to init." << std::endl;
        return ret;
    }

    kPActuatorDevs.push_back (std::move (pStateADev));
    kPActuatorDevs.push_back (std::move (pStateBDev));
    kPActuatorDevs.push_back (std::move (pStateCDev));
    kPActuatorDevs.push_back (std::move (pStateDDev));
    kPActuatorDevs.push_back (std::move (pStateEDev));

#elif DEVICE_NODE_TO_COMPILE == DEVICE_NODE1
    std::unique_ptr<DigitalOutDevice> pLed0Dev = nullptr;
    std::unique_ptr<DigitalOutDevice> pLed1Dev = nullptr;
    std::unique_ptr<DigitalOutDevice> pLed2Dev = nullptr;
    std::unique_ptr<DigitalOutDevice> pLed3Dev = nullptr;
    std::unique_ptr<DigitalOutDevice> pLed4Dev = nullptr;
    DigitalOutDevice::Config_t led0DevConfig = 
    {
        DV_ELEM_CN_LED0_CONTROL_VAL,
        DV_ELEM_CN_LED0_FEEDBACK_VAL,
        5,
    };
    DigitalOutDevice::Config_t led1DevConfig = 
    {
        DV_ELEM_CN_LED1_CONTROL_VAL,
        DV_ELEM_CN_LED1_FEEDBACK_VAL,
        7,
    };
    DigitalOutDevice::Config_t led2DevConfig = 
    {
        DV_ELEM_CN_LED2_CONTROL_VAL,
        DV_ELEM_CN_LED2_FEEDBACK_VAL,
        9,
    };
    DigitalOutDevice::Config_t led3DevConfig = 
    {
        DV_ELEM_CN_LED3_CONTROL_VAL,
        DV_ELEM_CN_LED3_FEEDBACK_VAL,
        11,
    };
    DigitalOutDevice::Config_t led4DevConfig = 
    {
        DV_ELEM_CN_LED4_CONTROL_VAL,
        DV_ELEM_CN_LED4_FEEDBACK_VAL,
        13,
    };
    Error_t ret = Device::createNew (kFpgaSession, kPDv, led0DevConfig, 
                                     pLed0Dev);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 0 Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, led1DevConfig, 
                             pLed1Dev);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 1 Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, led2DevConfig, 
                             pLed2Dev);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 2 Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, led3DevConfig, 
                             pLed3Dev);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 3 Device failed to init." << std::endl;
        return ret;
    }
    ret = Device::createNew (kFpgaSession, kPDv, led4DevConfig, 
                             pLed4Dev);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 4 Device failed to init." << std::endl;
        return ret;
    }

    kPActuatorDevs.push_back (std::move (pLed0Dev));
    kPActuatorDevs.push_back (std::move (pLed1Dev));
    kPActuatorDevs.push_back (std::move (pLed2Dev));
    kPActuatorDevs.push_back (std::move (pLed3Dev));
    kPActuatorDevs.push_back (std::move (pLed4Dev));

#else
    // LED Device.
    std::unique_ptr<DigitalOutDevice> pLedDev = nullptr;
    DigitalOutDevice::Config_t ledDevConfig = 
    {
        DV_ELEM_DN_FLASH_LED_CONTROL_VAL,
        DV_ELEM_DN_FLASH_LED_FEEDBACK_VAL,
        5,
    };
    Error_t ret = Device::createNew (kFpgaSession, kPDv, ledDevConfig, 
                                     pLedDev);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED Device failed to init." << std::endl;
        return ret;
    }
    kPActuatorDevs.push_back (std::move (pLedDev));

    // FlashLEDController.
    std::unique_ptr<FlashLEDController> pFlashLedCtrlr;
    FlashLEDController::Config_t flashLedConfig = {};
    ret = Controller::createNew (flashLedConfig, kPDv, 
                                 DV_ELEM_DN_FLASH_LED_CTRL_MODE, 
                                 pFlashLedCtrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "FlashLEDController failed to init." << std::endl;
        return ret;
    }

    kPCtrls.push_back (std::move (pFlashLedCtrlr));
#endif

    return E_SUCCESS;
}

/*********************************** MAIN *************************************/

void PlatformLEDSystemTest_DeviceNode::main (int, char**)
{
    DeviceNode::entry (
            PlatformLEDSystemTest_Config::mDnNmConfig, 
            PlatformLEDSystemTest_Config::mDnDvConfig,  
            (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevs,
            false);
}
