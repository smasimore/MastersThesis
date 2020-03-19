/**
 * Transitions class to manage a State's list of transition conditions and
 * State to transition to if the condition is met. Every State has one 
 * Transitions class.
 *
 * An individual Transition includes a Data Vector element, comparison type, 
 * value to compare the value stored by the Data Vector element to, and State to 
 * transition to if the condition is met.
 *
 *                            ---- Config ----
 *
 * Transitions are checked in order. This means the first transition to have its 
 * condition met will be the transition executed by the State Machine.
 *
 * When creating a Transition using the TR_CREATE_* macros, the type of the 
 * kTargetVal will go through *some* compile-time verification. For example, the 
 * following will not compile:
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

# ifndef TRANSITION_HPP
# define TRANSITION_HPP

#include <stdint.h>
#include <memory>
#include <vector>

#include "Errors.hpp"
#include "StateMachineEnums.hpp"
#include "DataVector.hpp"
#include "DataVectorEnums.hpp"

/*************************** HELPER MACROS FOR CONFIG ************************/

/**
 * Defines a Transition for a Data Vector element of type DV_T_UINT8.
 */
#define TR_CREATE_UINT8(kElem, kComparison, kTargetVal, kTargetState)          \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<uint8_t> {                                     \
        kElem,                                                                 \
        DV_T_UINT8,                                                            \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_UINT16.
 */
#define TR_CREATE_UINT16(kElem, kComparison, kTargetVal, kTargetState)         \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<uint16_t> {                                    \
        kElem,                                                                 \
        DV_T_UINT16,                                                           \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_UINT32.
 */
#define TR_CREATE_UINT32(kElem, kComparison, kTargetVal, kTargetState)         \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<uint32_t> {                                    \
        kElem,                                                                 \
        DV_T_UINT32,                                                           \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_UINT64.
 */
#define TR_CREATE_UINT64(kElem, kComparison, kTargetVal, kTargetState)         \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<uint64_t> {                                    \
        kElem,                                                                 \
        DV_T_UINT64,                                                           \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_INT8.
 */
#define TR_CREATE_INT8(kElem, kComparison, kTargetVal, kTargetState)           \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<int8_t> {                                      \
        kElem,                                                                 \
        DV_T_INT8,                                                             \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_INT16.
 */
#define TR_CREATE_INT16(kElem, kComparison, kTargetVal, kTargetState)          \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<int16_t> {                                     \
        kElem,                                                                 \
        DV_T_INT16,                                                            \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_INT32.
 */
#define TR_CREATE_INT32(kElem, kComparison, kTargetVal, kTargetState)          \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<int32_t> {                                     \
        kElem,                                                                 \
        DV_T_INT32,                                                            \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_INT64.
 */
#define TR_CREATE_INT64(kElem, kComparison, kTargetVal, kTargetState)          \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<int64_t> {                                     \
        kElem,                                                                 \
        DV_T_INT64,                                                            \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_FLOAT.
 */
#define TR_CREATE_FLOAT(kElem, kComparison, kTargetVal, kTargetState)          \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<float> {                                       \
        kElem,                                                                 \
        DV_T_FLOAT,                                                            \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_DOUBLE.
 */
#define TR_CREATE_DOUBLE(kElem, kComparison, kTargetVal, kTargetState)         \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<double> {                                      \
        kElem,                                                                 \
        DV_T_DOUBLE,                                                           \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

/**
 * Defines a Transition for a Data Vector kElement of type DV_T_BOOL.
 */
#define TR_CREATE_BOOL(kElem, kComparison, kTargetVal, kTargetState)           \
    std::shared_ptr<Transitions::TransitionBase>(                              \
    new Transitions::Transition<bool> {                                        \
        kElem,                                                                 \
        DV_T_BOOL,                                                             \
        kComparison,                                                           \
        kTargetVal,                                                            \
        kTargetState                                                           \
    })

class Transitions final
{
public:

    /**
     * Base struct that templated child extends. This is required so that the
     * Transitions::Config_t can be a vector of a common type
     * (std::shared_ptr<Transitions::TransitionBase>).
     */
    struct TransitionBase
    {

        /**
         * Data Vector element to compare targetVal to.
         */
        DataVectorElement_t mElem;

        /**
         * Data Vector element type.
         */
        DataVectorElementType_t mType;

        /**
         * Comparison type to make between targetVal and the value stored at
         * mElem.
         */
        TransitionComparison_t mComparison;

        /**
         * State to transition to if transition condition met.
         */
        StateId_t mTargetState;

        /**
         * Constructor.
         *
         * @param  kElem         Data Vector element to check.
         * @param  kType         Type of kElem.
         * @param  kComparison   Comparison type.
         * @param  kTargetState  State to transition to if comparison true.
         */
        TransitionBase (DataVectorElement_t kElem, 
                        DataVectorElementType_t kType,
                        TransitionComparison_t kComparison,
                        StateId_t kTargetState) :
            mElem (kElem),
            mType (kType),
            mComparison (kComparison),
            mTargetState (kTargetState) { }

        /**
         * Check if transition condition has been met.
         *
         * @param  kPDataVector          Pointer to Data Vector.
         * @param  kShouldTransitionRet  Set to true if transition condition 
         *                               met.
         * @param  kTargetStateRet       State to transition to.
         *
         * @ret    [other]               Status returned by child struct.
         */
        virtual Error_t checkTransition (
                                     std::shared_ptr<DataVector> kPDataVector,
                                     bool& kShouldTransitionRet, 
                                     StateId_t& kTargetStateRet) = 0;

        /** 
         * Destructor for superclass must be explicitly defined.
         */
        ~TransitionBase () { };
    };

    /**
     * Templated child struct to represent an individual Transition.
     */
    template<class T>
    struct Transition : public TransitionBase
    {
        /**
         * Target value to compare to the value stored at mElem.
         */
        T mTargetVal;

        /** 
         * Constructor required to allow for brace initialization of struct.
         *
         * @param  kElem         Data Vector element to check.
         * @param  kType         Type of kElem.
         * @param  kComparison   Comparison type.
         * @param  kTargetVal    Value to compare kElem value to.
         * @param  kTargetState  State to transition to if comparison true.
         */
        Transition (DataVectorElement_t kElem,
                    DataVectorElementType_t kType,
                    TransitionComparison_t kComparison,
                    T kTargetVal,
                    StateId_t kTargetState) :
            TransitionBase (kElem, kType, kComparison, kTargetState),
            mTargetVal (kTargetVal) { }

        /**
         * Check if transition condition has been met. Defined in header to 
         * avoid having to explicitly define for each type.
         *
         * @param  kPDataVector          Pointer to Data Vector.
         * @param  kShouldTransitionRet  Set to true if transition condition 
         *                               met.
         * @param  kTargetStateRet       State to transition to.
         *
         * @ret    E_SUCCESS             Transition successfully checked.
         *         E_DATA_VECTOR_READ    Failed to read Data Vector.
         *         E_INVALID_ENUM        Invalid comparison enum.
         *
         */
        Error_t checkTransition (std::shared_ptr<DataVector> kPDataVector,
                                 bool& kShouldTransitionRet, 
                                 StateId_t& kTargetStateRet)
        {
            // Read current value from Data Vector.
            T value; 
            Error_t ret = kPDataVector->read (mElem, value);
            if (ret != E_SUCCESS)
            {
                return E_DATA_VECTOR_READ;
            }

            // Do comparison.
            bool shouldTransition = false;
            switch (mComparison)
            {
                case CMP_EQUALS:
                    shouldTransition = mTargetVal == value;
                    break;

                case CMP_GREATER_EQUALS_THAN:
                    shouldTransition = value >= mTargetVal;
                    break;

                case CMP_LESS_EQUALS_THAN:
                    shouldTransition = value <= mTargetVal;
                    break;

                case CMP_GREATER_THAN:
                    shouldTransition = value > mTargetVal;
                    break;

                case CMP_LESS_THAN:
                    shouldTransition = value < mTargetVal;
                    break;

                default:
                    return E_INVALID_ENUM;
            }

            // Set return params.
            if (shouldTransition == true)
            {
                kShouldTransitionRet = true;
                kTargetStateRet = mTargetState;
            }
            else
            {
                // No transition condition met.
                kShouldTransitionRet = false;
            }

            return E_SUCCESS;
        }
    };

    /**
     * Transitions config type.
     */
    typedef std::vector<std::shared_ptr<TransitionBase>> Config_t;

    /**
     * Create a new Transitions object.
     *
     * @param   kConfig             Transitions config.
     * @param   kPDataVector        Pointer to Data Vector.
     * @param   kPTransitionsRet    Var to store pointer to Transitions object.
     *
     * @ret     E_SUCCESS           Successfully created the Transitions class.
     *          E_DATA_VECTOR_NULL  Data Vector ptr null.
     *          E_INVALID_ENUM      Comparison or StateId enum invalid.
     *          E_INVALID_ELEM      Element not in Data Vector.
     *          E_DATA_VECTOR_READ  Failed to read type data from DV.
     *          E_INCORRECT_TYPE    Type mismatch between transition and DV 
     *                              elem.
     */
    static Error_t createNew (const Config_t& kConfig,
                              std::shared_ptr<DataVector> kPDataVector,
                              std::shared_ptr<Transitions>& kPTransitionsRet);

    /**
     * Check if any transition condition has been met. Returns first transition
     * where this is true.
     *
     * @param   kShouldTransitionRet  Set to true if a transition condition was 
     *                                met.
     * @param   kTargetStateRet       Set to the State ID of the target state
     *                                of the transition. 
     *
     * @ret     E_SUCCESS             Successfully checked for transitions.
     *          E_DATA_VECTOR_READ    Failed to read from DV.
     */
    Error_t checkTransitions (bool& kShouldTransitionRet, 
                              StateId_t& kTargetStateRet);

    /**
     * Verify config.
     *
     * @ret     E_SUCCESS           Config valid.
     *          E_DATA_VECTOR_NULL  Data Vector ptr null.
     *          E_INVALID_ENUM      Comparison or StateId enum invalid.
     *          E_INVALID_ELEM      Element not in Data Vector.
     *          E_DATA_VECTOR_READ  Failed to read type data from DV.
     *          E_INCORRECT_TYPE    Type mismatch between transition and DV 
     *                              elem.
     */
    Error_t verifyConfig ();

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF TESTS
     *
     * Check if two Transitions objects have mTransitionsList vectors with the
     * same underlying TransitionBase objects. Does not check attributes
     * specific to the Transition struct, i.e. kTargetVal.
     *
     * @param  kRhs  Transitions object to compare this object to.
     *
     * @ret    True if objects have same underlying ActionBase objects, false
     *         otherwise
     */
    bool operator== (Transitions& rhs); 

private:

    /**
     * Constructor.
     */
    Transitions (const Transitions::Config_t &kConfig, 
                 std::shared_ptr<DataVector> kPDataVector);

    /**
     * List of transitions.
     */
    std::vector<std::shared_ptr<TransitionBase>> mTransitionsList;

    /**
     * Data Vector.
     */
    std::shared_ptr<DataVector> mPDataVector;

};

# endif
