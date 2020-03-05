#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "Errors.hpp"

#include "CppUTest/TestHarness.h"

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


TEST_GROUP (Errors)
{

};

/* Test exitOnError. */
TEST (Errors, exitOnError)
{
    TEST_EXIT_ON_ERROR (E_SUCCESS, EXIT_SUCCESS);
    TEST_EXIT_ON_ERROR (E_OVERFLOW, EXIT_FAILURE);
};

