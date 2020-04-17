/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <iostream>

#include "Time.hpp"
#include "ControlNode.hpp"
#include "ProfilePlatformRxnTime_ControlNode.hpp"


/******************************* CONTROLLERS **********************************/

/**
 * Controller to read elements set by Device Nodes and write those values to
 * corresponding elements to be read by Device Nodes.
 */
class ProfileCnRxnTimeController final : public Controller
{
    public:

        /**
         * Unused.
         */
        typedef struct Config {} Config_t;

        /**
         * Copy values in DV_ELEM_TEST387 (set by DN0), DV_ELEM_TEST513 (set by
         * DN1), and DV_ELEM_TEST639 (set by DN2) to elements DV_ELEM_TEST2
         * (read by DN0), DV_ELEM_TEST131 (read by DN1), and DV_ELEM_TEST260
         * (read by DN2).
         */
        Error_t runEnabled ()
        {
            uint64_t dn0Val = 0;
            uint64_t dn1Val = 0;
            uint64_t dn2Val = 0;

            // Read values set by Device Nodes.
            if (mPDataVector->read (DV_ELEM_TEST387, dn0Val) != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_TEST513, dn1Val) != E_SUCCESS ||
                mPDataVector->read (DV_ELEM_TEST639, dn2Val) != E_SUCCESS)
            {
                pthread_exit ((void*) E_DATA_VECTOR_READ);
            }

            // Write values to elements that will be read by Device Nodes.
            if (mPDataVector->write (DV_ELEM_TEST2,   dn0Val) != E_SUCCESS ||
                mPDataVector->write (DV_ELEM_TEST131, dn1Val) != E_SUCCESS ||
                mPDataVector->write (DV_ELEM_TEST260, dn2Val) != E_SUCCESS)
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
        ProfileCnRxnTimeController (Config_t kConfig,
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
    std::unique_ptr<ProfileCnRxnTimeController> pCtrlr;
    ProfileCnRxnTimeController::Config_t ctrlConfig = {};
    Error_t ret = Controller::createNew (ctrlConfig, kPDv, 
                                         DV_ELEM_RXN_TIME_CN_CTRL_MODE, 
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
    // Initial state. Enables controllers and loops forever. Test ends when
    // Device Nodes exit.
    //
    // ID
    {STATE_A,
    // 
    // ACTIONS
    {{0 * Time::NS_IN_S,
         {ACT_CREATE_UINT8  ( DV_ELEM_RXN_TIME_CN_CTRL_MODE,   MODE_ENABLED ),
          ACT_CREATE_UINT8  ( DV_ELEM_RXN_TIME_DN0_CTRL_MODE,  MODE_ENABLED ),
          ACT_CREATE_UINT8  ( DV_ELEM_RXN_TIME_DN1_CTRL_MODE,  MODE_ENABLED ),
          ACT_CREATE_UINT8  ( DV_ELEM_RXN_TIME_DN2_CTRL_MODE,  MODE_ENABLED )}}},
    // 
    // TRANSITIONS
    {}},
};

/*********************************** MAIN *************************************/

void ProfilePlatformRxnTime_ControlNode::main (int, char**)
{
    ControlNode::entry (
            ProfilePlatform_Config::mCnNmConfig, 
            ProfilePlatform_Config::mCnDvConfig, 
            ProfilePlatform_Config::mChConfig, 
            gSmConfig, 
            (ControlNode::fInitializeControllers_t) initializeControllers);
}
