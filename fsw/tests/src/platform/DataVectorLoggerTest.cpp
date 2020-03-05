/* All #include statements should come before the CppUTest include */
#include <fstream>
#include <iostream>
#include <cstdio>
#include <limits>

#include "Errors.hpp"
#include "DataVectorLogger.hpp"

#include "TestHelpers.hpp"

#define FILE_NAME "/home/admin/FlightSoftware/logger_test_file.log"
#define CSV_EXPECTED_HEADER "DV_REG_TEST0,DV_ELEM_TEST0,DV_REG_TEST1,"         \
                        "DV_ELEM_TEST1,DV_ELEM_TEST2,DV_ELEM_TEST3,"           \
                        "DV_ELEM_TEST4,DV_ELEM_TEST5,DV_ELEM_TEST6,"           \
                        "DV_ELEM_TEST7,DV_ELEM_TEST8,DV_ELEM_TEST9,"           \
                        "DV_ELEM_TEST10,DV_ELEM_TEST11,DV_ELEM_TEST12,\n"

/**
 * Verify file's contents equal expected.
 *
 * @param  kExpectedContents String containing expected file contents.
 *
 */
#define VERIFY_FILE_CONTENTS(kExpectedStr)                                     \
{                                                                              \
    std::ifstream f (FILE_NAME);                                               \
    if (f.good () == false)                                                    \
    {                                                                          \
        FAIL ("Failed to open file in VERIFY_FILE_CONTENTS");                  \
    }                                                                          \
    std::string actualStr ((std::istreambuf_iterator<char> (f)),               \
                           (std::istreambuf_iterator<char> ()));               \
    f.close ();                                                                \
    STRCMP_EQUAL (actualStr.c_str (), kExpectedStr.c_str ());                  \
}

/**
 * Initialize DV and Logger.
 *
 * @param  kDvConfig    Config for Data Vector.
 * @param  kLoggerMode  Logger mode.
 *
 */
#define CREATE_DV_AND_LOGGER(kDvConfig, kLoggerMode)                           \
    std::shared_ptr<DataVector> pDv;                                           \
    CHECK_SUCCESS (DataVector::createNew (kDvConfig, pDv));                    \
    std::shared_ptr<DataVectorLogger> pLogger;                                 \
    CHECK_SUCCESS (DataVectorLogger::createNew (kLoggerMode, pDv, FILE_NAME,   \
                                                pLogger));

/**
 * Update DV with test data.
 */
#define UPDATE_DV                                                              \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST0, std::numeric_limits<uint8_t>::max ()));     \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST1, std::numeric_limits<uint16_t>::max ()));    \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST2, std::numeric_limits<uint32_t>::max ()));    \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST3, std::numeric_limits<uint64_t>::max ()));    \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST4, std::numeric_limits<int8_t>::min ()));      \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST5, std::numeric_limits<int16_t>::min ()));     \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST6, std::numeric_limits<int32_t>::min ()));     \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST7, std::numeric_limits<int64_t>::min ()));     \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST8, (float) 1.2345678));                        \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST9, std::numeric_limits<float>::infinity ()));  \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST10, (double) -0.00234567));                    \
    CHECK_SUCCESS (                                                            \
        pDv->write (DV_ELEM_TEST11,                                            \
                    -1 * std::numeric_limits<double>::infinity ()));           \
    CHECK_SUCCESS (pDv->write (DV_ELEM_TEST12, (bool) true));

DataVector::Config_t gDvConfig =
// Regions
{
    ////////////////////////////////////////////////////////////////////////////

    // Region
    {DV_REG_TEST0,

    // Elements
    {
        DV_ADD_UINT8  (DV_ELEM_TEST0,  0    ),
    }},

    ////////////////////////////////////////////////////////////////////////////

    // Region
    {DV_REG_TEST1,

    // Elements
    {
        DV_ADD_UINT16 (DV_ELEM_TEST1,  0    ),
        DV_ADD_UINT32 (DV_ELEM_TEST2,  0    ),
        DV_ADD_UINT64 (DV_ELEM_TEST3,  0    ),
        DV_ADD_INT8   (DV_ELEM_TEST4,  0    ),
        DV_ADD_INT16  (DV_ELEM_TEST5,  0    ),
        DV_ADD_INT32  (DV_ELEM_TEST6,  0    ),
        DV_ADD_INT64  (DV_ELEM_TEST7,  0    ),
        DV_ADD_FLOAT  (DV_ELEM_TEST8,  0    ),
        DV_ADD_FLOAT  (DV_ELEM_TEST9,  0    ),
        DV_ADD_DOUBLE (DV_ELEM_TEST10,  0   ),
        DV_ADD_DOUBLE (DV_ELEM_TEST11,  0   ),
        DV_ADD_BOOL   (DV_ELEM_TEST12, false),
    }},

    ////////////////////////////////////////////////////////////////////////////
};

/*************************** CREATENEW TESTS *******************************/

/* Group of tests verifying verifyConfig method. */
TEST_GROUP (DataVectorLogger)
{
    void teardown ()
    {
        // Delete file if exists.
        std::remove (FILE_NAME);
    }
};

/* Test initialization with invalid mode. */
TEST (DataVectorLogger, InitInvalidMode)
{
    // Create DV.
    std::shared_ptr<DataVector> pDv; 
    CHECK_SUCCESS (DataVector::createNew (gDvConfig, pDv));

    // Create Logger.
    std::shared_ptr<DataVectorLogger> pLogger;
    CHECK_ERROR (DataVectorLogger::createNew (DataVectorLogger::Mode_t::LAST, 
                                              pDv, FILE_NAME, pLogger),
                 E_INVALID_ENUM);

    // Verify file does not exist.
    std::ifstream f (FILE_NAME);
    CHECK (!f.good ());
    f.close ();

    // Verify object not created.
    CHECK (pLogger == nullptr);
}

/* Test initialization with invalid mode. */
TEST (DataVectorLogger, InitInvalidFile)
{
    const std::string INVALID_FILE = "/invalid/file/path";

    // Create DV.
    std::shared_ptr<DataVector> pDv; 
    CHECK_SUCCESS (DataVector::createNew (gDvConfig, pDv));

    // Create Logger.
    std::shared_ptr<DataVectorLogger> pLogger;
    CHECK_ERROR (DataVectorLogger::createNew (DataVectorLogger::Mode_t::WATCH, 
                                              pDv, INVALID_FILE, pLogger),
                 E_FAILED_TO_OPEN_FILE);

    // Verify file does not exist.
    std::ifstream f (INVALID_FILE);
    CHECK (!f.good ());
    f.close ();

    // Verify file does not exist.
    std::ifstream ft (FILE_NAME);
    CHECK (!ft.good ());
    ft.close ();

    // Verify object not created.
    CHECK (pLogger == nullptr);
}

/* Test initialization where file does not exist. */
TEST (DataVectorLogger, InitNewFile)
{
    CREATE_DV_AND_LOGGER (gDvConfig, DataVectorLogger::Mode_t::WATCH);

    // Verify file now exists.
    std::ifstream f (FILE_NAME);
    CHECK (f.good ());
    f.close ();
}

/* Test initialization where file exists. */
TEST (DataVectorLogger, InitFileExists)
{
    // Create file and write file name to file.
    std::ofstream of (FILE_NAME);
    of << FILE_NAME;
    of.close ();

    // Verify file now has FILE_NAME written to it.
    std::string expectedStr = FILE_NAME;
    VERIFY_FILE_CONTENTS (expectedStr);

    CREATE_DV_AND_LOGGER (gDvConfig, DataVectorLogger::Mode_t::WATCH);

    // Verify file still exists.
    std::ifstream f (FILE_NAME);
    CHECK (f.good ());
    f.close ();

    // Verify file now empty.
    expectedStr = "";
    VERIFY_FILE_CONTENTS (expectedStr);
}

/* Test initialization in CSV mode. */
TEST (DataVectorLogger, InitSuccessCsv)
{
    CREATE_DV_AND_LOGGER (gDvConfig, DataVectorLogger::Mode_t::CSV);

    // Verify file created with header.
    std::string expectedStr = CSV_EXPECTED_HEADER;
    VERIFY_FILE_CONTENTS (expectedStr);
}

/* Test initialization in WATCH mode. */
TEST (DataVectorLogger, InitSuccessWatch)
{
    CREATE_DV_AND_LOGGER (gDvConfig, DataVectorLogger::Mode_t::WATCH);

    // Verify file created with header.
    std::string expectedStr = "";
    VERIFY_FILE_CONTENTS (expectedStr);
}

/******************************** LOG TESTS ***********************************/

/* Test logging in CSV mode. */
TEST (DataVectorLogger, LogCsv)
{
    CREATE_DV_AND_LOGGER (gDvConfig, DataVectorLogger::Mode_t::CSV);

    // Verify header logged.
    std::string expectedStr = CSV_EXPECTED_HEADER;
    VERIFY_FILE_CONTENTS (expectedStr);

    // Log first row (default DV values) and verify.
    pLogger->log ();
    expectedStr += ",0,,0,0,0,0,0,0,0,0.000000,0.000000,0.000000,0.000000,0,\n";
    VERIFY_FILE_CONTENTS (expectedStr);

    // Update DV values.
    UPDATE_DV;

    // Log second row (updated DV values) and verify.
    pLogger->log ();
    expectedStr += ",255,,65535,4294967295,18446744073709551615,-128,-32768,"
                   "-2147483648,-9223372036854775808,1.234568,inf,-0.002346,"
                   "-inf,1,\n";
    VERIFY_FILE_CONTENTS (expectedStr);
}

/* Test logging in WATCH mode. */
TEST (DataVectorLogger, LogWatch)
{
    CREATE_DV_AND_LOGGER (gDvConfig, DataVectorLogger::Mode_t::WATCH);

    // Log first row (default DV values) and verify file contents.
    pLogger->log ();
    std::string expectedStr = 
        "\n\n---------------------------------------------\n"
        "---------------- Data Vector ----------------\n"
        "---------------------------------------------\n\n\n\n"
        "Region: DV_REG_TEST0\n"
        "---------------------------------------------\n"
        "DV_ELEM_TEST0:                   0\n\n\n"
        "Region: DV_REG_TEST1\n"
        "---------------------------------------------\n"
        "DV_ELEM_TEST1:                   0\n"
        "DV_ELEM_TEST2:                   0\n"
        "DV_ELEM_TEST3:                   0\n"
        "DV_ELEM_TEST4:                   0\n"
        "DV_ELEM_TEST5:                   0\n"
        "DV_ELEM_TEST6:                   0\n"
        "DV_ELEM_TEST7:                   0\n"
        "DV_ELEM_TEST8:                   0.000000\n"
        "DV_ELEM_TEST9:                   0.000000\n"
        "DV_ELEM_TEST10:                  0.000000\n"
        "DV_ELEM_TEST11:                  0.000000\n"
        "DV_ELEM_TEST12:                  0\n\n";
    VERIFY_FILE_CONTENTS (expectedStr);

    // Update DV values.
    UPDATE_DV;

    // Log second row (updated DV values) and verify file contents.
    pLogger->log ();
    expectedStr = 
        "\n\n---------------------------------------------\n"
        "---------------- Data Vector ----------------\n"
        "---------------------------------------------\n\n\n\n"
        "Region: DV_REG_TEST0\n"
        "---------------------------------------------\n"
        "DV_ELEM_TEST0:                   255\n\n\n"
        "Region: DV_REG_TEST1\n"
        "---------------------------------------------\n"
        "DV_ELEM_TEST1:                   65535\n"
        "DV_ELEM_TEST2:                   4294967295\n"
        "DV_ELEM_TEST3:                   18446744073709551615\n"
        "DV_ELEM_TEST4:                   -128\n"
        "DV_ELEM_TEST5:                   -32768\n"
        "DV_ELEM_TEST6:                   -2147483648\n"
        "DV_ELEM_TEST7:                   -9223372036854775808\n"
        "DV_ELEM_TEST8:                   1.234568\n"
        "DV_ELEM_TEST9:                   inf\n"
        "DV_ELEM_TEST10:                  -0.002346\n"
        "DV_ELEM_TEST11:                  -inf\n"
        "DV_ELEM_TEST12:                  1\n\n";
    VERIFY_FILE_CONTENTS (expectedStr);
}
