/* All #include statements should come before the CppUTest include */
#include "Errors.h"
#include "StateVector.hpp"
#include "AvSWTestMacros.hpp"

#include "CppUTest/TestHarness.h"

/* Group of tests verifying verifyConfig method. */
TEST_GROUP (StateVectorConfig)
{

};

/* Test initializing with empty config. */
TEST (StateVectorConfig, Create_EmptyConfig)
{
    StateVector::StateVectorConfig_t config = {};
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_EMPTY_CONFIG);
}

/* Test initializing with element list empty. */
TEST (StateVectorConfig, Create_EmptyElementList)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_LAST, 

            // Elements
            {
            }},

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_EMPTY_ELEMS);
}

/* Test initializing with invalid region enum. */
TEST (StateVectorConfig, Create_InvalidRegionEnum)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_LAST,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            )
            }},

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_INVALID_ENUM);
}

/* Test initializing with invalid element enum. */
TEST (StateVectorConfig, Create_InvalidElemEnum)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_LAST,             0            )
            }},

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_INVALID_ENUM);
}

/* Test initializing with duplicate region name. */
TEST (StateVectorConfig, Create_DuplicateRegion)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            ),
                SV_ADD_BOOL   (           SV_ELEM_TEST1,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_FLOAT  (           SV_ELEM_TEST2,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_DUPLICATE_REGION);
}

/* Test initializing with duplicate element name in different region. */
TEST (StateVectorConfig, Create_DuplicateElementDiffRegion)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            ),
                SV_ADD_BOOL   (           SV_ELEM_TEST1,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_FLOAT  (           SV_ELEM_TEST0,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_DUPLICATE_ELEM);
}

/* Test initializing with duplicate element name in same region. */
TEST (StateVectorConfig, Create_DuplicateElementSameRegion)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            ),
                SV_ADD_BOOL   (           SV_ELEM_TEST0,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_FLOAT  (           SV_ELEM_TEST2,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_DUPLICATE_ELEM);
}

/* Test initializing with a valid config. */
TEST (StateVectorConfig, Create_Success)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            ),
                SV_ADD_BOOL   (           SV_ELEM_TEST1,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_FLOAT  (           SV_ELEM_TEST2,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_SUCCESS (StateVector::createNew (config, pSv));
}
