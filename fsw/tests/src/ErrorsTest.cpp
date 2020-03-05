#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "DataVector.hpp"
#include "Errors.hpp"

#include "TestHelpers.hpp"

/**
 * Fork a process and verify Errors:exitOnError behaves as expected.
 *
 * @param  kError     Error to test.
 * @param  kExpected  Expected process exit code.
 */
#define TEST_EXIT_ON_ERROR(kError, kExpected)                                 \
{                                                                             \
    pid_t pid = fork ();                                                      \
    if (pid == 0)                                                             \
    {                                                                         \
        Errors::exitOnError (kError, "Testing exitOnError");                  \
        exit (EXIT_SUCCESS);                                                  \
    }                                                                         \
    else if (pid > 0)                                                         \
    {                                                                         \
        int32_t status = 0;                                                   \
        pid_t waitedPid = waitpid (pid, &status, 0);                          \
        if (waitedPid != pid)                                                 \
        {                                                                     \
            FAIL ("Unknown PID waited for.");                                 \
        }                                                                     \
        if (WIFEXITED (status) != 0)                                          \
        {                                                                     \
            CHECK_EQUAL (kExpected, WEXITSTATUS (status));                    \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            FAIL ("Process exited unexpectedly.");                            \
        }                                                                     \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        FAIL ("Fork failed.");                                                \
    }                                                                         \
}

/**
 * Verify Errors:incrementOnError behaves as expected.
 *
 * @param  kError     Error to test.
 * @param  kPDv       Pointer to Data Vector.
 * @param  kElem      Data Vector element to increment.
 * @param  kExpVal    Expected value of element after Errors::incrementOnError 
 *                    call.
 */
#define TEST_INCREMENT_ON_ERROR(kError, kPDv, kElem, kExpVal)                 \
{                                                                             \
    Errors::incrementOnError (kError, kPDv, kElem);                           \
    uint8_t val = 0;                                                          \
    CHECK_SUCCESS (kPDv->read (kElem, val));                                  \
    CHECK_EQUAL (kExpVal, val);                                               \
}

TEST_GROUP (Errors)
{

};

/* Test exitOnError. */
TEST (Errors, exitOnError)
{
    TEST_EXIT_ON_ERROR (E_SUCCESS, EXIT_SUCCESS);
    TEST_EXIT_ON_ERROR (E_OVERFLOW, EXIT_FAILURE);
};

/* Test incrementOnError. */
TEST (Errors, incrementOnError)
{
    DataVector::Config_t dvConfig =
    {
        {DV_REG_TEST0,
        {
            DV_ADD_UINT8  (DV_ELEM_TEST0,  0    ),
        }},
    };
    std::shared_ptr<DataVector> pDv;
    CHECK_SUCCESS (DataVector::createNew (dvConfig, pDv));

    TEST_INCREMENT_ON_ERROR (E_SUCCESS, pDv, DV_ELEM_TEST0, 0);
    TEST_INCREMENT_ON_ERROR (E_OVERFLOW, pDv, DV_ELEM_TEST0, 1);
}
