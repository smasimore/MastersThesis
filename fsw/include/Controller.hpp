#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <memory>

#include "DataVector.hpp"
#include "Errors.hpp"

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

/**
 * Controller's mode. This determines which run function (runEnabled
 * vs. runSafed) is called. Defined outside of class to enable more 
 * succinct usage.
 */
enum Mode_t : uint8_t
{
	MODE_SAFED,
	MODE_ENABLED,

	MODE_LAST
};

class Controller
{

    public:

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
         * @param   kPDataVector        Ptr to initialized Data Vector for this
         *                              node.
         * @param   kDvModeElem         Data Vector element to read to 
         *                              determine controller's mode.
         * @param   kPControllerRet     Pointer to return controller.
         *
         * @ret    E_SUCCESS            Controller successfully created.
         *         E_DATA_VECTOR_NULL   Data Vector ptr null.
         *         E_INVALID_ELEM       Invalid DV mode elem.
         *         [other]              Initialization or validation error 
         *                              returned by controller.
        */
        template <class T_Controller, class T_Config>
        static Error_t createNew (T_Config kConfig,
                                  std::shared_ptr<DataVector> kPDataVector,
                                  DataVectorElement_t kDvModeElem,
                                  std::unique_ptr<T_Controller>& kPControllerRet)
        {
            Error_t ret = E_SUCCESS;

            if (kPDataVector == nullptr)
            {
                return E_DATA_VECTOR_NULL;
            }

            if (kPDataVector->elementExists (kDvModeElem) != E_SUCCESS)
            {
                return E_INVALID_ELEM;
            }

            // Create controller.
            kPControllerRet.reset (new T_Controller (kConfig, kPDataVector, 
                                                     kDvModeElem));

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
         *          E_DATA_VECTOR_READ  Failed to read controller's mode from 
         *                              Data Vector.
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
         *           E_DATA_VECTOR_READ  Failed to read controller's mode from 
         *                               Data Vector.
         */
        Error_t getMode (Mode_t& modeRet);

        /**
         * Verify config.
         *
         * @ret       E_SUCCESS       Config was correct.
         *            [other]         Error indicating why config was incorrect.
         */
        virtual Error_t verifyConfig () = 0;

    protected:

        /**
         * Data Vector.
         */
        std::shared_ptr<DataVector> mPDataVector;

        /**
         * Constructor. Should only be called by derived controller
         * constructors.
         *
         * @param kPDataVector         Ptr to initialized Data Vector for this 
         *                             node.
         * @param kDvModeElem          Data Vector element to read to determine
         *                             controller's mode.
         */
        Controller (std::shared_ptr<DataVector> kPDataVector, 
                    DataVectorElement_t kDvModeElem);

    private:

        /**
         * Data Vector element storing controller's mode.
         */
        DataVectorElement_t mDvModeElem;

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
