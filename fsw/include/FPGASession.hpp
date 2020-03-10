/**
 * Methods for managing the node's global FPGA session. Best practice is to use
 * the global session for all device nodes and device unit tests. If a global
 * session is used, it is automatically closed and the FPGA API automatically
 * finalized on regular program exit (main returns or exit() is called). The
 * interface is designed so that the global session can be created, closed, and
 * created again as many times as is needed, but the FPGA API itself is
 * initialized only once and finalized only once across the lifetime of a
 * program.
 *
 * WARNING: The global FPGA session is intended to be accessed from only one
 * thread and is not thread-safe.
 */

# ifndef FPGA_SESSION_HPP
# define FPGA_SESSION_HPP

#include <memory>

#include "Errors.hpp"
#include "NiFpga.h"
#include "NiFpga_IO.h"

namespace FPGASession
{
    /**
     * Gets the global FPGA session. If no session is open, one is made.
     *
     * Note 1: the global session should ONLY be closed through
     * FPGASession::closeSession().
     *
     * Note 2: this method may sleep the running thread briefly.
     *
     * @param   kSessionRet         Session return.
     * @param   kStatusRet          Session status return.
     *
     * @ret     E_SUCCESS           Successfully returned session.
     *          E_FPGA_INIT         Failed to initialize the FPGA API.
     *          E_FPGA_SESSION_INIT Tried to create a new session which did not
     *                              initialize successfully.
     */
    Error_t getSession (NiFpga_Session& kSessionRet, NiFpga_Status& kStatusRet);

    /**
     * PUBLIC FOR TESTING PURPOSES ONLY -- DO NOT USE OUTSIDE OF UNIT TESTS.
     *
     * Closes the global FPGA session. A new session can safely be created with
     * FPGASession::getSession().
     *
     * @param   kStatusRet           Session status return.
     *
     * @ret     E_SUCCESS            Successfully closed.
     *          E_FPGA_CLOSE_SESSION Attempting to close the session resulted
     *                               in an unsuccessful status.
     *          E_FPGA_NO_SESSION    A session is not currently open.
     */
    Error_t closeSession (NiFpga_Status& kStatusRet);
}

# endif
