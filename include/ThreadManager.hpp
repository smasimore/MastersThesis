/**
 * Singleton class to manage threads. To use this class call 
 * ThreadManager::getInstance to get the singleton. The first time this is 
 * called, ThreadManager::init will be called to initialize the kernel 
 * scheduling environment (e.g. increase the real-time priority of the 
 * ktimersoftd so they are not starved by the fsw threads).
 * 
 * NOTE: This object is intended to be called from only one thread and is not
 *       threadsafe.
 */

# ifndef THREAD_MANAGER_HPP
# define THREAD_MANAGER_HPP

#include <stdint.h>
#include <pthread.h>
#include <vector>

#include "Errors.h"

class ThreadManager final 
{
public:

    /**
     * Construct the ThreadManager if it does not already exist and return it
     * in the pThreadManager param.
     * 
     * @param   pThreadManager      Pointer to pointer to ThreadManager object.     
     * 
     * @ret     E_SUCCESS           Successfully pointed pThreadManager to
     *                              ThreadManager object.
     */
    static Error_t getInstance (ThreadManager **ppThreadManager);

    /**
     * Create a thread.
     * 
     * @param   pThread         Pointer to pthread_t object to fill.
     * @param   schedPolicy     SCHED_FIFO or SCHED_RR (round robin)
     * 
     * @ret     E_SUCCESS           Successfully pointed pThreadManager to
     *                              ThreadManager object
     */
    Error_t createThread (pthread_t *pThread, uint8_t schedPolicy, 
                          uint8_t priority, uint8_t cpuAffinity, 
                          void *(threadFunc) (void *));

private:

    /* Whether or not the init function has been called. */
    static bool initialized;

    /**
     * Constructor.
     */        
    ThreadManager ();

    /**
     * Private copy constructor to enforce singleton.
     */
    ThreadManager (ThreadManager const &);

    /**
     * Private assignment operator to enforce singleton.
     */
    ThreadManager& operator=(ThreadManager const &);

    /**
     * Initialize the kernel scheduling environment. Currently this function:
     *      1. Sets the ktimersoftd threads (one per core) to have a higher 
     *         priority than the minimum SCHED_FIFO priority (their default 
     *         priority) so that they can preempt the fsw threads. Otherwise a 
     *         fsw thread that does not block will starve a ktimersoftd thread.
     * 
     * @ret    E_SUCCESS    Successfully initialized kernel scheduling env. 
     */
    Error_t init ();
};

#endif
