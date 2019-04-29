/**
 * Lgger class for unit/integration tests. To use the logger:
 *      1. Create 2 Logger objects, expectedLog and actualLog
 *      2. For expectedLog, log the events you expect to happen using 
 *         ExpectedLog->logEvent (...)
 *      3. Run your test and log events throughout the test to the actualLog
 *         using actualLog->logEvent (...)
 *      4. Compare expectedLog to actualLog using 
 *         Logger::verify (expectedLog, actualLog).
 */

# ifndef LOG_HPP
# define LOG_HPP

#include <stdint.h>
#include <vector>

#include "Errors.h"

class Log final 
{
public:

    /* Log event describing what is being logged. */
    enum class LogEvent_t : uint32_t 
    {
        THREAD_START ,
        LAST
    };

    /* Extra element in a log row to store additional information (e.g. thread
       ID) */
    typedef uint32_t LogInfo_t;

    /**
     * Constructs a Log object. 
     */        
    Log ();

    /**
     * Log an event to the log.
     * 
     * @param   event       Event to log.
     * @param   info        Extra info to log in row.
     * 
     * @ret     E_SUCCESS   Successfully logged row.
     */
    Error_t logEvent (const LogEvent_t event, const LogInfo_t info);

    /**
     * Compare the rows of two logs to determine if they are equal.
     * 
     * @param logOne    First log.
     * @param logTwo    Second log. 
     * @param areEqual  Bool that will be set to true if the logs are equal and
     *                  false if they are not.
     * 
     * @ret             E_SUCCESS   areEqual set successfully.
     */
    static Error_t verify (const Log &logOne, const Log &logTwo, 
                           bool &areEqual);

private:

    /* Struct representing a row in the log. */
    struct LogRow {
        LogEvent_t event;
        LogInfo_t info;
    };

    /* Underlying data structure encapsulating log. */
    std::vector<struct LogRow> log;

};

#endif