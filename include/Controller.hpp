#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <memory>

#include "Errors.h"

/**
 * Base controller class for implementing high-level controllers on the rocket
 * (e.g. GNC, ParachuteDeploy).
 *
 * IMPLEMENTING A CONTROLLER
 *
 * 1. Extend the Controller base class to create YourController.
 * 2. Define struct YourConfig, which will contain any controller-specific 
 *    configuration (e.g. calibration values) and will be passed to 
 *    YourController on initialization.
 * 3. Implement the constructor (YourController(struct YourConfig)), runEnabled, 
 *    runSafed, and verifyConfig methods.
 *
 * See TestController in the tests directory for an example.
 *
 *
 * USING A CONTROLLER
 *
 * 1. Call Controller::createNew<YourController, 
 *                               struct YourController::YourConfig> with the
 *    relevant controller config data and a unique_ptr to store the 
 *    initialized controller pointer in.
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
            ENABLED,
            SAFED,
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
         * @param   config              Controller's config data.
         * @param   pControllerRet      Pointer to return controller.
         *
         * @ret    E_SUCCESS            Controller successfully created.
         *         E_INVALID_CONFIG     Config invalid.
        */
        template <class T_Controller, class T_Config>
        static Error_t createNew (T_Config config, 
                                  std::unique_ptr<T_Controller>& pControllerRet)
        {
            Error_t ret = E_SUCCESS;

            // Create controller.
            pControllerRet.reset (new T_Controller (config));
            
            // Verify config.
            bool configValid = false;
            ret = pControllerRet->verifyConfig (configValid); 
            if (ret != E_SUCCESS || configValid == false)
            {
                // Free controller.
                pControllerRet.reset (nullptr);
                return E_INVALID_CONFIG;
            }

            return E_SUCCESS;    
        }
    

        /**
         * Run controller logic once.
         *
         * @ret     E_SUCCESS   Controller ran successfully.
         *          [other]     Error returned by controller.
         */
        Error_t run();

        /**
         * Get a copy of the controller's mode.
         *
         * @param    modeRet    Copy of controller's mode returned in this 
         *                      param.
         *
         * @return   E_SUCCESS  Mode returned successfully.
         */
        Error_t getMode (Mode_t& modeRet);

        /**
         * Set the controller's mode.
         *
         * @param    newMode        New mode to set.    
         *
         * @return   E_SUCCESS      Mode set successfully.
         *           E_INVALID_ENUM Invalid mode.
         */
        Error_t setMode (Mode_t newMode);

        /**
         * Verify config. 
         * 
         * @param     configValidRet  Set to true if config valid, false if 
         *                            invalid.
         *
         * @ret       E_SUCCESS       Successfully verified config. Note, this
         *                            does not indicate config is valid.
         */
        virtual Error_t verifyConfig (bool &configValidRet) = 0;
    
    protected:

        /**
         * Constructor. Should only be called by derived controller 
         * constructors. 
         */
        Controller ();

    private:

        /**
         * Controller's current mode.
         */
        Mode_t Mode;

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
