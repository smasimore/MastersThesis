/* All #include statements should come before the CppUTest include */
#include "Errors.h"
#include "StateVector.hpp"
#include "AvSWTestMacros.hpp"

#include "CppUTest/TestHarness.h"

/********************************* TESTS **************************************/

TEST_GROUP (StateVector)
{

};

/* Test initializing with empty config. */
TEST (StateVector, Create_EmptyConfig)
{
    StateVector::StateVectorConfig_t config = {};
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_EMPTY_CONFIG);
}

/* Test initializing with element list empty. */
TEST (StateVector, Create_EmptyElementList)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_LAST, 

            // Elements
            {}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_EMPTY_ELEMS);
}

/* Test initializing with invalid region enum. */
TEST (StateVector, Create_InvalidRegionEnum)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_LAST,

            // Elements
            // ELEM             TYPE                                        INITIAL_VALUE
            {{SV_ELEM_TEST0,   T_UINT8,    StateVector::toUInt64<uint8_t> (       0       )}}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_INVALID_ENUM);
}

/* Test initializing with invalid element enum. */
TEST (StateVector, Create_InvalidElemEnum)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM             TYPE                                        INITIAL_VALUE
            {{SV_ELEM_LAST,    T_UINT8,    StateVector::toUInt64<uint8_t> (       0       )}}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_INVALID_ENUM);
}

/* Test initializing with invalid type enum. */
TEST (StateVector, Create_InvalidTypeEnum)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM              TYPE                                       INITIAL_VALUE
            {{SV_ELEM_LAST,     T_LAST,    StateVector::toUInt64<uint8_t> (       0       )}}}


            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_INVALID_ENUM);
}


/* Test initializing with duplicate region name. */
TEST (StateVector, Create_DuplicateRegion)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST0,    T_UINT8,    StateVector::toUInt64<uint8_t> (       0       )},
             {SV_ELEM_TEST1,    T_BOOL,     StateVector::toUInt64<bool>    (       1       )}}},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST2,   T_FLOAT,    StateVector::toUInt64<float>   (       1.23    )}}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_DUPLICATE_REGION);
}

/* Test initializing with duplicate element name in different region. */
TEST (StateVector, Create_DuplicateElementDiffRegion)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST0,    T_UINT8,    StateVector::toUInt64<uint8_t> (       0       )},
            { SV_ELEM_TEST1,    T_BOOL,     StateVector::toUInt64<bool>    (       1       )}}},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{ SV_ELEM_TEST0,   T_FLOAT,    StateVector::toUInt64<float>   (       1.23    )}}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_DUPLICATE_ELEM);
}

/* Test initializing with duplicate element name in same region. */
TEST (StateVector, Create_DuplicateElementSameRegion)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST0,    T_UINT8,    StateVector::toUInt64<uint8_t> (       0       )},
             {SV_ELEM_TEST0,    T_BOOL,     StateVector::toUInt64<bool>    (       1       )}}},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST2,   T_FLOAT,    StateVector::toUInt64<float>    (       1.23    )}}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_DUPLICATE_ELEM);
}

/* Test initializing with a valid config. */
TEST (StateVector, Create_Success)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST0,    T_UINT8,    StateVector::toUInt64<uint8_t> (       0       )},
             {SV_ELEM_TEST1,    T_BOOL,     StateVector::toUInt64<bool>    (       1       )}}},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            // ELEM             TYPE                                         INITIAL_VALUE
            {{SV_ELEM_TEST2,    T_FLOAT,    StateVector::toUInt64<float>   (       1.23    )}}}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<StateVector> pSv; 
    CHECK_SUCCESS (StateVector::createNew (config, pSv));
}

