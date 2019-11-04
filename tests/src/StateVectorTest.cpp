/* All #include statements should come before the CppUTest include */
#include <cstring>
#include <sstream>
#include <limits>

#include "Errors.h"
#include "StateVector.hpp"
#include "AvSWTestMacros.hpp"

#include "CppUTest/TestHarness.h"

/* Group of tests verifying verifyConfig method. */
TEST_GROUP (StateVectorVerifyConfig)
{

};

/* Test initializing with empty config. */
TEST (StateVectorVerifyConfig, EmptyConfig)
{
    StateVector::StateVectorConfig_t config = {};
    std::shared_ptr<StateVector> pSv; 
    CHECK_ERROR (StateVector::createNew (config, pSv), E_EMPTY_CONFIG);
}

/* Test initializing with element list empty. */
TEST (StateVectorVerifyConfig, EmptyElementList)
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
TEST (StateVectorVerifyConfig, InvalidRegionEnum)
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
TEST (StateVectorVerifyConfig, InvalidElemEnum)
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
TEST (StateVectorVerifyConfig, DuplicateRegion)
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
TEST (StateVectorVerifyConfig, DuplicateElementDiffRegion)
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
TEST (StateVectorVerifyConfig, DuplicateElementSameRegion)
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
TEST (StateVectorVerifyConfig, Success)
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

/* Group of tests to verify State Vector's underlying buffer. */
TEST_GROUP (StateVector)
{

};

/* Test constructing State Vector with 1 element. */
TEST (StateVector, Construct_1Elem_TypesAndBoundaryVals)
{
    typedef struct ConstructTestCase
    {
        StateVectorElementType_t type;
        uint64_t initialVal;
        std::vector<uint8_t> expectedBuf;
    } ConstructTestCase_t;

    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                SV_ADD_UINT8  (           SV_ELEM_TEST0,            0            )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };

    std::vector<ConstructTestCase_t> testCases =
    {
    // SUB-TEST     TYPE                                                   INITIAL_VALUE                                        EXPECTED_BUFFER
       /* 0  */  {SV_T_UINT8,   StateVector::toUInt64<uint8_t>  ( std::numeric_limits<uint8_t> ::min ()      ),  {0x00}                                           },
       /* 1  */  {SV_T_UINT8,   StateVector::toUInt64<uint8_t>  (  1                                         ),  {0x01}                                           },
       /* 2  */  {SV_T_UINT8,   StateVector::toUInt64<uint8_t>  ( std::numeric_limits<uint8_t> ::max ()      ),  {0xff}                                           },
       /* 3  */  {SV_T_UINT16,  StateVector::toUInt64<uint16_t> ( std::numeric_limits<uint16_t>::min ()      ),  {0x00, 0x00}                                     },
       /* 4  */  {SV_T_UINT16,  StateVector::toUInt64<uint16_t> (  1                                         ),  {0x01, 0x00}                                     },
       /* 5  */  {SV_T_UINT16,  StateVector::toUInt64<uint16_t> ( std::numeric_limits<uint16_t>::max ()      ),  {0xff, 0xff}                                     },
       /* 6  */  {SV_T_UINT32,  StateVector::toUInt64<uint32_t> ( std::numeric_limits<uint32_t>::min ()      ),  {0x00, 0x00, 0x00, 0x00}                         },
       /* 7  */  {SV_T_UINT32,  StateVector::toUInt64<uint32_t> (  1                                         ),  {0x01, 0x00, 0x00, 0x00}                         },
       /* 8  */  {SV_T_UINT32,  StateVector::toUInt64<uint32_t> ( std::numeric_limits<uint32_t>::max ()      ),  {0xff, 0xff, 0xff, 0xff}                         },
       /* 9  */  {SV_T_UINT64,  StateVector::toUInt64<uint64_t> ( std::numeric_limits<uint64_t>::min ()      ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 10 */  {SV_T_UINT64,  StateVector::toUInt64<uint64_t> (  1                                         ),  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 11 */  {SV_T_UINT64,  StateVector::toUInt64<uint64_t> ( std::numeric_limits<uint64_t>::max ()      ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff} },
       /* 12 */  {SV_T_INT8,    StateVector::toUInt64<int8_t>   ( std::numeric_limits<int8_t>  ::min ()      ),  {0x80}                                           },
       /* 13 */  {SV_T_INT8,    StateVector::toUInt64<int8_t>   ( -1                                         ),  {0xff}                                           },
       /* 14 */  {SV_T_INT8,    StateVector::toUInt64<int8_t>   (  0                                         ),  {0x00}                                           },
       /* 15 */  {SV_T_INT8,    StateVector::toUInt64<int8_t>   (  1                                         ),  {0x01}                                           },
       /* 16 */  {SV_T_INT8,    StateVector::toUInt64<int8_t>   ( std::numeric_limits<int8_t>  ::max ()      ),  {0x7f}                                           },
       /* 17 */  {SV_T_INT16,   StateVector::toUInt64<int16_t>  ( std::numeric_limits<int16_t> ::min ()      ),  {0x00, 0x80}                                     },
       /* 18 */  {SV_T_INT16,   StateVector::toUInt64<int16_t>  ( -1                                         ),  {0xff, 0xff}                                     },
       /* 19 */  {SV_T_INT16,   StateVector::toUInt64<int16_t>  (  0                                         ),  {0x00, 0x00}                                     },
       /* 20 */  {SV_T_INT16,   StateVector::toUInt64<int16_t>  (  1                                         ),  {0x01, 0x00}                                     },
       /* 21 */  {SV_T_INT16,   StateVector::toUInt64<int16_t>  ( std::numeric_limits<int16_t> ::max ()      ),  {0xff, 0x7f}                                     },
       /* 22 */  {SV_T_INT32,   StateVector::toUInt64<int32_t>  ( std::numeric_limits<int32_t> ::min ()      ),  {0x00, 0x00, 0x00, 0x80}                         },
       /* 23 */  {SV_T_INT32,   StateVector::toUInt64<int32_t>  ( -1                                         ),  {0xff, 0xff, 0xff, 0xff}                         },
       /* 24 */  {SV_T_INT32,   StateVector::toUInt64<int32_t>  (  0                                         ),  {0x00, 0x00, 0x00, 0x00}                         },
       /* 25 */  {SV_T_INT32,   StateVector::toUInt64<int32_t>  (  1                                         ),  {0x01, 0x00, 0x00, 0x00}                         },
       /* 26 */  {SV_T_INT32,   StateVector::toUInt64<int32_t>  ( std::numeric_limits<int32_t> ::max ()      ),  {0xff, 0xff, 0xff, 0x7f}                         },
       /* 27 */  {SV_T_INT64,   StateVector::toUInt64<int64_t>  ( std::numeric_limits<int64_t> ::min ()      ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80} },
       /* 28 */  {SV_T_INT64,   StateVector::toUInt64<int64_t>  ( -1                                         ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff} },
       /* 29 */  {SV_T_INT64,   StateVector::toUInt64<int64_t>  (  0                                         ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 30 */  {SV_T_INT64,   StateVector::toUInt64<int64_t>  (  1                                         ),  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 31 */  {SV_T_INT64,   StateVector::toUInt64<int64_t>  ( std::numeric_limits<int64_t> ::max ()      ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f} },
       /* 32 */  {SV_T_FLOAT,   StateVector::toUInt64<float>    ( std::numeric_limits<float>   ::min ()      ),  {0x00, 0x00, 0x80, 0x00}                         },
       /* 33 */  {SV_T_FLOAT,   StateVector::toUInt64<float>    (  0                                         ),  {0x00, 0x00, 0x00, 0x00}                         },
       /* 34 */  {SV_T_FLOAT,   StateVector::toUInt64<float>    (  37.81999                                  ),  {0xab, 0x47, 0x17, 0x42}                         },
       /* 35 */  {SV_T_FLOAT,   StateVector::toUInt64<float>    ( -37.81999                                  ),  {0xab, 0x47, 0x17, 0xc2}                         },
       /* 36 */  {SV_T_FLOAT,   StateVector::toUInt64<float>    ( std::numeric_limits<float>   ::max ()      ),  {0xff, 0xff, 0x7f, 0x7f}                         },
       /* 37 */  {SV_T_FLOAT,   StateVector::toUInt64<float>    ( std::numeric_limits<float>   ::infinity () ),  {0x00, 0x00, 0x80, 0x7f}                         },
       /* 38 */  {SV_T_DOUBLE,  StateVector::toUInt64<double>   ( std::numeric_limits<double>  ::min ()      ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00} },
       /* 39 */  {SV_T_DOUBLE,  StateVector::toUInt64<double>   (  0                                         ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 40 */  {SV_T_DOUBLE,  StateVector::toUInt64<double>   (  37.81999                                  ),  {0x05, 0x86, 0xac, 0x6e, 0xf5, 0xe8, 0x42, 0x40} },
       /* 41 */  {SV_T_DOUBLE,  StateVector::toUInt64<double>   ( -37.81999                                  ),  {0x05, 0x86, 0xac, 0x6e, 0xf5, 0xe8, 0x42, 0xc0} },
       /* 42 */  {SV_T_DOUBLE,  StateVector::toUInt64<double>   ( std::numeric_limits<double>  ::max ()      ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x7f} },
       /* 43 */  {SV_T_DOUBLE,  StateVector::toUInt64<double>   ( std::numeric_limits<double>  ::infinity () ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f} },
       /* 44 */  {SV_T_BOOL,    StateVector::toUInt64<bool>     (  false                                     ),  {0x00}                                           },
       /* 45 */  {SV_T_BOOL,    StateVector::toUInt64<bool>     (  true                                      ),  {0x01}                                           },
    };

    // Loop through each test case, modify config, create State Vector with 
    // single element, and verify.
    for (uint8_t i = 0; i < testCases.size (); i++)
    {
        ConstructTestCase_t testCase = testCases[i];

        // Modify config for test case.
        config[0].elems[0].type       = testCase.type;
        config[0].elems[0].initialVal = testCase.initialVal;

        // Create SV
        std::shared_ptr<StateVector> pSv; 
        CHECK_SUCCESS (StateVector::createNew (config, pSv));

        // Get State Vector info.
        StateVector::StateVectorInfo_t stateVectorInfo;
        pSv->getStateVectorInfo (stateVectorInfo);

        // Get region info.
        StateVector::RegionInfo_t regionInfo;
        pSv->getRegionInfo (SV_REG_TEST0, regionInfo);

        // Verify SV pStart is the same as region's pStart.
        CHECK (regionInfo.pStart == stateVectorInfo.pStart);

        // Verify SV and Region size matches expected size.
        uint32_t actualStateVectorSizeBytes = stateVectorInfo.sizeBytes;
        uint32_t actualRegionSizeBytes = regionInfo.sizeBytes;
        uint32_t expectedSizeBytes = testCase.expectedBuf.size ();
        if (actualStateVectorSizeBytes != expectedSizeBytes ||
            actualRegionSizeBytes != expectedSizeBytes)
        {
            std::stringstream stream;
            stream << "-- Sub-test " << static_cast<int> (i) << " failed --" << 
                std::endl;
            stream << "Expected Size Bytes:  " << expectedSizeBytes << 
                std::endl;
            stream << "Actual SV Size Bytes: " << actualStateVectorSizeBytes <<
                std::endl;
            stream << "Actual Region Size Bytes: " << actualRegionSizeBytes <<
                std::endl;

            FAIL (stream.str ().c_str ());
        }

        // Verify SV's underlying buffer matches expected data.
        int cmpRet = std::memcmp (regionInfo.pStart, 
                                  testCase.expectedBuf.data (),
                                  actualStateVectorSizeBytes);
        if (cmpRet != 0)
        {
            uint64_t expected = 0;
            uint64_t actual = 0;
            std::memcpy (&expected, testCase.expectedBuf.data (), 
                         expectedSizeBytes);
            std::memcpy (&actual, regionInfo.pStart, expectedSizeBytes);
  
            std::stringstream stream;
            stream << "-- Sub-test " << static_cast<int> (i) << " failed --" << 
                std::endl;
            stream << "Expected Buffer: " << "0x" << std::hex << expected <<
                std::endl;
            stream << "Actual Buffer:   " << "0x" << std::hex << actual 
                << std::endl;

            FAIL (stream.str ().c_str ());
        }
    }
}

/* Test constructing State Vector with multiple elements. */
TEST (StateVector, Construct_MultipleElem_TypesAndBoundaryVals)
{
    StateVector::StateVectorConfig_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST0,

            // Elements
            //      TYPE               ELEM                   INITIAL_VALUE
            {
                SV_ADD_UINT8  (    SV_ELEM_TEST0,   std::numeric_limits<uint8_t> ::min ()     ),
                SV_ADD_UINT16 (    SV_ELEM_TEST5,   std::numeric_limits<uint16_t>::max ()     ),
                SV_ADD_UINT32 (    SV_ELEM_TEST7,    1                                        ),
                SV_ADD_UINT64 (    SV_ELEM_TEST9,   std::numeric_limits<uint64_t>::min ()     ),
                SV_ADD_INT8   (    SV_ELEM_TEST12,  std::numeric_limits<int8_t>  ::min ()     ),
                SV_ADD_INT8   (    SV_ELEM_TEST15,   1                                        ),
                SV_ADD_INT16  (    SV_ELEM_TEST18,  -1                                        ),
                SV_ADD_INT16  (    SV_ELEM_TEST21,  std::numeric_limits<int16_t> ::max ()     ),
                SV_ADD_INT32  (    SV_ELEM_TEST24,   0                                        ),
                SV_ADD_INT64  (    SV_ELEM_TEST27,  std::numeric_limits<int64_t> ::min ()     ),
                SV_ADD_INT64  (    SV_ELEM_TEST30,   1                                        ),
                SV_ADD_FLOAT  (    SV_ELEM_TEST33,   0                                        ),
                SV_ADD_FLOAT  (    SV_ELEM_TEST36,  std::numeric_limits<float>   ::max ()     ),
                SV_ADD_DOUBLE (    SV_ELEM_TEST39,   0                                        ),
                SV_ADD_DOUBLE (    SV_ELEM_TEST42,  std::numeric_limits<double>  ::max ()     ),
                SV_ADD_BOOL   (    SV_ELEM_TEST45,   true                                     ),
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST1,

            // Elements
            //      TYPE               ELEM                   INITIAL_VALUE
            {
                SV_ADD_UINT8  (    SV_ELEM_TEST1,    1                                        ),
                SV_ADD_UINT16 (    SV_ELEM_TEST4,    1                                        ),
                SV_ADD_UINT32 (    SV_ELEM_TEST8,   std::numeric_limits<uint32_t>::max ()     ),
                SV_ADD_UINT64 (    SV_ELEM_TEST10,   1                                        ),
                SV_ADD_INT8   (    SV_ELEM_TEST13,  -1                                        ),
                SV_ADD_INT8   (    SV_ELEM_TEST16,  std::numeric_limits<int8_t>  ::max ()     ),
                SV_ADD_INT16  (    SV_ELEM_TEST19,   0                                        ),
                SV_ADD_INT32  (    SV_ELEM_TEST22,  std::numeric_limits<int32_t> ::min ()     ),
                SV_ADD_INT32  (    SV_ELEM_TEST25,   1                                        ),
                SV_ADD_INT64  (    SV_ELEM_TEST28,  -1                                        ),
                SV_ADD_INT64  (    SV_ELEM_TEST31,  std::numeric_limits<int64_t> ::max ()     ),
                SV_ADD_FLOAT  (    SV_ELEM_TEST34,   37.81999                                 ),
                SV_ADD_FLOAT  (    SV_ELEM_TEST37,  std::numeric_limits<float>   ::infinity ()),
                SV_ADD_DOUBLE (    SV_ELEM_TEST40,   37.81999                                 ),
                SV_ADD_DOUBLE (    SV_ELEM_TEST43,  std::numeric_limits<double>  ::infinity ()),
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {SV_REG_TEST2,

            // Elements
            //      TYPE               ELEM                   INITIAL_VALUE
            {
                SV_ADD_UINT8  (    SV_ELEM_TEST2,   std::numeric_limits<uint8_t> ::max ()     ),
                SV_ADD_UINT16 (    SV_ELEM_TEST3,   std::numeric_limits<uint16_t>::min ()     ),
                SV_ADD_UINT32 (    SV_ELEM_TEST6,   std::numeric_limits<uint32_t>::min ()     ),
                SV_ADD_UINT64 (    SV_ELEM_TEST11,  std::numeric_limits<uint64_t>::max ()     ),
                SV_ADD_INT8   (    SV_ELEM_TEST14,   0                                        ),
                SV_ADD_INT16  (    SV_ELEM_TEST17,  std::numeric_limits<int16_t> ::min ()     ),
                SV_ADD_INT16  (    SV_ELEM_TEST20,   1                                        ),
                SV_ADD_INT32  (    SV_ELEM_TEST23,  -1                                        ),
                SV_ADD_INT32  (    SV_ELEM_TEST26,  std::numeric_limits<int32_t> ::max ()     ),
                SV_ADD_INT64  (    SV_ELEM_TEST29,   0                                        ),
                SV_ADD_FLOAT  (    SV_ELEM_TEST32,  std::numeric_limits<float>   ::min ()     ),
                SV_ADD_FLOAT  (    SV_ELEM_TEST35,  -37.81999                                 ),
                SV_ADD_DOUBLE (    SV_ELEM_TEST38,  std::numeric_limits<double>  ::min ()     ),
                SV_ADD_DOUBLE (    SV_ELEM_TEST41,  -37.81999                                 ),
                SV_ADD_BOOL   (    SV_ELEM_TEST44,   false                                    ),
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };

    std::vector<uint8_t> region0ExpectedBuffer = {
        0x00, 
        0xff, 0xff,
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80,
        0x01,
        0xff, 0xff,
        0xff, 0x7f,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0x7f, 0x7f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x7f,
        0x01,
    };

    std::vector<uint8_t> region1ExpectedBuffer = {
        0x01, 
        0x01, 0x00,
        0xff, 0xff, 0xff, 0xff,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff,
        0x7f,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x80,
        0x01, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
        0xab, 0x47, 0x17, 0x42,
        0x00, 0x00, 0x80, 0x7f,
        0x05, 0x86, 0xac, 0x6e, 0xf5, 0xe8, 0x42, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f,
    };

    std::vector<uint8_t> region2ExpectedBuffer = {
        0xff, 
        0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00,
        0x00, 0x80,
        0x01, 0x00,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x7f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x00,
        0xab, 0x47, 0x17, 0xc2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x05, 0x86, 0xac, 0x6e, 0xf5, 0xe8, 0x42, 0xc0,
        0x00,
    };

    // Create SV
    std::shared_ptr<StateVector> pSv; 
    CHECK_SUCCESS (StateVector::createNew (config, pSv));

    // Get State Vector info.
    StateVector::StateVectorInfo_t stateVectorInfo;
    pSv->getStateVectorInfo (stateVectorInfo);

    // Get Region0 info.
    StateVector::RegionInfo_t region0Info;
    pSv->getRegionInfo (SV_REG_TEST0, region0Info);

    // Get Region1 info.
    StateVector::RegionInfo_t region1Info;
    pSv->getRegionInfo (SV_REG_TEST1, region1Info);

    // Get Region2 info.
    StateVector::RegionInfo_t region2Info;
    pSv->getRegionInfo (SV_REG_TEST2, region2Info);

    // Verify State Vector and region sizes matches expected size.
    uint32_t region0ExpectedSizeBytes = region0ExpectedBuffer.size ();
    uint32_t region1ExpectedSizeBytes = region1ExpectedBuffer.size ();
    uint32_t region2ExpectedSizeBytes = region2ExpectedBuffer.size ();
    uint32_t stateVectorExpectedSizeBytes = region0ExpectedSizeBytes +
                                            region1ExpectedSizeBytes +
                                            region2ExpectedSizeBytes;
    CHECK (stateVectorInfo.sizeBytes == stateVectorExpectedSizeBytes);
    CHECK (region0Info.sizeBytes == region0ExpectedSizeBytes);
    CHECK (region1Info.sizeBytes == region1ExpectedSizeBytes);
    CHECK (region2Info.sizeBytes == region2ExpectedSizeBytes);

    // Verify each region's data matches expected.
    int cmpRet = std::memcmp (region0Info.pStart, 
                              region0ExpectedBuffer.data (),
                              region0ExpectedSizeBytes);
    CHECK (cmpRet == 0);
    cmpRet = std::memcmp (region1Info.pStart, 
                          region1ExpectedBuffer.data (),
                          region1ExpectedSizeBytes);
    CHECK (cmpRet == 0);
    cmpRet = std::memcmp (region2Info.pStart, 
                          region2ExpectedBuffer.data (),
                          region2ExpectedSizeBytes);
    CHECK (cmpRet == 0);

    // Verify State Vector's data matches expected.
    std::vector<uint8_t> stateVectorExpectedBuffer;
    stateVectorExpectedBuffer.reserve(stateVectorExpectedSizeBytes);
    stateVectorExpectedBuffer.insert (stateVectorExpectedBuffer.end (), 
                                      region0ExpectedBuffer.begin (),
                                      region0ExpectedBuffer.end ());
    stateVectorExpectedBuffer.insert (stateVectorExpectedBuffer.end (), 
                                      region1ExpectedBuffer.begin (),
                                      region1ExpectedBuffer.end ());
    stateVectorExpectedBuffer.insert (stateVectorExpectedBuffer.end (), 
                                      region2ExpectedBuffer.begin (),
                                      region2ExpectedBuffer.end ());
    cmpRet = std::memcmp (stateVectorInfo.pStart, 
                          stateVectorExpectedBuffer.data (),
                          stateVectorExpectedSizeBytes);
    CHECK (cmpRet == 0);
}

/* Group of tests to verify getSizeBytes. */
TEST_GROUP (StateVectorGetSizeFromBytes)
{

};

/* Verify that all types are supported by getSizeBytesFromType. This will fail
   if someone, for example, adds SV_T_ARRAY and forgets to add the case to the
   switch statement in getSizeBytesFromType. */
TEST (StateVectorGetSizeFromBytes, AllTypesInSwitch)
{
    uint8_t sizeBytes;
    for (uint8_t typeEnum; typeEnum < SV_T_LAST; typeEnum++)
    {
        CHECK_SUCCESS (StateVector::getSizeBytesFromType (
                    (StateVectorElementType_t) typeEnum, 
                    sizeBytes));
    }
}

/* Test getting size of invalid type. */
TEST (StateVectorGetSizeFromBytes, InvalidEnum)
{
    uint8_t sizeBytes;
    CHECK_ERROR (StateVector::getSizeBytesFromType (SV_T_LAST, sizeBytes), 
                 E_INVALID_ENUM);
}

/* Test getting size of all valid types. */
TEST (StateVectorGetSizeFromBytes, Success)
{
    std::vector<std::pair<StateVectorElementType_t, uint8_t>> testCases = 
    {
        {SV_T_UINT8,  1},
        {SV_T_UINT16, 2},
        {SV_T_UINT32, 4},
        {SV_T_UINT64, 8},
        {SV_T_INT8,   1},
        {SV_T_INT16,  2},
        {SV_T_INT32,  4},
        {SV_T_INT64,  8},
        {SV_T_FLOAT,  4},
        {SV_T_DOUBLE, 8},
        {SV_T_BOOL,   1}
    };

    uint8_t sizeBytes;
    for (uint8_t i = 0; i < testCases.size (); i++)
    {
        std::pair<StateVectorElementType_t, uint8_t> testCase = testCases[i];
        CHECK_SUCCESS (StateVector::getSizeBytesFromType (testCase.first, 
                                                          sizeBytes));
        CHECK (sizeBytes == testCase.second); 
    }
}

/**
 * Globals used in StateVectorGetRegionInfo tests.
 */
std::shared_ptr<StateVector> PGetRegionSv; 
StateVector::StateVectorConfig_t GetRegionConfig = {
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

/* Group of tests to verify getRegionInfo error returns. Successful returns will
   be verified in the State Vector constructor test group. */
TEST_GROUP (StateVectorGetRegionInfo)
{
    void setup () 
    {
        CHECK_SUCCESS (StateVector::createNew (GetRegionConfig, PGetRegionSv));
    }

    void teardown ()
    {
        PGetRegionSv.reset ();
    }

};

/* Test getting invalid region enum. */
TEST (StateVectorGetRegionInfo, getRegionInfo_InvalidEnum)
{
    StateVector::RegionInfo_t regionInfo;
    CHECK_ERROR (PGetRegionSv->getRegionInfo (SV_REG_LAST, regionInfo),
                 E_INVALID_REGION);
}

/* Test getting region not in State Vector. */
TEST (StateVectorGetRegionInfo, getRegionInfo_NotInSV)
{
    StateVector::RegionInfo_t regionInfo;
    CHECK_ERROR (PGetRegionSv->getRegionInfo (SV_REG_TEST2, regionInfo),
                 E_INVALID_REGION);
}

/* Test getting region not in State Vector. */
TEST (StateVectorGetRegionInfo, getRegionInfo_Success)
{
    StateVector::RegionInfo_t regionInfo;
    CHECK_SUCCESS (PGetRegionSv->getRegionInfo (SV_REG_TEST0, regionInfo));
    CHECK_SUCCESS (PGetRegionSv->getRegionInfo (SV_REG_TEST1, regionInfo));
}
