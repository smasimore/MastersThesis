/**
 * Actions class to manage the actions for a State. Each State has one Actions 
 * object, which contains a map from nanoseconds elapsed in state to a list of
 * actions to execute. 
 *
 * An action is comprised of a Data Vector element and value to set the element
 * to if the specified amount of time in the state has passed. While it is
 * possible that many actions may have no effect (e.g. writing a value to a 
 * sensor element that will be promptly overwritten), there is only 1 explicitly
 * disallowed action: writing to the Data Vector's state element. This is to
 * ensure a config does not attempt to change the State Machine's state via an
 * action. State changes are only possible through the SM->switchState method.
 * This is because changing state has side-actions that must occur (e.g. 
 * resetting the action iterator).
 *
 *                            ---- Config ----
 *
 * The order of the map does not matter, since the underlying data structure
 * sorts by time. However, it is required for readability that configs order by
 * elapsed time. When creating a Action using the ACT_CREATE_* macros, the type 
 * of the kTargetVal will go through *some* compile-time verification. For 
 * example, the following will not compile:
 *
 *   - A negative kTargetVal in the config when the target type is an unsigned 
 *     integer.
 *   - A kTargetVal like 1.23 in the config when the target type is any type but 
 *     a double or a float.
 *
 * However, be careful in ensuring that the hardcoded kTargetVal is the correct
 * type to avoid errors not caught at compile time, e.g.:
 *
 *    - A kTargetVal like true or false, will evaluate to 1 and 0 respectively 
 *      if the target elem is an integer.
 *    - A kTargetVal that is an integer with a boolean target elem will assign 
 *      to a boolean; 0 will evaluate to false, and any other value will turn 
 *      into true.
 *    - A kTargetValue of 2^33 for a 32-bit target elem.
 *
 */

#ifndef ACTION_HPP
#define ACTION_HPP

#include <string>
#include <memory>
#include <vector>
#include <iterator>
#include <map>

#include "Errors.hpp"
#include "Time.hpp"
#include "DataVector.hpp"

/*************************** HELPER MACROS FOR CONFIG ************************/

/**
 * Defines a Action on a Data Vector element of type DV_T_UINT8.
 */
#define ACT_CREATE_UINT8(kElem, kTargetVal)                                    \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<uint8_t> {                                             \
        kElem,                                                                 \
        DV_T_UINT8,                                                            \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_UINT16.
 */
#define ACT_CREATE_UINT16(kElem, kTargetVal)                                   \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<uint16_t> {                                            \
        kElem,                                                                 \
        DV_T_UINT16,                                                           \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_UINT32.
 */
#define ACT_CREATE_UINT32(kElem, kTargetVal)                                   \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<uint32_t> {                                            \
        kElem,                                                                 \
        DV_T_UINT32,                                                           \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_UINT64.
 */
#define ACT_CREATE_UINT64(kElem, kTargetVal)                                   \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<uint64_t> {                                            \
        kElem,                                                                 \
       DV_T_UINT64,                                                            \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_INT8.
 */
#define ACT_CREATE_INT8(kElem, kTargetVal)                                     \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<int8_t> {                                              \
        kElem,                                                                 \
        DV_T_INT8,                                                             \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_INT16.
 */
#define ACT_CREATE_INT16(kElem, kTargetVal)                                    \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<int16_t> {                                             \
        kElem,                                                                 \
        DV_T_INT16,                                                            \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_INT32.
 */
#define ACT_CREATE_INT32(kElem, kTargetVal)                                    \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<int32_t> {                                             \
        kElem,                                                                 \
        DV_T_INT32,                                                            \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_INT64.
 */
#define ACT_CREATE_INT64(kElem, kTargetVal)                                    \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<int64_t> {                                             \
        kElem,                                                                 \
        DV_T_INT64,                                                            \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_FLOAT.
 */
#define ACT_CREATE_FLOAT(kElem, kTargetVal)                                    \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<float> {                                               \
        kElem,                                                                 \
        DV_T_FLOAT,                                                            \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_DOUBLE.
 */
#define ACT_CREATE_DOUBLE(kElem, kTargetVal)                                   \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<double> {                                              \
        kElem,                                                                 \
        DV_T_DOUBLE,                                                           \
        kTargetVal,                                                            \
    })

/**
 * Defines a Action on a Data Vector kElement of type DV_T_BOOL.
 */
#define ACT_CREATE_BOOL(kElem, kTargetVal)                                     \
    std::shared_ptr<Actions::ActionBase>(                                      \
    new Actions::Action<bool> {                                                \
        kElem,                                                                 \
        DV_T_BOOL,                                                             \
        kTargetVal,                                                            \
    })

class Actions final
{
public:

    /**
     * Base struct that templated child extends. This is required so that the
     * Actions::Config_t can be a vector of a common type
     * (std::shared_ptr<Actions::ActionBase>).
     */
    struct ActionBase
    {

        /**
         * Data Vector element to act on.
         */
        DataVectorElement_t mElem;

        /**
         * Data Vector element type.
         */
        DataVectorElementType_t mType;

        /**
         * Constructor.
         *
         * @param  kElem         Data Vector element to act on.
         * @param  kType         Type of kElem.
         */
        ActionBase (DataVectorElement_t kElem, DataVectorElementType_t kType) : 
            mElem (kElem),
            mType (kType) { }

        /**
         * Execute action.
         *
         * @param  kPDataVector  Pointer to DV.
         *
         * @ret    [other]               Status returned by child struct.
         */
        virtual Error_t execute (std::shared_ptr<DataVector> kPDataVector) = 0;

        /** 
         * Destructor for superclass must be explicitly defined.
         */
        ~ActionBase () { };

    };

    /**
     * Templated config struct to represent an individual Action config
     */
    template<class T>
    struct Action : public ActionBase
    {

        /**
         * Value to set Data Vector element to.
         */
        T mTargetVal;

        /**
         * Constructor. Required to all for brace initialization of struct.
         *
         * @param  kElem         Data Vector element to act on.
         * @param  kType         Type of kElem.
         * @param  kTargetVal    Value to set Data Vector element to.
         */
        Action (DataVectorElement_t kElem, DataVectorElementType_t kType, 
                T kTargetVal) :
            ActionBase (kElem, kType),
            mTargetVal (kTargetVal) { }

        /**
         * Execute action. Writes target value to Data Vector elem.
         *
         * @param  kPDataVector          Pointer to DV.
         *
         * @ret    E_SUCCESS             Successfully executed action.
         *         E_DATA_VECTOR_WRITE   Failed to write to DV.
         */
        Error_t execute (std::shared_ptr<DataVector> kPDataVector)
        {
            Error_t ret = kPDataVector->write (mElem, mTargetVal);
            if (ret != E_SUCCESS)
            {
                return E_DATA_VECTOR_WRITE;
            }

            return E_SUCCESS;
        }
    };

    /**
     * Actions config. Defines map from time elapsed in State to set of actions
     * to execute. 
     */
    typedef std::map<Time::TimeNs_t, std::vector<std::shared_ptr<ActionBase>>> 
        Config_t;

    /**
     * Create a new Actions object.
     *
     * @param   kConfig             Actions config.
     * @param   kPDataVector        Pointer to DV.
     * @param   kStateElem          DV element storing current state.
     * @param   kPActionsRet        Param to store pointer to Actions object.
     *
     * @ret     E_SUCCESS           Successfully created the Actions object.
     *          E_DATA_VECTOR_NULL  Data Vector ptr null.
     *          E_INVALID_ELEM      Element not in Data Vector.
     *          E_DATA_VECTOR_READ  Failed to read type data from DV.
     *          E_INCORRECT_TYPE    Type mismatch between action and DV elem.
     */
    static Error_t createNew (const Config_t& kConfig,
                              std::shared_ptr<DataVector> kPDataVector,
                              DataVectorElement_t kStateElem,
                              std::shared_ptr<Actions>& kPActionsRet);

    /**
     * Verify config.
     *
     * @param   kStateElem          DV element storing current state.
     *
     * @ret     E_SUCCESS           Config valid.
     *          E_DATA_VECTOR_NULL  Data Vector ptr null.
     *          E_INVALID_ELEM      Element not in Data Vector.
     *          E_DATA_VECTOR_READ  Failed to read type data from DV.
     *          E_INCORRECT_TYPE    Type mismatch between action and DV elem.
     *          E_INVALID_ACTION    Cannot change State value using an action.
     */
    Error_t verifyConfig (DataVectorElement_t kStateElem);

    /**
     * Check for actions that should be executed and return to caller. Does not 
     * execute the actions.
     *
     * Note: kActionsToExecuteRet is cleared at the beginning of this function.
     *
     * @param   kTimeElapsedNs          Time elapsed in State.
     * @param   kActionsToExecuteRet    Vector to store actions to execute.
     *
     * @ret     E_SUCCESS               Successfully checked actions.
     */
    Error_t checkActions (Time::TimeNs_t kTimeElapsedNs,
                          std::vector<std::shared_ptr<ActionBase>>&
                          kActionsToExecuteRet);
   
    /**
     * Reset action iterator. This is used when the State Machine enters a new
     * state.
     *
     * @ret     E_SUCCESS    Action iterator reset successfully.
     */
    Error_t resetActionIterator ();

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF ACTIONS
     *
     * Check if two Actions objects have mTimeToActionsMap with the
     * same underlying ActionBase objects. Does not check attributes
     * specific to the Action struct, i.e. kTargetVal.
     *
     * @param  kRhs  Actions object to compare this object to.
     *
     * @ret    True if objects have same underlying ActionBase objects, false
     *         otherwise
     */
    bool operator== (Actions& kRhs);

private:

    /**
     * Constructor.
     *
     * @param   kConfig       Actions config.
     * @param   kPDataVector  Pointer to DV.
     */
    Actions (const Actions::Config_t& kConfig, 
             std::shared_ptr<DataVector> kPDataVector);

    /**
     * Map of time elapsed in State to list of actions to execute.
     */
    Config_t mTimeToActionsMap;

    /**
     * Pointer to Data Vector.
     */
    std::shared_ptr<DataVector> mPDataVector;

    /** 
     * Iterator to step through the action sequence.
     */
    Config_t::iterator mActionIter;

    /** 
    * End iterator to catch the end of the action sequence.
    */
    Config_t::iterator mActionEnd;


};

# endif
