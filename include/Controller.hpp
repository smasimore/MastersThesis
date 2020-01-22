#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <memory>

#include "StateVector.hpp"
#include "Errors.h"

/**
 * Base controller class for implementing high-level controllers on the rocket
 * (e.g. GNC, ParachuteDeploy).
 *
 * IMPLEMENTING A CONTROLLER
 *
 * 1. Extend the Controller base class to create YourController.
 * 2. Define YourController::Config_t, which will contain any controller-
 *    specific configuration (e.g. calibration values) and will be passed to
 *    YourController on initialization.
 * 3. Implement the constructor,  runEnabled, runSafed, and verifyConfig 
 *    methods.
 *
 * See TestController in the tests directory for an example.
 *
 *
 * USING A CONTROLLER
 *
 * 1. Call Controller::createNew<YourController> (...).
 *
 *       Note: Controllers should not be initialized directly, but instead
 *       through the createNew function below. This ensures a controller's
 *       config is validated before the controller is used.
 *
 * 2. Set the controller's mode (ENABLED or SAFED) using setMode.
 * 3. Call YourController->run () for each loop of your main periodic thread.
 *
 */

class Controller
{

    public:

        /**
         * Controller's mode. This determines which run function (runEnabled
         * vs. runSafed) is called.
         */
        enum Mode_t : uint8_t
        {
            SAFED,
            ENABLED,

            LAST
        };

        /**
         * Destructor.
         */
        virtual ~Controller ();

        /**
         * Entry point for creating a new controller object. Validates the
         * passed in controller config. Defined in the header so that the
         * templatized functions do not need to each be instantiated explicitly.
         *
         * @param   kConfig             Controller's config data.
         * @param   kPStateVector       Ptr to initialized State Vector for this
         *                              node.
         * @param   kSvModeElem         State Vector element to read to 
         *                              determine controller's mode.
         * @param   kPControllerRet     Pointer to return controller.
         *
         * @ret    E_SUCCESS            Controller successfully created.
         *         E_STATE_VECTOR_NULL  State Vector ptr null.
         *         E_INVALID_ELEM       Invalid SV mode elem.
         *         [other]              Validation error returned by controller.
        */
        template <class T_Controller, class T_Config>
        static Error_t createNew (T_Config kConfig,
                                  std::shared_ptr<StateVector> kPStateVector,
                                  StateVectorElement_t kSvModeElem,
                                  std::unique_ptr<T_Controller>& kPControllerRet)
        {
            Error_t ret = E_SUCCESS;

            if (kPStateVector == nullptr)
            {
                return E_STATE_VECTOR_NULL;
            }

            if (kPStateVector->elementExists (kSvModeElem) != E_SUCCESS)
            {
                return E_INVALID_ELEM;
            }

            // Create controller.
            kPControllerRet.reset (new T_Controller (kConfig, kPStateVector, 
                                                     kSvModeElem));

            // Verify kConfig.
            ret = kPControllerRet->verifyConfig ();
            if (ret != E_SUCCESS)
            {
                // Free controller.
                kPControllerRet.reset (nullptr);
                return ret;
            }

            return E_SUCCESS;
        }

        /**
         * Run controller logic once.
         *
         * @ret     E_SUCCESS           Controller ran successfully.
         *          E_STATE_VECTOR_READ Failed to read controller's mode from 
         *                              State Vector.
         *          E_INVALID_ENUM      Invalid mode.
         *          [other]             Error returned by controller.
         */
        Error_t run ();

        /**
         * Get a copy of the controller's mode.
         *
         * @param    modeRet    Copy of controller's mode returned in this
         *                      param.
         *
         * @return   E_SUCCESS           Mode returned successfully.
         *           E_STATE_VECTOR_READ Failed to read controller's mode from 
         *                               State Vector.
         */
        Error_t getMode (Mode_t& modeRet);

        /**
         * Verify config.
         *
         * @param     configValidRet  Set to true if config valid, false if
         *                            invalid.
         *
         * @ret       E_SUCCESS       Config was correct.
         *            [other]         Error indicating why config was incorrect.
         */
        virtual Error_t verifyConfig () = 0;

    protected:

        /**
         * State Vector.
         */
        std::shared_ptr<StateVector> mPStateVector;

        /**
         * Constructor. Should only be called by derived controller
         * constructors.
         *
         * @param kPStateVector        Ptr to initialized State Vector for this 
         *                             node.
         * @param kSvModeElem          State Vector element to read to determine
         *                             controller's mode.
         */
        Controller (std::shared_ptr<StateVector> kPStateVector, 
                    StateVectorElement_t kSvModeElem);

    private:

        /**
         * State Vector element storing controller's mode.
         */
        StateVectorElement_t mSvModeElem;

        /**
         * Method that is called by run when controller is ENABLED.
         *
         * Note: This function must always return. I.e. derived controller
         * implementation cannot have an infinite loop.
         *
         * @ret     E_SUCCESS    Controller ran successfully.
         *          [other]      Error returned by controller.
         */
        virtual Error_t runEnabled () = 0;

        /**
         * Method that is called by run when controller is SAFED.
         *
         * Note: This function must always return. I.e. derived controller
         * implementation cannot have an infinite loop.
         *
         * @ret     E_SUCCESS    Controller ran successfully.
         *          [other]      Error returned by controller.
         */
        virtual Error_t runSafed () = 0;
};

#endif
