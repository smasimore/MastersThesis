#include "Errors.hpp"
#include "Log.hpp"
#include "Controller.hpp"

/* Logs used for testing. */
extern Log *gPExpectedLog;
extern Log *gPTestLog;

class TestController final : public Controller
{

    public:

        /* Controller configuration. */
        typedef struct Config
        {
            bool valid; /* Whether config is valid or not. */
        } Config_t;

        /**
         * Inherited from base.
         */
        virtual Error_t runEnabled ();

        /**
         * Inherited from base.
         */
        virtual Error_t runSafed ();

        /**
         * Inherited from base.
         */
        virtual Error_t verifyConfig ();

        /**
         * Derived controller constructor. This must be public so that it is
         * visible to Controller::createNew.
         *
         * @param   kConfig        Controller configuration.
         * @param   kDataVector    Ptr to initialized Data Vector for this 
         *                         node.
         * @param   kDvModeElem    Data Vector element to read to determine 
         *                         controller's mode.
         */
        TestController (Config_t kConfig, 
                        std::shared_ptr<DataVector> kPDataVector,
                        DataVectorElement_t kDvModeElem);

    private:

        /* Controller configuration. */
        Config_t mConfig;
};
