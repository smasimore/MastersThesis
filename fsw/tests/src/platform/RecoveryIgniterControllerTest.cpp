#include "DataVector.hpp"
#include "RecoveryIgniterController.hpp"

#include "TestHelpers.hpp"

/**
 * Initializes DV and creates a correct controller config.
 */
#define INIT_DV_AND_CTRL                                                       \
    std::shared_ptr<DataVector> pDv = nullptr;                                 \
    CHECK_SUCCESS (DataVector::createNew (gRecIgnTestDvConfig, pDv));          \
    std::unique_ptr<RecoveryIgniterController> pCont = nullptr;                \
    RecoveryIgniterController::Config_t contConfig =                           \
    {                                                                          \
        DV_ELEM_DEPLOY_DROG0_CMD,                                              \
        DV_ELEM_DEPLOY_DROG0_TIME_NS,                                          \
        DV_ELEM_CN_TIME_NS,                                                    \
        DV_ELEM_DROG0_IGN0_CTRL,                                               \
        DV_ELEM_RECOVERY_ARMED,                                                \
        5,                                                                     \
        10                                                                     \
    };

/**
 * Verifies that deployment either has or has not occurred by checking both
 * the igniter control and deployment time DV elems.
 *
 * @param   kDep True if deployment should have occurred, false otherwise.
 */
#define CHECK_DEPLOYED(kDep)                                                   \
{                                                                              \
    Time::TimeNs_t depTimeNs = 0;                                              \
    CHECK_SUCCESS (pDv->read (DV_ELEM_DEPLOY_DROG0_TIME_NS, depTimeNs));       \
    CHECK_TRUE (kDep ? depTimeNs > 0 : depTimeNs == 0);                        \
    bool igniterControlVal;                                                    \
    CHECK_SUCCESS (pDv->read (DV_ELEM_DROG0_IGN0_CTRL, igniterControlVal));    \
    CHECK_EQUAL (kDep, igniterControlVal);                                     \
}

/**
 * DV config used for all RecoveryIgniterController tests. Contains all elements
 * necessary to control one igniter.
 */
static DataVector::Config_t gRecIgnTestDvConfig =
{
    // Region
    {DV_REG_TEST0,

    // Elements
    {
        DV_ADD_UINT64 (DV_ELEM_CN_TIME_NS,           0                 ),
        DV_ADD_BOOL   (DV_ELEM_DEPLOY_DROG0_CMD,     false             ),
        DV_ADD_UINT8  (DV_ELEM_REC_CTRL_DROG0_MODE,  Mode_t::MODE_SAFED),
        DV_ADD_BOOL   (DV_ELEM_RECOVERY_ARMED,       false             ),
        DV_ADD_BOOL   (DV_ELEM_DROG0_IGN0_CTRL,      false             ),
        DV_ADD_UINT64 (DV_ELEM_DEPLOY_DROG0_TIME_NS, false             ),
        DV_ADD_BOOL   (DV_ELEM_DROG0_IGN0_FB,        false             ),
    }},
};

TEST_GROUP(RecoveryIgniterControllerTest)
{
};

/**
 * Constructor rejects invalid configs.
 */
TEST(RecoveryIgniterControllerTest, BadConfig)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Reject if a config DV elem is absent from the DV.
    contConfig.depCommandElem = DV_ELEM_TEST0;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_INVALID_ELEM, err);
    contConfig.depCommandElem = DV_ELEM_DEPLOY_DROG0_CMD;

    contConfig.tDepTimeElem = DV_ELEM_TEST0;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_INVALID_ELEM, err);
    contConfig.tDepTimeElem = DV_ELEM_DEPLOY_DROG0_TIME_NS;

    contConfig.missionTimeElem = DV_ELEM_TEST0;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_INVALID_ELEM, err);
    contConfig.missionTimeElem = DV_ELEM_CN_TIME_NS;

    contConfig.igniterControlElem = DV_ELEM_TEST0;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_INVALID_ELEM, err);
    contConfig.igniterControlElem = DV_ELEM_DROG0_IGN0_CTRL;

    contConfig.recArmedElem = DV_ELEM_TEST0;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_INVALID_ELEM, err);
    contConfig.recArmedElem = DV_ELEM_RECOVERY_ARMED;

    // Reject if reversed bounds.
    contConfig.tDepBoundLowNs = 10;
    contConfig.tDepBoundHighNs = 5;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_OUT_OF_BOUNDS, err);

    // Reject if equal bounds.
    contConfig.tDepBoundLowNs = 10;
    contConfig.tDepBoundHighNs = 10;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_OUT_OF_BOUNDS, err);

    // Reject if zero lower bound.
    contConfig.tDepBoundLowNs = 0;
    contConfig.tDepBoundHighNs = 5;
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_EQUAL (E_OUT_OF_BOUNDS, err);
}

/**
 * Controller will not trigger deployment if commanded to before lower bound.
 */
TEST(RecoveryIgniterControllerTest, DepCmdBeforeLowBound)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Enable and arm controller.
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_SUCCESS (err);
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                               (uint8_t) Mode_t::MODE_ENABLED));
    CHECK_SUCCESS (pDv->write (DV_ELEM_RECOVERY_ARMED, true));

    // Controller has not been commanded in any way, does not deploy.
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (false);

    // Command deployment before lower bound. Controller does not deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_DEPLOY_DROG0_CMD, true));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (false);
}

/**
 * Controller automatically triggers deployment after upper bound.
 */
TEST(RecoveryIgniterControllerTest, AutoDepAfterUpperBound)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Enable and arm controller.
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_SUCCESS (err);
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                   (uint8_t) Mode_t::MODE_ENABLED));
    CHECK_SUCCESS (pDv->write (DV_ELEM_RECOVERY_ARMED, true));

    // Time is within bounds, does not trigger auto deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) 7));
    CHECK_SUCCESS (pCont-> run ());
    CHECK_DEPLOYED (false);

    // Time exceeds upper bound, triggers auto deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) 11));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (true);
}

/**
 * Controller does not deploy even under command and timeout conditions if it
 * is not armed.
 */
TEST(RecoveryIgniterControllerTest, DisarmamentPrecludesDeployment)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Enable controller but do not arm.
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_SUCCESS (err);
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                   (uint8_t) Mode_t::MODE_ENABLED));

    // Deployment is commanded within bounds but controller does not deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_DEPLOY_DROG0_CMD, true));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) 7));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (false);

    // Upper bound passes, controller still does not deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) 11));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (false);
}

/**
 * Enabled and armed controller deploys when commanded within time bounds.
 */
TEST(RecoveryIgniterControllerTest, NominalDeployment)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Enable and arm controller.
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_SUCCESS (err);
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                   (uint8_t) Mode_t::MODE_ENABLED));
    CHECK_SUCCESS (pDv->write (DV_ELEM_RECOVERY_ARMED, true));

    // Deployment is commanded within bounds, controller deploys.
    Time::TimeNs_t t = 7;
    CHECK_SUCCESS (pDv->write (DV_ELEM_DEPLOY_DROG0_CMD, true));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) t));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (true);

    // Controller timestamps the deployment correctly.
    Time::TimeNs_t tDep = 0;
    CHECK_SUCCESS (pDv->read (DV_ELEM_DEPLOY_DROG0_TIME_NS, tDep));
    CHECK_EQUAL (t, tDep);

    // Just before ignition timeout, igniter is still on.
    t += RecoveryIgniterController::IGNITION_DURATION_NS - 1;
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, t));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (true);
    
    // Once sufficient time elapses, igniter is disabled.
    t += 1;
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, t));
    CHECK_SUCCESS (pCont->run ());

    bool igniterControlVal;
    CHECK_SUCCESS (pDv->read (DV_ELEM_DROG0_IGN0_CTRL, igniterControlVal));
    CHECK_EQUAL (false, igniterControlVal);
}

/**
 * Disarming the recovery system while the controller is enabled and the igniter
 * is active will disable the igniter.
 */
TEST(RecoveryIgniterControllerTest, RecDisarmDisablesIgniter)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Enable and arm controller.
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_SUCCESS (err);
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                   (uint8_t) Mode_t::MODE_ENABLED));
    CHECK_SUCCESS (pDv->write (DV_ELEM_RECOVERY_ARMED, true));

    // Deployment is commanded within bounds, controller deploys.
    Time::TimeNs_t t = 7;
    CHECK_SUCCESS (pDv->write (DV_ELEM_DEPLOY_DROG0_CMD, true));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) t));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (true);

    // Disable the recovery system and rerun the controller. Igniter should be
    // disabled afterwards.
    CHECK_SUCCESS (pDv->write (DV_ELEM_RECOVERY_ARMED, false));
    CHECK_SUCCESS (pCont->run ());

    bool igniterControlVal;
    CHECK_SUCCESS (pDv->read (DV_ELEM_DROG0_IGN0_CTRL, igniterControlVal));
    CHECK_EQUAL (false, igniterControlVal);
}

/**
 * Controller does not deploy if safed. A controller that has already deployed
 * and is then safed will disable the igniter.
 */
TEST(RecoveryIgniterControllerTest, SafedBehavior)
{
    INIT_DV_AND_CTRL;
    Error_t err;

    // Arm controller but do not enable.
    err = RecoveryIgniterController::createNew (
        contConfig, pDv, DV_ELEM_REC_CTRL_DROG0_MODE, pCont);
    CHECK_SUCCESS (err);
    CHECK_SUCCESS (pDv->write (DV_ELEM_RECOVERY_ARMED, true));

    // Deployment is commanded within bounds but controller does not deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_DEPLOY_DROG0_CMD, true));
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) 7));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (false);

    // Upper bound passes, controller still does not deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_CN_TIME_NS, (uint64_t) 11));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (false);

    // Now enable controller and allow to deploy.
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                   (uint8_t) Mode_t::MODE_ENABLED));
    CHECK_SUCCESS (pCont->run ());
    CHECK_DEPLOYED (true);

    // Safe controller again, should disable igniter.
    CHECK_SUCCESS (pDv->write (DV_ELEM_REC_CTRL_DROG0_MODE,
                   (uint8_t) Mode_t::MODE_SAFED));
    CHECK_SUCCESS (pCont->run ());

    bool igniterControlVal;
    CHECK_SUCCESS (pDv->read (DV_ELEM_DROG0_IGN0_CTRL,
                              igniterControlVal));
    CHECK_EQUAL (false, igniterControlVal);
}