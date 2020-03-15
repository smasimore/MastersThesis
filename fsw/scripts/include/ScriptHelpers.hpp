/**
 * Various utilities for writing scripts.
 */
# ifndef SCRIPT_HELPERS_HPP
# define SCRIPT_HELPERS_HPP

#include <cstdint>

/**
 * Kills the program with a message.
 *
 * @param   kFmt    Exit message.
 *          [other] Format arguments.
 */
#define ERROR(kFmt, ...)                                                       \
{                                                                              \
    printf (kFmt "\n", ##__VA_ARGS__);                                         \
    exit (0);                                                                  \
}

namespace ScriptHelpers
{
    /**
     * Sleeps the running thread.
     *
     * @param   kMs Time to sleep in milliseconds.
     */
    void sleepMs (uint32_t kMs);

    /**
     * Gets the current time. 
     *
     * @ret     Current time in seconds.
     */
    double timeS ();
}

# endif
