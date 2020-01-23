/* All #include statements should come before the CppUTest include */
#include <cstring>
#include <sstream>
#include <limits>
#include <tuple>

#include "Errors.h"
#include "DataVector.hpp"
#include "Log.hpp"

#include "TestHelpers.hpp"

/*************************** VERIFYCONFIG TESTS *******************************/

/* Group of tests verifying verifyConfig method. */
TEST_GROUP (DataVector_verifyConfig)
{

};

/* Test initializing with empty config. */
TEST (DataVector_verifyConfig, EmptyConfig)
{
    DataVector::Config_t config = {};
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_EMPTY_CONFIG);
}

/* Test initializing with element list empty. */
TEST (DataVector_verifyConfig, EmptyElementList)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_LAST, 

            // Elements
            {
            }},

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_EMPTY_ELEMS);
}

/* Test initializing with invalid region enum. */
TEST (DataVector_verifyConfig, InvalidRegionEnum)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_LAST,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            )
            }},

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_INVALID_ENUM);
}

/* Test initializing with invalid element enum. */
TEST (DataVector_verifyConfig, InvalidElemEnum)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_LAST,             0            )
            }},

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_INVALID_ENUM);
}

/* Test initializing with duplicate region name. */
TEST (DataVector_verifyConfig, DuplicateRegion)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            ),
                DV_ADD_BOOL   (           DV_ELEM_TEST1,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_FLOAT  (           DV_ELEM_TEST2,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_DUPLICATE_REGION);
}

/* Test initializing with duplicate element name in different region. */
TEST (DataVector_verifyConfig, DuplicateElementDiffRegion)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            ),
                DV_ADD_BOOL   (           DV_ELEM_TEST1,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST1,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_FLOAT  (           DV_ELEM_TEST0,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_DUPLICATE_ELEM);
}

/* Test initializing with duplicate element name in same region. */
TEST (DataVector_verifyConfig, DuplicateElementSameRegion)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            ),
                DV_ADD_BOOL   (           DV_ELEM_TEST0,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST1,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_FLOAT  (           DV_ELEM_TEST2,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_ERROR (DataVector::createNew (config, pDv), E_DUPLICATE_ELEM);
}

/* Test initializing with a valid config. */
TEST (DataVector_verifyConfig, Success)
{
    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            ),
                DV_ADD_BOOL   (           DV_ELEM_TEST1,            1            )
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST1,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_FLOAT  (           DV_ELEM_TEST2,            1.23         )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };
    std::shared_ptr<DataVector> pDv; 
    CHECK_SUCCESS (DataVector::createNew (config, pDv));
}

/**************************** CONSTRUCTOR TESTS *******************************/

/**
 * Comprehensive Data Vector config to test constructing, reading from, and 
 * writing to a Data Vector.
 */
DataVector::Config_t gMultiElemConfig = {
    // Regions
    {
        //////////////////////////////////////////////////////////////////////////////////

        // Region
        {DV_REG_TEST0,

        // Elements
        //      TYPE               ELEM                   INITIAL_VALUE
        {
            DV_ADD_UINT8  (    DV_ELEM_TEST0,   std::numeric_limits<uint8_t> ::min ()     ),
            DV_ADD_UINT16 (    DV_ELEM_TEST5,   std::numeric_limits<uint16_t>::max ()     ),
            DV_ADD_UINT32 (    DV_ELEM_TEST7,    1                                        ),
            DV_ADD_UINT64 (    DV_ELEM_TEST9,   std::numeric_limits<uint64_t>::min ()     ),
            DV_ADD_INT8   (    DV_ELEM_TEST12,  std::numeric_limits<int8_t>  ::min ()     ),
            DV_ADD_INT8   (    DV_ELEM_TEST15,   1                                        ),
            DV_ADD_INT16  (    DV_ELEM_TEST18,  -1                                        ),
            DV_ADD_INT16  (    DV_ELEM_TEST21,  std::numeric_limits<int16_t> ::max ()     ),
            DV_ADD_INT32  (    DV_ELEM_TEST24,   0                                        ),
            DV_ADD_INT64  (    DV_ELEM_TEST27,  std::numeric_limits<int64_t> ::min ()     ),
            DV_ADD_INT64  (    DV_ELEM_TEST30,   1                                        ),
            DV_ADD_FLOAT  (    DV_ELEM_TEST33,   0                                        ),
            DV_ADD_FLOAT  (    DV_ELEM_TEST36,  std::numeric_limits<float>   ::max ()     ),
            DV_ADD_DOUBLE (    DV_ELEM_TEST39,   0                                        ),
            DV_ADD_DOUBLE (    DV_ELEM_TEST42,  std::numeric_limits<double>  ::max ()     ),
            DV_ADD_BOOL   (    DV_ELEM_TEST45,   true                                     ),
        }},

        //////////////////////////////////////////////////////////////////////////////////

        // Region
        {DV_REG_TEST1,

        // Elements
        //      TYPE               ELEM                   INITIAL_VALUE
        {
            DV_ADD_UINT8  (    DV_ELEM_TEST1,    1                                        ),
            DV_ADD_UINT16 (    DV_ELEM_TEST4,    1                                        ),
            DV_ADD_UINT32 (    DV_ELEM_TEST8,   std::numeric_limits<uint32_t>::max ()     ),
            DV_ADD_UINT64 (    DV_ELEM_TEST10,   1                                        ),
            DV_ADD_INT8   (    DV_ELEM_TEST13,  -1                                        ),
            DV_ADD_INT8   (    DV_ELEM_TEST16,  std::numeric_limits<int8_t>  ::max ()     ),
            DV_ADD_INT16  (    DV_ELEM_TEST19,   0                                        ),
            DV_ADD_INT32  (    DV_ELEM_TEST22,  std::numeric_limits<int32_t> ::min ()     ),
            DV_ADD_INT32  (    DV_ELEM_TEST25,   1                                        ),
            DV_ADD_INT64  (    DV_ELEM_TEST28,  -1                                        ),
            DV_ADD_INT64  (    DV_ELEM_TEST31,  std::numeric_limits<int64_t> ::max ()     ),
            DV_ADD_FLOAT  (    DV_ELEM_TEST34,   37.81999                                 ),
            DV_ADD_FLOAT  (    DV_ELEM_TEST37,  std::numeric_limits<float>   ::infinity ()),
            DV_ADD_DOUBLE (    DV_ELEM_TEST40,   37.81999                                 ),
            DV_ADD_DOUBLE (    DV_ELEM_TEST43,  std::numeric_limits<double>  ::infinity ()),
        }},

        //////////////////////////////////////////////////////////////////////////////////

        // Region
        {DV_REG_TEST2,

        // Elements
        //      TYPE               ELEM                   INITIAL_VALUE
        {
            DV_ADD_UINT8  (    DV_ELEM_TEST2,   std::numeric_limits<uint8_t> ::max ()     ),
            DV_ADD_UINT16 (    DV_ELEM_TEST3,   std::numeric_limits<uint16_t>::min ()     ),
            DV_ADD_UINT32 (    DV_ELEM_TEST6,   std::numeric_limits<uint32_t>::min ()     ),
            DV_ADD_UINT64 (    DV_ELEM_TEST11,  std::numeric_limits<uint64_t>::max ()     ),
            DV_ADD_INT8   (    DV_ELEM_TEST14,   0                                        ),
            DV_ADD_INT16  (    DV_ELEM_TEST17,  std::numeric_limits<int16_t> ::min ()     ),
            DV_ADD_INT16  (    DV_ELEM_TEST20,   1                                        ),
            DV_ADD_INT32  (    DV_ELEM_TEST23,  -1                                        ),
            DV_ADD_INT32  (    DV_ELEM_TEST26,  std::numeric_limits<int32_t> ::max ()     ),
            DV_ADD_INT64  (    DV_ELEM_TEST29,   0                                        ),
            DV_ADD_FLOAT  (    DV_ELEM_TEST32,  std::numeric_limits<float>   ::min ()     ),
            DV_ADD_FLOAT  (    DV_ELEM_TEST35,  -37.81999                                 ),
            DV_ADD_DOUBLE (    DV_ELEM_TEST38,  std::numeric_limits<double>  ::min ()     ),
            DV_ADD_DOUBLE (    DV_ELEM_TEST41,  -37.81999                                 ),
            DV_ADD_BOOL   (    DV_ELEM_TEST44,   false                                    ),
        }}

        //////////////////////////////////////////////////////////////////////////////////
    }
};

/* Group of tests to verify Data Vector's underlying buffer. */
TEST_GROUP (DataVector_Construct)
{

};

/* Test constructing Data Vector with 1 element. */
TEST (DataVector_Construct, 1Elem_TypesAndBoundaryVals)
{
    typedef struct ConstructTestCase
    {
        DataVectorElementType_t type;
        uint64_t initialVal;
        std::vector<uint8_t> expectedBuf;
    } ConstructTestCase_t;

    DataVector::Config_t config = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE                      ELEM            INITIAL_VALUE
            {
                DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            )
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };

    std::vector<ConstructTestCase_t> testCases =
    {
    // SUB-TEST     TYPE                                                   INITIAL_VALUE                                        EXPECTED_BUFFER
       /* 0  */  {DV_T_UINT8,   DataVector::toUInt64<uint8_t>  ( std::numeric_limits<uint8_t> ::min ()      ),  {0x00}                                           },
       /* 1  */  {DV_T_UINT8,   DataVector::toUInt64<uint8_t>  (  1                                         ),  {0x01}                                           },
       /* 2  */  {DV_T_UINT8,   DataVector::toUInt64<uint8_t>  ( std::numeric_limits<uint8_t> ::max ()      ),  {0xff}                                           },
       /* 3  */  {DV_T_UINT16,  DataVector::toUInt64<uint16_t> ( std::numeric_limits<uint16_t>::min ()      ),  {0x00, 0x00}                                     },
       /* 4  */  {DV_T_UINT16,  DataVector::toUInt64<uint16_t> (  1                                         ),  {0x01, 0x00}                                     },
       /* 5  */  {DV_T_UINT16,  DataVector::toUInt64<uint16_t> ( std::numeric_limits<uint16_t>::max ()      ),  {0xff, 0xff}                                     },
       /* 6  */  {DV_T_UINT32,  DataVector::toUInt64<uint32_t> ( std::numeric_limits<uint32_t>::min ()      ),  {0x00, 0x00, 0x00, 0x00}                         },
       /* 7  */  {DV_T_UINT32,  DataVector::toUInt64<uint32_t> (  1                                         ),  {0x01, 0x00, 0x00, 0x00}                         },
       /* 8  */  {DV_T_UINT32,  DataVector::toUInt64<uint32_t> ( std::numeric_limits<uint32_t>::max ()      ),  {0xff, 0xff, 0xff, 0xff}                         },
       /* 9  */  {DV_T_UINT64,  DataVector::toUInt64<uint64_t> ( std::numeric_limits<uint64_t>::min ()      ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 10 */  {DV_T_UINT64,  DataVector::toUInt64<uint64_t> (  1                                         ),  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 11 */  {DV_T_UINT64,  DataVector::toUInt64<uint64_t> ( std::numeric_limits<uint64_t>::max ()      ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff} },
       /* 12 */  {DV_T_INT8,    DataVector::toUInt64<int8_t>   ( std::numeric_limits<int8_t>  ::min ()      ),  {0x80}                                           },
       /* 13 */  {DV_T_INT8,    DataVector::toUInt64<int8_t>   ( -1                                         ),  {0xff}                                           },
       /* 14 */  {DV_T_INT8,    DataVector::toUInt64<int8_t>   (  0                                         ),  {0x00}                                           },
       /* 15 */  {DV_T_INT8,    DataVector::toUInt64<int8_t>   (  1                                         ),  {0x01}                                           },
       /* 16 */  {DV_T_INT8,    DataVector::toUInt64<int8_t>   ( std::numeric_limits<int8_t>  ::max ()      ),  {0x7f}                                           },
       /* 17 */  {DV_T_INT16,   DataVector::toUInt64<int16_t>  ( std::numeric_limits<int16_t> ::min ()      ),  {0x00, 0x80}                                     },
       /* 18 */  {DV_T_INT16,   DataVector::toUInt64<int16_t>  ( -1                                         ),  {0xff, 0xff}                                     },
       /* 19 */  {DV_T_INT16,   DataVector::toUInt64<int16_t>  (  0                                         ),  {0x00, 0x00}                                     },
       /* 20 */  {DV_T_INT16,   DataVector::toUInt64<int16_t>  (  1                                         ),  {0x01, 0x00}                                     },
       /* 21 */  {DV_T_INT16,   DataVector::toUInt64<int16_t>  ( std::numeric_limits<int16_t> ::max ()      ),  {0xff, 0x7f}                                     },
       /* 22 */  {DV_T_INT32,   DataVector::toUInt64<int32_t>  ( std::numeric_limits<int32_t> ::min ()      ),  {0x00, 0x00, 0x00, 0x80}                         },
       /* 23 */  {DV_T_INT32,   DataVector::toUInt64<int32_t>  ( -1                                         ),  {0xff, 0xff, 0xff, 0xff}                         },
       /* 24 */  {DV_T_INT32,   DataVector::toUInt64<int32_t>  (  0                                         ),  {0x00, 0x00, 0x00, 0x00}                         },
       /* 25 */  {DV_T_INT32,   DataVector::toUInt64<int32_t>  (  1                                         ),  {0x01, 0x00, 0x00, 0x00}                         },
       /* 26 */  {DV_T_INT32,   DataVector::toUInt64<int32_t>  ( std::numeric_limits<int32_t> ::max ()      ),  {0xff, 0xff, 0xff, 0x7f}                         },
       /* 27 */  {DV_T_INT64,   DataVector::toUInt64<int64_t>  ( std::numeric_limits<int64_t> ::min ()      ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80} },
       /* 28 */  {DV_T_INT64,   DataVector::toUInt64<int64_t>  ( -1                                         ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff} },
       /* 29 */  {DV_T_INT64,   DataVector::toUInt64<int64_t>  (  0                                         ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 30 */  {DV_T_INT64,   DataVector::toUInt64<int64_t>  (  1                                         ),  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 31 */  {DV_T_INT64,   DataVector::toUInt64<int64_t>  ( std::numeric_limits<int64_t> ::max ()      ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f} },
       /* 32 */  {DV_T_FLOAT,   DataVector::toUInt64<float>    ( std::numeric_limits<float>   ::min ()      ),  {0x00, 0x00, 0x80, 0x00}                         },
       /* 33 */  {DV_T_FLOAT,   DataVector::toUInt64<float>    (  0                                         ),  {0x00, 0x00, 0x00, 0x00}                         },
       /* 34 */  {DV_T_FLOAT,   DataVector::toUInt64<float>    (  37.81999                                  ),  {0xab, 0x47, 0x17, 0x42}                         },
       /* 35 */  {DV_T_FLOAT,   DataVector::toUInt64<float>    ( -37.81999                                  ),  {0xab, 0x47, 0x17, 0xc2}                         },
       /* 36 */  {DV_T_FLOAT,   DataVector::toUInt64<float>    ( std::numeric_limits<float>   ::max ()      ),  {0xff, 0xff, 0x7f, 0x7f}                         },
       /* 37 */  {DV_T_FLOAT,   DataVector::toUInt64<float>    ( std::numeric_limits<float>   ::infinity () ),  {0x00, 0x00, 0x80, 0x7f}                         },
       /* 38 */  {DV_T_DOUBLE,  DataVector::toUInt64<double>   ( std::numeric_limits<double>  ::min ()      ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00} },
       /* 39 */  {DV_T_DOUBLE,  DataVector::toUInt64<double>   (  0                                         ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
       /* 40 */  {DV_T_DOUBLE,  DataVector::toUInt64<double>   (  37.81999                                  ),  {0x05, 0x86, 0xac, 0x6e, 0xf5, 0xe8, 0x42, 0x40} },
       /* 41 */  {DV_T_DOUBLE,  DataVector::toUInt64<double>   ( -37.81999                                  ),  {0x05, 0x86, 0xac, 0x6e, 0xf5, 0xe8, 0x42, 0xc0} },
       /* 42 */  {DV_T_DOUBLE,  DataVector::toUInt64<double>   ( std::numeric_limits<double>  ::max ()      ),  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x7f} },
       /* 43 */  {DV_T_DOUBLE,  DataVector::toUInt64<double>   ( std::numeric_limits<double>  ::infinity () ),  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f} },
       /* 44 */  {DV_T_BOOL,    DataVector::toUInt64<bool>     (  false                                     ),  {0x00}                                           },
       /* 45 */  {DV_T_BOOL,    DataVector::toUInt64<bool>     (  true                                      ),  {0x01}                                           },
    };

    // Loop through each test case, modify config, create Data Vector with 
    // single element, and verify.
    for (uint8_t i = 0; i < testCases.size (); i++)
    {
        ConstructTestCase_t testCase = testCases[i];
        uint32_t expectedSizeBytes = testCase.expectedBuf.size ();

        // Modify config for test case.
        config[0].elems[0].type       = testCase.type;
        config[0].elems[0].initialVal = testCase.initialVal;

        // Create DV
        INIT_DATA_VECTOR (config);

        // Get DV size and copy.
        uint32_t actualDataVectorSizeBytes = 0;
        CHECK_SUCCESS (pDv->getDataVectorSizeBytes (
                    actualDataVectorSizeBytes));
        std::vector<uint8_t> stateVectorBufCopy (actualDataVectorSizeBytes);
        CHECK_SUCCESS (pDv->readDataVector (stateVectorBufCopy));

        // Get region info.
        uint32_t actualRegionSizeBytes;
        CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST0, 
                                                actualRegionSizeBytes));
        std::vector<uint8_t> regionBufCopy (actualRegionSizeBytes);
        CHECK_SUCCESS (pDv-> readRegion (DV_REG_TEST0, regionBufCopy));

        // Verify sizes.
        if (actualDataVectorSizeBytes != expectedSizeBytes ||
            actualRegionSizeBytes != expectedSizeBytes)
        {
            std::stringstream stream;
            stream << "-- Sub-test " << static_cast<int> (i) << " failed --" << 
                std::endl;
            stream << "Expected Size Bytes:  " << expectedSizeBytes << 
                std::endl;
            stream << "Actual DV Size Bytes: " << actualDataVectorSizeBytes <<
                std::endl;
            stream << "Actual Region Size Bytes: " << actualRegionSizeBytes <<
                std::endl;

            FAIL (stream.str ().c_str ());
        }

        // Verify DV's underlying buffer matches expected data.
        int cmpDvRet = std::memcmp (stateVectorBufCopy.data (), 
                                    testCase.expectedBuf.data (),
                                    actualDataVectorSizeBytes);
        int cmpRegRet = std::memcmp (regionBufCopy.data (), 
                                     testCase.expectedBuf.data (),
                                     actualDataVectorSizeBytes);
        if (cmpDvRet != 0 || cmpRegRet != 0)
        {
            uint64_t expected = 0;
            uint64_t actualDv = 0;
            uint64_t actualReg = 0;
            std::memcpy (&expected, testCase.expectedBuf.data (), 
                         expectedSizeBytes);
            std::memcpy (&actualDv, stateVectorBufCopy.data (), expectedSizeBytes);
            std::memcpy (&actualReg, regionBufCopy.data (), expectedSizeBytes);
  
            std::stringstream stream;
            stream << "-- Sub-test " << static_cast<int> (i) << " failed --" << 
                std::endl;
            stream << "Expected Buffer: " << "0x" << std::hex << expected <<
                std::endl;
            stream << "Actual DV Buffer:   " << "0x" << std::hex << actualDv 
                << std::endl;
            stream << "Actual Reg Buffer:   " << "0x" << std::hex << actualReg
                << std::endl;

            FAIL (stream.str ().c_str ());
        }
    }
}

/* Test constructing Data Vector with multiple elements. */
TEST (DataVector_Construct, MultipleElem_TypesAndBoundaryVals) {
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

    // Create DV
    INIT_DATA_VECTOR (gMultiElemConfig);

    // Get DV size and copy.
    uint32_t actualDataVectorSizeBytes = 0;
    CHECK_SUCCESS (pDv->getDataVectorSizeBytes (
                   actualDataVectorSizeBytes));
    std::vector<uint8_t> stateVectorBufCopy (actualDataVectorSizeBytes);
    CHECK_SUCCESS (pDv->readDataVector (stateVectorBufCopy));

    // Get Region0 info.
    uint32_t region0ActualSizeBytes;
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST0, 
                                            region0ActualSizeBytes));
    std::vector<uint8_t> region0BufCopy (region0ActualSizeBytes);
    CHECK_SUCCESS (pDv-> readRegion (DV_REG_TEST0, region0BufCopy));

    // Get Region1 info.
    uint32_t region1ActualSizeBytes;
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST1, 
                                            region1ActualSizeBytes));
    std::vector<uint8_t> region1BufCopy (region1ActualSizeBytes);
    CHECK_SUCCESS (pDv-> readRegion (DV_REG_TEST1, region1BufCopy));

    // Get Region2 info.
    uint32_t region2ActualSizeBytes;
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST2, 
                                            region2ActualSizeBytes));
    std::vector<uint8_t> region2BufCopy (region2ActualSizeBytes);
    CHECK_SUCCESS (pDv->readRegion (DV_REG_TEST2, region2BufCopy));

    // Verify Data Vector and region sizes matches expected size.
    uint32_t region0ExpectedSizeBytes = region0ExpectedBuffer.size ();
    uint32_t region1ExpectedSizeBytes = region1ExpectedBuffer.size ();
    uint32_t region2ExpectedSizeBytes = region2ExpectedBuffer.size ();
    uint32_t stateVectorExpectedSizeBytes = region0ExpectedSizeBytes +
                                            region1ExpectedSizeBytes +
                                            region2ExpectedSizeBytes;
    CHECK (actualDataVectorSizeBytes == stateVectorExpectedSizeBytes);
    CHECK (region0ActualSizeBytes == region0ExpectedSizeBytes);
    CHECK (region1ActualSizeBytes == region1ExpectedSizeBytes);
    CHECK (region2ActualSizeBytes == region2ExpectedSizeBytes);

    // Verify each region's data matches expected.
    int cmpRet = std::memcmp (region0BufCopy.data (), 
                              region0ExpectedBuffer.data (),
                              region0ExpectedSizeBytes);
    CHECK (cmpRet == 0);
    cmpRet = std::memcmp (region1BufCopy.data (), 
                          region1ExpectedBuffer.data (),
                          region1ExpectedSizeBytes);
    CHECK (cmpRet == 0);
    cmpRet = std::memcmp (region2BufCopy.data (), 
                          region2ExpectedBuffer.data (),
                          region2ExpectedSizeBytes);
    CHECK (cmpRet == 0);

    // Verify Data Vector's data matches expected.
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
    cmpRet = std::memcmp (stateVectorBufCopy.data (), 
                          stateVectorExpectedBuffer.data (),
                          stateVectorExpectedSizeBytes);
    CHECK (cmpRet == 0);
}

/* Group of tests to verify getSizeBytes. */
TEST_GROUP (DataVector_getSizeFromBytes)
{

};

/* Verify that all types are supported by getSizeBytesFromType. This will fail
   if someone, for example, adds DV_T_ARRAY and forgets to add the case to the
   switch statement in getSizeBytesFromType. */
TEST (DataVector_getSizeFromBytes, AllTypesInSwitch)
{
    uint8_t sizeBytes;
    for (uint8_t typeEnum; typeEnum < DV_T_LAST; typeEnum++)
    {
        CHECK_SUCCESS (DataVector::getSizeBytesFromType (
                    (DataVectorElementType_t) typeEnum, 
                    sizeBytes));
    }
}

/* Test getting size of invalid type. */
TEST (DataVector_getSizeFromBytes, InvalidEnum)
{
    uint8_t sizeBytes;
    CHECK_ERROR (DataVector::getSizeBytesFromType (DV_T_LAST, sizeBytes), 
                 E_INVALID_ENUM);
}

/* Test getting size of all valid types. */
TEST (DataVector_getSizeFromBytes, Success)
{
    std::vector<std::pair<DataVectorElementType_t, uint8_t>> testCases = 
    {
        {DV_T_UINT8,  1},
        {DV_T_UINT16, 2},
        {DV_T_UINT32, 4},
        {DV_T_UINT64, 8},
        {DV_T_INT8,   1},
        {DV_T_INT16,  2},
        {DV_T_INT32,  4},
        {DV_T_INT64,  8},
        {DV_T_FLOAT,  4},
        {DV_T_DOUBLE, 8},
        {DV_T_BOOL,   1}
    };

    uint8_t sizeBytes;
    for (uint8_t i = 0; i < testCases.size (); i++)
    {
        std::pair<DataVectorElementType_t, uint8_t> testCase = testCases[i];
        CHECK_SUCCESS (DataVector::getSizeBytesFromType (testCase.first, 
                                                          sizeBytes));
        CHECK (sizeBytes == testCase.second); 
    }
}

DataVector::Config_t gSimpleConfig = 
{
    {DV_REG_TEST0,
    {
        DV_ADD_BOOL (DV_ELEM_TEST0, true),
    }},
}; 

/* Group of tests to verify elementExists. */
TEST_GROUP (DataVector_elementExists)
{

};

/* Test nonexistent elem. */
TEST (DataVector_elementExists, DNE)
{
    INIT_DATA_VECTOR (gSimpleConfig);
    CHECK_ERROR (pDv->elementExists (DV_ELEM_TEST1), E_INVALID_ELEM);
}

/* Test existent elem. */
TEST (DataVector_elementExists, Exists)
{
    INIT_DATA_VECTOR (gSimpleConfig);
    CHECK_SUCCESS (pDv->elementExists (DV_ELEM_TEST0));
}

/***************************** READ/WRITE TESTS *******************************/

/**
 * Check if read is successful and verify value read. 
 */
#define CHECK_READ_SUCCESS(elem, actualVal, expectedVal)                       \
{                                                                              \
    CHECK_SUCCESS (pDv->read (elem, actualVal));                               \
    CHECK_EQUAL (actualVal, expectedVal);                                      \
}

/**
 * Check if write is successful and verify by reading the value. 
 */
#define CHECK_WRITE_SUCCESS(elem, readVar, writeVar)                           \
{                                                                              \
    CHECK_SUCCESS (pDv->write (elem, writeVar));                               \
    CHECK_READ_SUCCESS (elem, readVar, writeVar);                              \
}

/**
 * Helper function to test read method on each element in a Data Vector 
 * initialized with gMultiElemConfig,
 */
static void checkMultiElemReadSuccess ()
{
    // Create DV
    INIT_DATA_VECTOR (gMultiElemConfig);

    uint8_t  valU8     = 0;
    uint16_t valU16    = 0;
    uint32_t valU32    = 0;
    uint64_t valU64    = 0;
    int8_t   val8      = 0;
    int16_t  val16     = 0;
    int32_t  val32     = 0;
    int64_t  val64     = 0;
    float    valFloat  = 0;
    double   valDouble = 0;
    bool     valBool   = 0;

    CHECK_READ_SUCCESS (DV_ELEM_TEST0,  valU8,     std::numeric_limits<uint8_t> ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST1,  valU8,      1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST2,  valU8,     std::numeric_limits<uint8_t> ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST3,  valU16,    std::numeric_limits<uint16_t>::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST4,  valU16,     1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST5,  valU16,    std::numeric_limits<uint16_t>::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST6,  valU32,    std::numeric_limits<uint32_t>::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST7,  valU32,     1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST8,  valU32,    std::numeric_limits<uint32_t>::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST9,  valU64,    std::numeric_limits<uint64_t>::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST10, valU64,     1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST11, valU64,    std::numeric_limits<uint64_t>::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST12, val8,      std::numeric_limits<int8_t>  ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST13, val8,      -1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST14, val8,       0                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST15, val8,       1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST16, val8,      std::numeric_limits<int8_t>  ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST17, val16,     std::numeric_limits<int16_t> ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST18, val16,     -1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST19, val16,      0                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST20, val16,      1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST21, val16,     std::numeric_limits<int16_t> ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST22, val32,     std::numeric_limits<int32_t> ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST23, val32,     -1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST24, val32,      0                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST25, val32,      1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST26, val32,     std::numeric_limits<int32_t> ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST27, val64,     std::numeric_limits<int64_t> ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST28, val64,     -1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST29, val64,      0                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST30, val64,      1                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST31, val64,     std::numeric_limits<int64_t> ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST32, valFloat,  std::numeric_limits<float>   ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST33, valFloat,   0                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST34, valFloat,  (float)  37.81999                         );
    CHECK_READ_SUCCESS (DV_ELEM_TEST35, valFloat,  (float) -37.81999                         );
    CHECK_READ_SUCCESS (DV_ELEM_TEST36, valFloat,  std::numeric_limits<float>   ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST37, valFloat,  std::numeric_limits<float>   ::infinity ());
    CHECK_READ_SUCCESS (DV_ELEM_TEST38, valDouble, std::numeric_limits<double>  ::min ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST39, valDouble,  0                                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST40, valDouble, (double)  37.81999                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST41, valDouble, (double) -37.81999                        );
    CHECK_READ_SUCCESS (DV_ELEM_TEST42, valDouble, std::numeric_limits<double>  ::max ()     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST43, valDouble, std::numeric_limits<double>  ::infinity ());
    CHECK_READ_SUCCESS (DV_ELEM_TEST44, valBool,   false                                     );
    CHECK_READ_SUCCESS (DV_ELEM_TEST45, valBool,   true                                      );
}

/**
 * Helper function to test writing to a Data Vector with all elements
 * initialized to 0.
 */
static void checkMultiElemWriteSuccess ()
{
    DataVector::Config_t multiElemConfigEmpty = {
        // Regions
        {
            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST0,

            // Elements
            //      TYPE               ELEM      INITIAL_VALUE
            {
                DV_ADD_UINT8  (    DV_ELEM_TEST0,      0    ),
                DV_ADD_UINT16 (    DV_ELEM_TEST5,      0    ),
                DV_ADD_UINT32 (    DV_ELEM_TEST7,      0    ),
                DV_ADD_UINT64 (    DV_ELEM_TEST9,      0    ),
                DV_ADD_INT8   (    DV_ELEM_TEST12,     0    ),
                DV_ADD_INT8   (    DV_ELEM_TEST15,     0    ),
                DV_ADD_INT16  (    DV_ELEM_TEST18,     0    ),
                DV_ADD_INT16  (    DV_ELEM_TEST21,     0    ),
                DV_ADD_INT32  (    DV_ELEM_TEST24,     0    ),
                DV_ADD_INT64  (    DV_ELEM_TEST27,     0    ),
                DV_ADD_INT64  (    DV_ELEM_TEST30,     0    ),
                DV_ADD_FLOAT  (    DV_ELEM_TEST33,     0    ),
                DV_ADD_FLOAT  (    DV_ELEM_TEST36,     0    ),
                DV_ADD_DOUBLE (    DV_ELEM_TEST39,     0    ),
                DV_ADD_DOUBLE (    DV_ELEM_TEST42,     0    ),
                DV_ADD_BOOL   (    DV_ELEM_TEST45,     0    ),
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST1,

            // Elements
            //      TYPE               ELEM      INITIAL_VALUE
            {
                DV_ADD_UINT8  (    DV_ELEM_TEST1,      0    ),
                DV_ADD_UINT16 (    DV_ELEM_TEST4,      0    ),
                DV_ADD_UINT32 (    DV_ELEM_TEST8,      0    ),
                DV_ADD_UINT64 (    DV_ELEM_TEST10,     0    ),
                DV_ADD_INT8   (    DV_ELEM_TEST13,     0    ),
                DV_ADD_INT8   (    DV_ELEM_TEST16,     0    ),
                DV_ADD_INT16  (    DV_ELEM_TEST19,     0    ),
                DV_ADD_INT32  (    DV_ELEM_TEST22,     0    ),
                DV_ADD_INT32  (    DV_ELEM_TEST25,     0    ),
                DV_ADD_INT64  (    DV_ELEM_TEST28,     0    ),
                DV_ADD_INT64  (    DV_ELEM_TEST31,     0    ),
                DV_ADD_FLOAT  (    DV_ELEM_TEST34,     0    ),
                DV_ADD_FLOAT  (    DV_ELEM_TEST37,     0    ),
                DV_ADD_DOUBLE (    DV_ELEM_TEST40,     0    ),
                DV_ADD_DOUBLE (    DV_ELEM_TEST43,     0    ),
            }},

            //////////////////////////////////////////////////////////////////////////////////

            // Region
            {DV_REG_TEST2,

            // Elements
            //      TYPE               ELEM      INITIAL_VALUE
            {
                DV_ADD_UINT8  (    DV_ELEM_TEST2,      0    ),
                DV_ADD_UINT16 (    DV_ELEM_TEST3,      0    ),
                DV_ADD_UINT32 (    DV_ELEM_TEST6,      0    ),
                DV_ADD_UINT64 (    DV_ELEM_TEST11,     0    ),
                DV_ADD_INT8   (    DV_ELEM_TEST14,     0    ),
                DV_ADD_INT16  (    DV_ELEM_TEST17,     0    ),
                DV_ADD_INT16  (    DV_ELEM_TEST20,     0    ),
                DV_ADD_INT32  (    DV_ELEM_TEST23,     0    ),
                DV_ADD_INT32  (    DV_ELEM_TEST26,     0    ),
                DV_ADD_INT64  (    DV_ELEM_TEST29,     0    ),
                DV_ADD_FLOAT  (    DV_ELEM_TEST32,     0    ),
                DV_ADD_FLOAT  (    DV_ELEM_TEST35,     0    ),
                DV_ADD_DOUBLE (    DV_ELEM_TEST38,     0    ),
                DV_ADD_DOUBLE (    DV_ELEM_TEST41,     0    ),
                DV_ADD_BOOL   (    DV_ELEM_TEST44,     0    ),
            }}

            //////////////////////////////////////////////////////////////////////////////////
        }
    };

    // Create DV
    INIT_DATA_VECTOR (multiElemConfigEmpty);

    uint8_t  readVarU8     = 0;
    uint16_t readVarU16    = 0;
    uint32_t readVarU32    = 0;
    uint64_t readVarU64    = 0;
    int8_t   readVar8      = 0;
    int16_t  readVar16     = 0;
    int32_t  readVar32     = 0;
    int64_t  readVar64     = 0;
    float    readVarFloat  = 0;
    double   readVarDouble = 0;
    bool     readVarBool   = 0;

    CHECK_WRITE_SUCCESS (DV_ELEM_TEST0,  readVarU8,     (uint8_t)  std::numeric_limits<uint8_t> ::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST1,  readVarU8,     (uint8_t)   1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST2,  readVarU8,     (uint8_t)  std::numeric_limits<uint8_t> ::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST3,  readVarU16,    (uint16_t) std::numeric_limits<uint16_t>::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST4,  readVarU16,    (uint16_t)  1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST5,  readVarU16,    (uint16_t) std::numeric_limits<uint16_t>::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST6,  readVarU32,    (uint32_t) std::numeric_limits<uint32_t>::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST7,  readVarU32,    (uint32_t)  1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST8,  readVarU32,    (uint32_t) std::numeric_limits<uint32_t>::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST9,  readVarU64,    (uint64_t) std::numeric_limits<uint64_t>::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST10, readVarU64,    (uint64_t)  1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST11, readVarU64,    (uint64_t) std::numeric_limits<uint64_t>::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST12, readVar8,      (int8_t)   std::numeric_limits<int8_t>  ::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST13, readVar8,      (int8_t)   -1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST14, readVar8,      (int8_t)    0                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST15, readVar8,      (int8_t)    1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST16, readVar8,      (int8_t)   std::numeric_limits<int8_t>  ::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST17, readVar16,     (int16_t)  std::numeric_limits<int16_t> ::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST18, readVar16,     (int16_t)  -1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST19, readVar16,     (int16_t)   0                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST20, readVar16,     (int16_t)   1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST21, readVar16,     (int16_t)  std::numeric_limits<int16_t> ::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST22, readVar32,     (int32_t)  std::numeric_limits<int32_t> ::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST23, readVar32,     (int32_t)  -1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST24, readVar32,     (int32_t)   0                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST25, readVar32,     (int32_t)   1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST26, readVar32,     (int32_t)  std::numeric_limits<int32_t> ::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST27, readVar64,     (int64_t)  std::numeric_limits<int64_t> ::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST28, readVar64,     (int64_t)  -1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST29, readVar64,     (int64_t)   0                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST30, readVar64,     (int64_t)   1                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST31, readVar64,     (int64_t)  std::numeric_limits<int64_t> ::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST32, readVarFloat,  (float)    std::numeric_limits<float>   ::min ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST33, readVarFloat,  (float)     0                                       );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST34, readVarFloat,  (float)     37.81999                                );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST35, readVarFloat,  (float)    -37.81999                                );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST36, readVarFloat,  (float)    std::numeric_limits<float>   ::max ()    );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST37, readVarFloat,  (float)    std::numeric_limits<float>   ::infinity ());
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST38, readVarDouble, (double)   std::numeric_limits<double>  ::min ()     );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST39, readVarDouble, (double)    0                                        );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST40, readVarDouble, (double)    37.81999                                 );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST41, readVarDouble, (double)   -37.81999                                 );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST42, readVarDouble, (double)   std::numeric_limits<double>  ::max ()     );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST43, readVarDouble, (double)   std::numeric_limits<double>  ::infinity ());
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST44, readVarBool,   (bool)     false                                     );
    CHECK_WRITE_SUCCESS (DV_ELEM_TEST45, readVarBool,   (bool)     true                                      );
}

/* Test Data Vector read and write methods. */
TEST_GROUP (DataVector_readWrite)
{

};

/* Test reading invalid elem. */
TEST (DataVector_readWrite, InvalidReadElem)
{
    // Create DV
    INIT_DATA_VECTOR (gMultiElemConfig);

    bool value = false;
    CHECK_ERROR (pDv->read (DV_ELEM_TEST46, value), E_INVALID_ELEM);
}

/* Test reading elem with incorrect type. */
TEST (DataVector_readWrite, InvalidReadType)
{
    // Create DV
    INIT_DATA_VECTOR (gMultiElemConfig);

    bool value = false;
    CHECK_ERROR (pDv->read (DV_ELEM_TEST0, value), E_INCORRECT_TYPE);
}

/* Test writing invalid elem. */
TEST (DataVector_readWrite, InvalidWriteElem)
{
    // Create DV
    INIT_DATA_VECTOR (gMultiElemConfig);

    bool value = false;
    CHECK_ERROR (pDv->write (DV_ELEM_TEST46, value), E_INVALID_ELEM);
}

/* Test writing elem with incorrect type. */
TEST (DataVector_readWrite, InvalidWriteType)
{
    // Create DV
    INIT_DATA_VECTOR (gMultiElemConfig);

    bool value = false;
    CHECK_ERROR (pDv->write (DV_ELEM_TEST0, value), E_INCORRECT_TYPE);
}

/* Test reading each element after constructing. */
TEST (DataVector_readWrite, SuccessfulRead)
{
    // Check reads without lock.
    checkMultiElemReadSuccess ();
}

/* Test writing each element after constructing with all elems set to 0. */
TEST (DataVector_readWrite, SuccessfulWrite)
{
    // Check writes without lock.
    checkMultiElemWriteSuccess ();
}

/*********************** READREGION/WRITEREGION TESTS *************************/
DataVector::Config_t gReadRegionWriteRegionConfig = {
    // Regions
    {
        //////////////////////////////////////////////////////////////////////////////////

        // Region
        {DV_REG_TEST0,

        // Elements
        //      TYPE                      ELEM            INITIAL_VALUE
        {
            DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            ),
            DV_ADD_BOOL   (           DV_ELEM_TEST1,            1            )
        }},

        //////////////////////////////////////////////////////////////////////////////////

        // Region
        {DV_REG_TEST1,

        // Elements
        //      TYPE                      ELEM            INITIAL_VALUE
        {
            DV_ADD_FLOAT  (           DV_ELEM_TEST2,            1.23         )
        }}

        //////////////////////////////////////////////////////////////////////////////////
    }
};

/* Test Data Vector readRegion and writeRegion methods. */
TEST_GROUP (DataVector_readRegionWriteRegion)
{

};

/* Test reading region not in DV. */
TEST (DataVector_readRegionWriteRegion, ReadRegionNotInDV)
{
    // Create DV
    INIT_DATA_VECTOR (gReadRegionWriteRegionConfig);

    std::vector<uint8_t> regionBufCopy;
    CHECK_ERROR (pDv->readRegion (DV_REG_TEST2, regionBufCopy), 
                 E_INVALID_REGION);
}

/* Test reading region with incorrect vector size. */
TEST (DataVector_readRegionWriteRegion, ReadIncorrectRegionSize)
{
    // Create DV
    INIT_DATA_VECTOR (gReadRegionWriteRegionConfig);

    uint32_t regionSizeBytes;
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST0, regionSizeBytes));

    std::vector<uint8_t> regionBufCopy (regionSizeBytes + 1);
    CHECK_ERROR (pDv->readRegion (DV_REG_TEST0, regionBufCopy), 
                 E_INCORRECT_SIZE);
}

/* Test writing region not in DV. */
TEST (DataVector_readRegionWriteRegion, WriteRegionNotInDV)
{
    // Create DV
    INIT_DATA_VECTOR (gReadRegionWriteRegionConfig);

    std::vector<uint8_t> regionBuf;
    CHECK_ERROR (pDv->writeRegion (DV_REG_TEST2, regionBuf), 
                 E_INVALID_REGION);
}

/* Test writing region with incorrect vector size. */
TEST (DataVector_readRegionWriteRegion, WriteIncorrectRegionSize)
{
    // Create DV
    INIT_DATA_VECTOR (gReadRegionWriteRegionConfig);

    uint32_t regionSizeBytes;
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST0, regionSizeBytes));

    std::vector<uint8_t> regionBuf (regionSizeBytes + 1);
    CHECK_ERROR (pDv->writeRegion (DV_REG_TEST0, regionBuf), 
                 E_INCORRECT_SIZE);
}

/* Test writing region with correct vector size. */
TEST (DataVector_readRegionWriteRegion, Success)
{
    // Create DV
    INIT_DATA_VECTOR (gReadRegionWriteRegionConfig);

    uint32_t region0SizeBytes;
    uint32_t region1SizeBytes;
    uint32_t sVSizeBytes;
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST0, region0SizeBytes));
    CHECK_SUCCESS (pDv->getRegionSizeBytes (DV_REG_TEST1, region1SizeBytes));
    CHECK_SUCCESS (pDv->getDataVectorSizeBytes (sVSizeBytes));

    // Verify sizes match expected.
    CHECK_EQUAL (region0SizeBytes, 2);
    CHECK_EQUAL (region1SizeBytes, 4);
    CHECK_EQUAL (sVSizeBytes, 6);

    // Get copy of region and dv buffers.
    std::vector<uint8_t> reg0Buf (region0SizeBytes);
    std::vector<uint8_t> reg1Buf (region1SizeBytes);
    std::vector<uint8_t> sVBuf (sVSizeBytes);
    CHECK_SUCCESS (pDv->readRegion (DV_REG_TEST0, reg0Buf));
    CHECK_SUCCESS (pDv->readRegion (DV_REG_TEST1, reg1Buf));
    CHECK_SUCCESS (pDv->readDataVector (sVBuf));

    // Verify buffers match expected.
    std::vector<uint8_t> reg0ExpBuf = {0x0, 0x1};
    std::vector<uint8_t> reg1ExpBuf = {0xa4, 0x70, 0x9d, 0x3f};
    std::vector<uint8_t> sVExpBuf = {0x0, 0x1, 0xa4, 0x70, 0x9d, 0x3f};
    CHECK (reg0Buf == reg0ExpBuf);
    CHECK (reg1Buf == reg1ExpBuf);
    CHECK (sVBuf   == sVExpBuf);
    
    // Write region 0 and verify Data Vector updated.
    std::vector<uint8_t> reg0WriteBuf = {0xff, 0x0};
    CHECK_SUCCESS (pDv->writeRegion (DV_REG_TEST0, reg0WriteBuf));
    CHECK_SUCCESS (pDv->readRegion (DV_REG_TEST0, reg0Buf));
    CHECK (reg0Buf == reg0WriteBuf);
    
    // Write region 1 and verify Data Vector updated.
    std::vector<uint8_t> reg1WriteBuf = {0x00, 0xff, 0x00, 0xff};
    CHECK_SUCCESS (pDv->writeRegion (DV_REG_TEST1, reg1WriteBuf));
    CHECK_SUCCESS (pDv->readRegion (DV_REG_TEST1, reg1Buf));
    CHECK (reg1Buf == reg1WriteBuf);
    
    // Verify entire DV matches expected.
    std::vector<uint8_t> sVExpBufAfterWrites = 
        {0xff, 0x0, 0x0, 0xff, 0x0, 0xff};
    CHECK_SUCCESS (pDv->readDataVector (sVBuf));
    CHECK (sVBuf == sVExpBufAfterWrites);
}

/**************************** SYNCHRONIZATION TESTS ***************************/

/**
 * Params to pass log, Data Vector, and thread ID to thread functions.
 */
struct ThreadFuncArgs 
{
    Log*                         testLog;
    std::shared_ptr<DataVector> stateVector;
    uint8_t                      threadId;
};

/**
 * Lock used to synchronize between threads so that certain test conditions can 
 * be achieved.
 */
pthread_mutex_t gTestLock;

/**
 * Config for synchronization tests.
 */
DataVector::Config_t gSynchronizationConfig = {
    // Regions
    {
        //////////////////////////////////////////////////////////////////////////////////

        // Region
        {DV_REG_TEST0,

        // Elements
        //      TYPE                      ELEM            INITIAL_VALUE
        {
            DV_ADD_UINT8  (           DV_ELEM_TEST0,            0            ),
        }},

        //////////////////////////////////////////////////////////////////////////////////
    }
};

/**
 * Thread that:
 *   1) Acquires the Data Vector lock
 *   2) Logs its thread ID to the test log
 *   3) Releases the Data Vector lock
 */
static void* threadFuncLockAndLog (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs     = (struct ThreadFuncArgs *) rawArgs;
    Log* log                         = pArgs->testLog;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;
    uint8_t threadId                 = pArgs->threadId;

    ret = pDv->acquireLock ();
    if (ret == E_SUCCESS)
    {
        log->logEvent (Log::LogEvent_t::ACQUIRED_LOCK, threadId);
        ret = pDv->releaseLock ();
    }

    return (void *) ret;
}

/**
 * Thread that: 
 *   1) Acquires the Data Vector lock
 *   2) Logs its thread ID to the test log
 *   3) Acquires the test lock
 *   4) Releases the Data Vector lock
 *   5) Releases the test lock
 *   6) Logs to the test log
 */
static void* threadFuncLockAndLogThenBlock (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs     = (struct ThreadFuncArgs *) rawArgs;
    Log* log                         = pArgs->testLog;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;
    uint8_t threadId                 = pArgs->threadId;

    ret = pDv->acquireLock ();
    if (ret == E_SUCCESS)
    {
        log->logEvent (Log::LogEvent_t::ACQUIRED_LOCK, threadId);

        // Wait on test lock before releasing DV lock.
        pthread_mutex_lock (&gTestLock);
        ret = pDv->releaseLock ();
        pthread_mutex_unlock (&gTestLock);

        // Log again to indicate thread has reached this point.
        log->logEvent (Log::LogEvent_t::RELEASED_LOCK, threadId);
    }

    return (void *) ret;
}

/**
 * Thread that calls read on DV_ELEM_TEST0 and logs the result to the test log.
 */
static void* threadFuncRead (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs     = (struct ThreadFuncArgs *) rawArgs;
    Log* log                         = pArgs->testLog;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;

    // Read first element in DV.
    uint8_t value = 0;
    ret = pDv->read (DV_ELEM_TEST0, value);

    // Log value read from DV.
    log->logEvent (Log::LogEvent_t::READ_VALUE, value);

    return (void *) ret;
}

/**
 * Thread that calls write to update DV_ELEM_TEST0.
 */
static void* threadFuncWrite (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs = (struct ThreadFuncArgs *) rawArgs;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;

    // Write first element in DV.
    uint8_t value = 2;
    ret = pDv->write (DV_ELEM_TEST0, value);

    return (void *) ret;
}

/**
 * Thread that calls readRegion on DV_REG_TEST0 and logs the result to the test 
 * log.
 */
static void* threadFuncReadRegion (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs     = (struct ThreadFuncArgs *) rawArgs;
    Log* log                         = pArgs->testLog;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;

    // Read first region in DV.
    uint32_t regionSizeBytes;
    pDv->getRegionSizeBytes (DV_REG_TEST0, regionSizeBytes);
    std::vector<uint8_t> regBufCopy (regionSizeBytes);
    ret = pDv->readRegion (DV_REG_TEST0, regBufCopy);

    // There's only 1, 1-byte element in region0. Log value.
    log->logEvent (Log::LogEvent_t::READ_VALUE, regBufCopy[0]);

    return (void *) ret;
}

/**
 * Thread that calls writeRegion to update DV_REG_TEST0.
 */
static void* threadFuncWriteRegion (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs = (struct ThreadFuncArgs *) rawArgs;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;

    // Write first region in DV.
    uint32_t regionSizeBytes;
    pDv->getRegionSizeBytes (DV_REG_TEST0, regionSizeBytes);
    std::vector<uint8_t> regBufWrite = {0x2};
    ret = pDv->writeRegion (DV_REG_TEST0, regBufWrite);

    return (void *) ret;
}

/**
 * Thread that calls readDataVector logs the result to the test log.
 */
static void* threadFuncReadDataVector (void *rawArgs)
{
    Error_t ret = E_SUCCESS;

    // Parse args.
    struct ThreadFuncArgs* pArgs     = (struct ThreadFuncArgs *) rawArgs;
    Log* log                         = pArgs->testLog;
    std::shared_ptr<DataVector> pDv = pArgs->stateVector;

    // Read DV.
    uint32_t sVSizeBytes;
    pDv->getDataVectorSizeBytes (sVSizeBytes);
    std::vector<uint8_t> sVBufCopy (sVSizeBytes);
    ret = pDv->readDataVector (sVBufCopy);

    // There's only 1, 1-byte element in DV. Log value.
    log->logEvent (Log::LogEvent_t::READ_VALUE, sVBufCopy[0]);

    return (void *) ret;
}

/**
 * Helper function to test the Data Vector's lock acquire semantics.
 *
 * @param  t1Pri     Priority of first thread.
 * @param  t2Pri     Priority of second thread.
 * @param  t3Pri     Priority of third thread.
 * @param  expected  Vector of tuples representing expected log state at
 *                   the end of the function.
 */
static void testLockAcquireSemantics (
            ThreadManager::Priority_t t1Pri, 
            ThreadManager::Priority_t t2Pri, 
            ThreadManager::Priority_t t3Pri, 
            std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> &expected)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize threads.
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    struct ThreadFuncArgs argsThread2 = {&testLog, pDv, 2}; 
    struct ThreadFuncArgs argsThread3 = {&testLog, pDv, 3}; 

    ThreadManager::ThreadFunc_t *pThreadFuncLockAndLog = 
        (ThreadManager::ThreadFunc_t *) &threadFuncLockAndLog;

    // Acquire lock so that all other threads initially block on acquire.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Create each thread and sleep so that they run until they are blocked
    // by the acquireLock call. Sleeping between creations ensures the blocking
    // occurs in the following order:
    //
    // 1) low pri thread
    // 2) high pri thread
    // 3) mid pri thread
    //
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncLockAndLog,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t2, pThreadFuncLockAndLog,
                                    &argsThread2, sizeof (argsThread2),
                                    t2Pri,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t3, pThreadFuncLockAndLog,
                                    &argsThread3, sizeof (argsThread3),
                                    t3Pri,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Release lock.
    CHECK_SUCCESS (pDv->releaseLock ());

    // Sleep so that the three threads run to completion.
    TestHelpers::sleepMs(100);
   
    // Wait for threads.
    Error_t threadReturn;
    ret = pThreadManager->waitForThread (t1, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pThreadManager->waitForThread (t2, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pThreadManager->waitForThread (t3, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Build expected log.
    for (uint32_t i = 0; i < expected.size (); i++)
    {
        expectedLog.logEvent (std::get<0> (expected[i]), 
                              std::get<1> (expected[i]));
    }
    
    // Verify actual == expected.
    VERIFY_LOGS;
}

/**
 * Helper function to test the Data Vector's lock release semantics.
 *
 * @param  t1Pri     Priority of first thread.
 * @param  t2Pri     Priority of second thread.
 * @param  expected  Vector of tuples representing expected log state at
 *                   the end of the function.
 */
static void testLockReleaseSemantics (
            ThreadManager::Priority_t t1Pri, 
            ThreadManager::Priority_t t2Pri, 
            std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> &expected)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize 2 threads.
    pthread_t t1;
    pthread_t t2;

    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    struct ThreadFuncArgs argsThread2 = {&testLog, pDv, 2}; 

    ThreadManager::ThreadFunc_t *pThreadFuncLockAndLogThenBlock = 
        (ThreadManager::ThreadFunc_t *) &threadFuncLockAndLogThenBlock;

    ThreadManager::ThreadFunc_t *pThreadFuncLockAndLog = 
        (ThreadManager::ThreadFunc_t *) &threadFuncLockAndLog;

    // Initialize test lock used to synchronize between cpputest thread and 
    // t1.
    pthread_mutex_init (&gTestLock, NULL);

    // Acquire test lock so that t1 blocks before releasing the Data Vector 
    // lock.
    pthread_mutex_lock (&gTestLock);

    // Create t1 and sleep so that the thread acquires the DV lock, logs, and 
    // then blocks on the test lock (which is currently held by the cpputest 
    // thread).
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncLockAndLogThenBlock,
                                    &argsThread1, sizeof (argsThread1),
                                    t1Pri, ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Create t2 and sleep so that the thread blocks on attempting to acquire 
    // the DV lock.
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t2, pThreadFuncLockAndLog,
                                    &argsThread2, sizeof (argsThread2),
                                    t2Pri, ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Release test lock and sleep. This will unblock t1 and then t2 once t1
    // releases the DV lock.
    pthread_mutex_unlock (&gTestLock);
    TestHelpers::sleepMs (100);
   
    // Wait for threads.
    Error_t threadReturn;
    pThreadManager->waitForThread (t1, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    ret = pThreadManager->waitForThread (t2, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);

    // Build expected log.
    for (uint32_t i = 0; i < expected.size (); i++)
    {
        expectedLog.logEvent (std::get<0> (expected[i]), 
                              std::get<1> (expected[i]));
    }
    
    // Verify actual == expected.
    VERIFY_LOGS;
}

/* Test Data Vector read and write methods. */
TEST_GROUP (DataVector_acquireReleaseLock)
{

};

/* Test acquiring lock twice. */
TEST (DataVector_acquireReleaseLock, AcquireTwice)
{
    // Create DV
    INIT_DATA_VECTOR (gSynchronizationConfig);

    CHECK_SUCCESS (pDv->acquireLock ());
    CHECK_ERROR (pDv->acquireLock (), E_FAILED_TO_LOCK);
}

/* Test releasing lock twice. */
TEST (DataVector_acquireReleaseLock, ReleaseTwice)
{
    // Create DV
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Fail to release lock since don't have lock.
    CHECK_ERROR (pDv->releaseLock (), E_FAILED_TO_UNLOCK);

    // Acquire lock.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Release successfully.
    CHECK_SUCCESS (pDv->releaseLock ());

    // Fail to release a second time.
    CHECK_ERROR (pDv->releaseLock (), E_FAILED_TO_UNLOCK);
}

/* Test Data Vector lock synchronization between threads. */
TEST_GROUP (DataVector_threadSynchronization)
{

};


/* Verify acquireLock will dequeue highest priority waiting thread. */
TEST (DataVector_threadSynchronization, AcquireByPriority)
{
    std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> expected =
    {
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 2),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 3),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 1),
    };

    testLockAcquireSemantics (ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                              ThreadManager::MIN_NEW_THREAD_PRIORITY + 2,
                              ThreadManager::MIN_NEW_THREAD_PRIORITY + 1,
                              expected);
}

/* Verify acquireLock will dequeue in FIFO order when threads have the same
 * priority. */
TEST (DataVector_threadSynchronization, AcquireByFIFOWithSamePriority)
{
    std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> expected =
    {
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 1),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 2),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 3),
    };

    testLockAcquireSemantics (ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                              ThreadManager::MIN_NEW_THREAD_PRIORITY,
                              ThreadManager::MIN_NEW_THREAD_PRIORITY,
                              expected);
}

/* Verify low pri thread not blocked on lockRelease when a lower pri thread is
 * waiting on the lock. */
TEST (DataVector_threadSynchronization, ReleaseNoBlock_LowPriWaiter)
{
    std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> expected =
    {
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 1),
        std::make_tuple (Log::LogEvent_t::RELEASED_LOCK, 1),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 2),
    };

    testLockReleaseSemantics (ThreadManager::MIN_NEW_THREAD_PRIORITY + 1, 
                              ThreadManager::MIN_NEW_THREAD_PRIORITY,
                              expected);
}

/* Verify low pri thread not blocked on lockRelease when a thread with the
 * same priority is waiting for the lock. */
TEST (DataVector_threadSynchronization, ReleaseNoBlock_SamePriWaiter)
{
    std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> expected =
    {
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 1),
        std::make_tuple (Log::LogEvent_t::RELEASED_LOCK, 1),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 2),
    };

    testLockReleaseSemantics (ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                              ThreadManager::MIN_NEW_THREAD_PRIORITY,
                              expected);
}

/* Verify low pri thread blocked on lockRelease when a higher pri thread 
 * is waiting for the lock. */
TEST (DataVector_threadSynchronization, ReleaseNoBlock_HighPriWaiter)
{
    std::vector<std::tuple<Log::LogEvent_t, Log::LogInfo_t>> expected =
    {
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 1),
        std::make_tuple (Log::LogEvent_t::ACQUIRED_LOCK, 2),
        std::make_tuple (Log::LogEvent_t::RELEASED_LOCK, 1),
    };

    testLockReleaseSemantics (ThreadManager::MIN_NEW_THREAD_PRIORITY, 
                              ThreadManager::MAX_NEW_THREAD_PRIORITY,
                              expected);
}

/* Verify read will block until lock is available. */
TEST (DataVector_threadSynchronization, ReadBlocked)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize thread.
    pthread_t t1;
    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    ThreadManager::ThreadFunc_t *pThreadFuncRead = 
        (ThreadManager::ThreadFunc_t *) &threadFuncRead;

    // Write initial value to DV.
    pDv->write (DV_ELEM_TEST0, (uint8_t) 1);

    // Acquire lock so that thread blocks on read attempt.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Create thread and sleep so that thread blocks on read.
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncRead,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Write new value to DV.
    CHECK_SUCCESS (pDv->writeImpl (DV_ELEM_TEST0, (uint8_t) 2));

    // Release lock and sleep. Expect this to unblock t1, resulting in t1
    // reading the updated value.
    pDv->releaseLock ();
    TestHelpers::sleepMs (100);

    // Build expected log.
    expectedLog.logEvent (Log::LogEvent_t::READ_VALUE, 2);
    
    // Verify expected == actual.
    VERIFY_LOGS;
}

/* Verify write will block until lock is available. */
TEST (DataVector_threadSynchronization, WriteBlocked)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize thread.
    pthread_t t1;
    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    ThreadManager::ThreadFunc_t *pThreadFuncWrite = 
        (ThreadManager::ThreadFunc_t *) &threadFuncWrite;

    // Acquire lock so that thread blocks on write attempt.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Create thread and sleep so that thread blocks on read.
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncWrite,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Verify value is still 0.
    uint8_t value = 0;
    CHECK_SUCCESS (pDv->readImpl (DV_ELEM_TEST0, value));
    CHECK_EQUAL (0, value);

    // Release lock and sleep. Expect this to unblock t1, resulting in t1
    // updating the value.
    pDv->releaseLock ();
    TestHelpers::sleepMs (100);
   
    // Wait for thread.
    Error_t threadReturn;
    pThreadManager->waitForThread (t1, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    
    // Verify value is now 2.
    pDv->read (DV_ELEM_TEST0, value);
    CHECK_EQUAL (2, value);
}

/* Verify readRegion will block until lock is available. */
TEST (DataVector_threadSynchronization, ReadRegionBlocked)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize thread.
    pthread_t t1;
    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    ThreadManager::ThreadFunc_t *pThreadFuncReadRegion = 
        (ThreadManager::ThreadFunc_t *) &threadFuncReadRegion;

    // Write initial value to DV.
    pDv->write (DV_ELEM_TEST0, (uint8_t) 1);

    // Acquire lock so that thread blocks on read attempt.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Create thread and sleep so that thread blocks on read.
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncReadRegion,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Write new value to DV.
    CHECK_SUCCESS (pDv->writeImpl (DV_ELEM_TEST0, (uint8_t) 2));

    // Release lock and sleep. Expect this to unblock t1, resulting in t1
    // reading the updated value.
    pDv->releaseLock ();
    TestHelpers::sleepMs (100);

    // Build expected log.
    expectedLog.logEvent (Log::LogEvent_t::READ_VALUE, 2);
    
    // Verify expected == actual.
    VERIFY_LOGS;
}

/* Verify writeRegion will block until lock is available. */
TEST (DataVector_threadSynchronization, WriteRegionBlocked)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize thread.
    pthread_t t1;
    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    ThreadManager::ThreadFunc_t *pThreadFuncWriteRegion = 
        (ThreadManager::ThreadFunc_t *) &threadFuncWriteRegion;

    // Acquire lock so that thread blocks on write attempt.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Create thread and sleep so that thread blocks on read.
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncWriteRegion,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Verify value is still 0.
    uint8_t value = 0;
    CHECK_SUCCESS (pDv->readImpl (DV_ELEM_TEST0, value));
    CHECK_EQUAL (0, value);

    // Release lock and sleep. Expect this to unblock t1, resulting in t1
    // updating the value.
    pDv->releaseLock ();
    TestHelpers::sleepMs (100);
   
    // Wait for thread.
    Error_t threadReturn;
    pThreadManager->waitForThread (t1, threadReturn);
    CHECK_EQUAL (E_SUCCESS, ret);
    
    // Verify value is now 2.
    pDv->read (DV_ELEM_TEST0, value);
    CHECK_EQUAL (2, value);
}

/* Verify readDataVector will block until lock is available. */
TEST (DataVector_threadSynchronization, ReadDataVectorBlocked)
{
    INIT_THREAD_MANAGER_AND_LOGS;
    INIT_DATA_VECTOR (gSynchronizationConfig);

    // Initialize thread.
    pthread_t t1;
    struct ThreadFuncArgs argsThread1 = {&testLog, pDv, 1}; 
    ThreadManager::ThreadFunc_t *pThreadFuncReadDataVector = 
        (ThreadManager::ThreadFunc_t *) &threadFuncReadDataVector;

    // Write initial value to DV.
    pDv->write (DV_ELEM_TEST0, (uint8_t) 1);

    // Acquire lock so that thread blocks on read attempt.
    CHECK_SUCCESS (pDv->acquireLock ());

    // Create thread and sleep so that thread blocks on read.
    CHECK_SUCCESS (pThreadManager->createThread (
                                    t1, pThreadFuncReadDataVector,
                                    &argsThread1, sizeof (argsThread1),
                                    ThreadManager::MIN_NEW_THREAD_PRIORITY,
                                    ThreadManager::Affinity_t::CORE_0));
    TestHelpers::sleepMs (10);

    // Write new value to DV.
    CHECK_SUCCESS (pDv->writeImpl (DV_ELEM_TEST0, (uint8_t) 2));

    // Release lock and sleep. Expect this to unblock t1, resulting in t1
    // reading the updated value.
    pDv->releaseLock ();
    TestHelpers::sleepMs (100);

    // Build expected log.
    expectedLog.logEvent (Log::LogEvent_t::READ_VALUE, 2);
    
    // Verify expected == actual.
    VERIFY_LOGS;
}
