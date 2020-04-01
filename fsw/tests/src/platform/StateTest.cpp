#include <unordered_map>

#include "State.hpp"
#include "DataVector.hpp"
#include "Errors.hpp"

#include "TestHelpers.hpp"

/**
 * Global DV config.
 */
static DataVector::Config_t gDvConfig =                      
{                                                    
    {DV_REG_TEST0,                               
    {                                            
        DV_ADD_INT16   ( DV_ELEM_TEST0,  0     ),
        DV_ADD_BOOL    ( DV_ELEM_TEST1,  false ),
        DV_ADD_UINT64  ( DV_ELEM_TEST2,  0     ),
    }},                                          
};                                                   

/******************************** TESTS ***************************************/

/* Tests to create States with data and successfully access the data */
TEST_GROUP (States) 
{
};

/* Create and access ID of State */
TEST (States, AccessID)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Create the State's ID
    StateId_t id = STATE_A;

    // Create the State
    Error_t ret = E_SUCCESS;
    State state = State (pDv, id, {}, {}, DV_ELEM_STATE, ret);
    CHECK_SUCCESS (ret);

    // Access the State's ID
    StateId_t result;
    CHECK_SUCCESS (state.getId (result));
    CHECK_TRUE (result == id);
}

/* Create and access Transitions of State */
TEST (States, AccessTransitions)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Create the State's ID and Transitions class
    StateId_t id = STATE_A;
    Transitions::Config_t transitionsA =
    {
        TR_CREATE_BOOL   ( DV_ELEM_TEST1,  CMP_EQUALS,        true,  STATE_B),
        TR_CREATE_UINT64 ( DV_ELEM_TEST2,  CMP_GREATER_THAN,  16,    STATE_C)
    };
    std::shared_ptr<Transitions> pT (nullptr);
    CHECK_SUCCESS (Transitions::createNew (transitionsA, pDv, pT));

    // Create the State
    Error_t ret = E_SUCCESS;
    State state = State (pDv, id, transitionsA, {}, DV_ELEM_STATE, ret);

    // Access the State's Transitions class
    std::shared_ptr<Transitions> pResult (nullptr);
    CHECK_SUCCESS (state.getTransitions (pResult));

    CHECK_TRUE (*pResult == *pT);
}

/* Create and access actions of State */
TEST (States, AccessActions)
{
    INIT_DATA_VECTOR (gDvConfig);

    // Create the State's ID and Actions class
    StateId_t id = STATE_A;
    Actions::Config_t actionsA =
    {
        {0 * Time::NS_IN_S,
            {
                ACT_CREATE_INT16  ( DV_ELEM_TEST0,  1    ),
                ACT_CREATE_BOOL   ( DV_ELEM_TEST1,  true ),
                ACT_CREATE_UINT64 ( DV_ELEM_TEST2,  1    )
            }},

        {.5 * Time::NS_IN_S,
            {
                ACT_CREATE_INT16  ( DV_ELEM_TEST0,  2     ),
                ACT_CREATE_BOOL   ( DV_ELEM_TEST1,  false ),
                ACT_CREATE_UINT64 ( DV_ELEM_TEST2,  2     )
            }},
    };
    std::shared_ptr<Actions> pA (nullptr);
    CHECK_SUCCESS (Actions::createNew (actionsA, pDv, DV_ELEM_STATE, pA));

    // Create the State
    Error_t ret = E_SUCCESS;
    State S = State (pDv, id, {}, actionsA, DV_ELEM_STATE, ret);

    // Access the State's Actions class
    std::shared_ptr<Actions> pResult (nullptr);
    CHECK_SUCCESS (S.getActions (pResult));
    CHECK_TRUE (*pResult == *pA);
}
