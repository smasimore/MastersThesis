#include "Errors.h"
#include "Log.hpp"
#include "Controller.hpp"

/* Logs used for testing. */
extern Log *PExpectedLog;
extern Log *PTestLog;

class TestController final : public Controller
{

    public:

        /* Controller configuration. */
        struct TestConfig
        {
            bool valid; /* Whether config is valid or not. */
        };

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
         * @param     config        Controller configuration.
         */
        TestController (struct TestConfig config);

    private:

        /* Controller configuration. */
        struct TestConfig Config;
};
