/**
 * See PlatformLEDSystemTest_ControlNode.hpp for instructions on running test.
 */

#include <iostream>

#include "LEDController.hpp"
#include "ControlNode.hpp"
#include "PlatformLEDSystemTest_ControlNode.hpp"

/******************************* CONTROLLERS **********************************/

/**
 * Controller to set LED's on Device Node 0 to display the state system is in. 
 */
class StateLEDController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Set LED control value depending on state.
         */
        Error_t runEnabled ()
        {
            // todo: switch on state mapping to digital out control
            uint32_t state = STATE_A;
            if (mPDataVector->read (DV_ELEM_STATE, state) != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            bool stateALedControlVal = false;
            bool stateBLedControlVal = false;
            bool stateCLedControlVal = false;
            bool stateDLedControlVal = false;
            bool stateELedControlVal = false;
            switch ((StateId_t) state)
            {
                case STATE_A:
                    stateALedControlVal = true;
                    break;
                case STATE_B:
                    stateBLedControlVal = true;
                    break;
                case STATE_C:
                    stateCLedControlVal = true;
                    break;
                case STATE_D:
                    stateDLedControlVal = true;
                    break;
                case STATE_E:
                    stateELedControlVal = true;
                    break;
                default:
                    return E_INVALID_ENUM;
            }

            if (mPDataVector->write (DV_ELEM_STATE_A_LED_CONTROL_VAL, 
                                     stateALedControlVal) != E_SUCCESS ||
                mPDataVector->write (DV_ELEM_STATE_B_LED_CONTROL_VAL, 
                                     stateBLedControlVal) != E_SUCCESS ||
                mPDataVector->write (DV_ELEM_STATE_C_LED_CONTROL_VAL, 
                                     stateCLedControlVal) != E_SUCCESS ||
                mPDataVector->write (DV_ELEM_STATE_D_LED_CONTROL_VAL, 
                                     stateDLedControlVal) != E_SUCCESS ||
                mPDataVector->write (DV_ELEM_STATE_E_LED_CONTROL_VAL, 
                                     stateELedControlVal) != E_SUCCESS)
            {
                return E_DATA_VECTOR_WRITE;
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
        StateLEDController (Config_t kConfig,
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

/**
 * Controller to current exit thread. 
 */
class ExitThreadController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Exit thread.
         */
        Error_t runEnabled ()
        {
            pthread_exit ((void*) E_SUCCESS);
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
        ExitThreadController (Config_t kConfig,
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
    // StateLEDController.
    std::unique_ptr<StateLEDController> pStateLedCtrlr;
    StateLEDController::Config_t stateLedConfig = {};
    Error_t ret = Controller::createNew (stateLedConfig, kPDv, 
                                         DV_ELEM_STATE_LED_CTRL_MODE, 
                                         pStateLedCtrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "StateLEDController failed to init." << std::endl;
        return ret;
    }

    // LED Controllers.
    std::unique_ptr<LEDController> pLed0Ctrlr;
    std::unique_ptr<LEDController> pLed1Ctrlr;
    std::unique_ptr<LEDController> pLed2Ctrlr;
    std::unique_ptr<LEDController> pLed3Ctrlr;
    std::unique_ptr<LEDController> pLed4Ctrlr;
    LEDController::Config_t led0Config = {DV_ELEM_CN_LED0_CONTROL_VAL};
    LEDController::Config_t led1Config = {DV_ELEM_CN_LED1_CONTROL_VAL};
    LEDController::Config_t led2Config = {DV_ELEM_CN_LED2_CONTROL_VAL};
    LEDController::Config_t led3Config = {DV_ELEM_CN_LED3_CONTROL_VAL};
    LEDController::Config_t led4Config = {DV_ELEM_CN_LED4_CONTROL_VAL};
    ret = Controller::createNew (led0Config, kPDv, DV_ELEM_CN_LED0_CTRL_MODE, 
                                 pLed0Ctrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 0 Controller failed to init." << std::endl;
        return ret;
    }
    ret = Controller::createNew (led1Config, kPDv, DV_ELEM_CN_LED1_CTRL_MODE, 
                                 pLed1Ctrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 1 Controller failed to init." << std::endl;
        return ret;
    }
    ret = Controller::createNew (led2Config, kPDv, DV_ELEM_CN_LED2_CTRL_MODE, 
                                 pLed2Ctrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 2 Controller failed to init." << std::endl;
        return ret;
    }
    ret = Controller::createNew (led3Config, kPDv, DV_ELEM_CN_LED3_CTRL_MODE, 
                                 pLed3Ctrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 3 Controller failed to init." << std::endl;
        return ret;
    }
    ret = Controller::createNew (led4Config, kPDv, DV_ELEM_CN_LED4_CTRL_MODE, 
                                 pLed4Ctrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "LED 4 Controller failed to init." << std::endl;
        return ret;
    }

    // ExitThreadController.
    std::unique_ptr<ExitThreadController> pExitThreadCtrlr;
    ExitThreadController::Config_t exitThreadConfig = {};
    ret = Controller::createNew (exitThreadConfig, kPDv, 
                                 DV_ELEM_THREAD_KILL_CTRL_MODE, 
                                 pExitThreadCtrlr);
    if (ret != E_SUCCESS)
    {
        std::cout << "ExitThreadController failed to init." << std::endl;
        return ret;
    }

    // Add controllers to return vector. std::move necessary since controller
    // stored in unique_ptr. Move transfers ownership of the object.
    kPCtlrsVecRet.push_back (std::move (pStateLedCtrlr));
    kPCtlrsVecRet.push_back (std::move (pLed0Ctrlr));
    kPCtlrsVecRet.push_back (std::move (pLed1Ctrlr));
    kPCtlrsVecRet.push_back (std::move (pLed2Ctrlr));
    kPCtlrsVecRet.push_back (std::move (pLed3Ctrlr));
    kPCtlrsVecRet.push_back (std::move (pLed4Ctrlr));
    kPCtlrsVecRet.push_back (std::move (pExitThreadCtrlr));

    return E_SUCCESS;
}

/**
 * State Machine config. 
 */
static StateMachine::Config_t gSmConfig =
{
    //////////////////////////////// STATE_A ///////////////////////////////////
    //
    // Initial state. Enables StateLEDController and loops until LAUNCH command
    // received from Ground Node.
    //
    // ID
    {STATE_A,
    // 
    // ACTIONS
    //
    // SECONDS ELAPSED IN STATE
    //      TYPE                 ELEMENT                       VALUE
    //
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_STATE_LED_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    //
    // TYPE            ELEMENT       COMPARISON   VALUE        STATE
    //
    {TR_CREATE_UINT8 ( DV_ELEM_CMD,  CMP_EQUALS,  CMD_LAUNCH,  STATE_B )}},

    //////////////////////////////// STATE_B ///////////////////////////////////
    //
    // After 3 seconds, set transition flag.
    //
    // ID
    {STATE_B,
    // 
    // ACTIONS
    //
    // SECONDS ELAPSED IN STATE
    //      TYPE                 ELEMENT                       VALUE
    //
    {{3 * Time::NS_IN_S,
         {ACT_CREATE_BOOL  ( DV_ELEM_STATE_B_TRANS_FLAG,  true )}}},
    // 
    // TRANSITIONS
    //
    // TYPE           ELEMENT                      COMPARISON   VALUE  STATE
    //
    {TR_CREATE_BOOL ( DV_ELEM_STATE_B_TRANS_FLAG,  CMP_EQUALS,  true,  STATE_C )}},

    //////////////////////////////// STATE_C ///////////////////////////////////
    //
    // Sequentially enable 5 LED Controllers running on the Control Node. 
    // Transition after last LED is verified to be on based on the LED's 
    // feedback value.
    //
    // ID
    {STATE_C,
    // 
    // ACTIONS
    //
    // SECONDS ELAPSED IN STATE
    //      TYPE                 ELEMENT                       VALUE
    //
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_CN_LED0_CTRL_MODE,  MODE_ENABLED )}},
     {1 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_CN_LED1_CTRL_MODE,  MODE_ENABLED )}},
     {2 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_CN_LED2_CTRL_MODE,  MODE_ENABLED )}},
     {3 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_CN_LED3_CTRL_MODE,  MODE_ENABLED )}},
     {4 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_CN_LED4_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    //
    // TYPE           ELEMENT                        COMPARISON   VALUE  STATE
    //
    {TR_CREATE_BOOL ( DV_ELEM_CN_LED4_FEEDBACK_VAL,  CMP_EQUALS,  true,  STATE_D )}},

    //////////////////////////////// STATE_D ///////////////////////////////////
    //
    // Enable Device Node 2's Flashing LED Controller. Transition after receive 
    // abort command.
    //
    // ID
    {STATE_D,
    // 
    // ACTIONS
    //
    // SECONDS ELAPSED IN STATE
    //      TYPE                 ELEMENT                       VALUE
    //
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_DN_FLASH_LED_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    //
    // TYPE            ELEMENT       COMPARISON   VALUE       STATE
    //
    {TR_CREATE_UINT8 ( DV_ELEM_CMD,  CMP_EQUALS,  CMD_ABORT,  STATE_E )}},

    //////////////////////////////// STATE_E ///////////////////////////////////
    //
    // Safe all Controllers, then enable ProgramExitController. 
    //
    // ID
    {STATE_E,
    // 
    // ACTIONS
    //
    // SECONDS ELAPSED IN STATE
    //      TYPE                 ELEMENT                       VALUE
    //
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_CN_LED0_CTRL_MODE,       MODE_SAFED ),
          ACT_CREATE_UINT8  ( DV_ELEM_CN_LED1_CTRL_MODE,       MODE_SAFED ),
          ACT_CREATE_UINT8  ( DV_ELEM_CN_LED2_CTRL_MODE,       MODE_SAFED ),
          ACT_CREATE_UINT8  ( DV_ELEM_CN_LED3_CTRL_MODE,       MODE_SAFED ),
          ACT_CREATE_UINT8  ( DV_ELEM_CN_LED4_CTRL_MODE,       MODE_SAFED ),
          ACT_CREATE_UINT8  ( DV_ELEM_DN_FLASH_LED_CTRL_MODE,  MODE_SAFED )}},
     {1 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_THREAD_KILL_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    {}},
};

/*********************************** MAIN *************************************/

void PlatformLEDSystemTest_ControlNode::main (int, char**)
{
    ControlNode::entry (
            PlatformLEDSystemTest_Config::mCnNmConfig, 
            PlatformLEDSystemTest_Config::mCnDvConfig, 
            PlatformLEDSystemTest_Config::mChConfig, 
            gSmConfig, 
            (ControlNode::fInitializeControllers_t) initializeControllers);
}
