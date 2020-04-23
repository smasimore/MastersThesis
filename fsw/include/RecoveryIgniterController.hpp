/**
 * A RecoveryIgniterController controls a single recovery igniter. The
 * controller "listens" for a boolean DV element to flip from false to true
 * before triggering deployment. Deployment is then triggered by raising a
 * control value in the DV that is being listened to by an igniter DIO device.
 *
 * The controller itself can also trigger deployment if the elapsed mission time
 * exceeds some upper bound. A lower bound provides a minimum mission time that
 * must elapse before deployment is allowed. Deployment is also disallowed if
 * the recovery system is disarmed, as indicated by the DV. For deployment to be
 * possible, the controller must be enabled and the recovery system must be
 * armed.
 *
 * When the controller is safed, it lowers the igniter DIO line. When the
 * controller is enabled, it lowers the igniter DIO line some time after
 * ignition or if the recovery system is disarmed.
 */

#ifndef RECOVERY_IGNITER_CONTROLLER_HPP
#define RECOVERY_IGNITER_CONTROLLER_HPP

#include "Controller.hpp"
#include "Time.hpp"

class RecoveryIgniterController final : public Controller
{
public:
    /**
     * Controller config.
     */
    typedef struct
    {
        /**
         * DV element that commands the controller to deploy.
         * Type: DV_T_BOOL
         */
        DataVectorElement_t depCommandElem;
        /**
         * DV element where the controller stores the time at which it
         * triggered deployment.
         * Type: DV_T_UINT64
         */
        DataVectorElement_t tDepTimeElem;
        /**
         * DV element holding current mission time.
         * Type: DV_T_UINT64
         */
        DataVectorElement_t missionTimeElem;
        /**
         * Control value for igniter digital device.
         * Type: DV_T_BOOL
         */
        DataVectorElement_t igniterControlElem;
        /**
         * DV element for recovery system armed. This must be true for
         * deployment to be possible.
         * Type: DV_T_BOOL
         */
        DataVectorElement_t recArmedElem;
        /**
         * Lower mission time bound on the deployment window in nanoseconds.
         * Must be lower than tDepBoundHighS and greater than 0.
         */
        Time::TimeNs_t tDepBoundLowNs;
        /**
         * Upper mission time bound on the deployment window in nanoseconds.
         */
        Time::TimeNs_t tDepBoundHighNs;

    } Config_t;

    /**
     * Length of time following ignition after which the igniter is
     * automatically turned off.
     */
    static const Time::TimeNs_t IGNITION_DURATION_NS = 5 * Time::NS_IN_S;

    /**
     * Controller evaluates deployment conditions and controls the igniter
     * appropriately. Some time after ignition, the igniter is turned off.
     *
     * @ret     E_SUCCESS           Success.
     *          E_DATA_VECTOR_READ  Failed to read from DV.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     */
    Error_t runEnabled ();

    /**
     * Controller turns off the igniter device while safed.
     *
     * @ret     E_SUCCESS           Success.
     *          E_DATA_VECTOR_WRITE Failed to write to DV.
     */
    Error_t runSafed ();

    /**
     * Verifies controller config.
     *
     * @ret     E_SUCCESS       Configuration is valid.
     *          E_INVALID_ELEM  A DV element does not exist in the DV provided.
     *          E_OUT_OF_BOUNDS Deployment time bounds were reversed or zero.
     */
    Error_t verifyConfig ();

    /**
     * Configures a new controller.
     *
     * @param   kConfig     Controller configuration.
     * @param   kPDv        Node data vector.
     * @param   kDvModeElem DV elem for controller mode.
     */
    RecoveryIgniterController (Config_t& kConfig,
                               std::shared_ptr<DataVector> kPDv,
                               DataVectorElement_t kDvModeElem);

private:
    /**
     * Controller configuration.
     */
    const Config_t mCONFIG;
};

#endif
