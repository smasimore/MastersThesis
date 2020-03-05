/**
 * Methods for managing the node's global FPGA session. Best practice is to use
 * the global session for all device nodes and device unit tests. If a global
 * session is used, it is automatically closed and the FPGA API automatically
 * finalized on regular program exit (main returns or exit() is called). The
 * interface is designed so that the global session can be created, closed, and
 * created again as many times as is needed, but the FPGA API itself is
 * initialized only once and finalized only once.
 */

# ifndef FPGA_HPP
# define FPGA_HPP

#include <memory>

#include "Errors.hpp"
#include "NiFpga.h"
#include "NiFpga_IO.h"

namespace Fpga
{
    /**
     * Gets the global FPGA session. If no session is open, one is made.
     *
     * Note: the global session should ONLY be closed through
     * Fpga::closeSession().
     *
     * @param   kRet                Session return.
     *
     * @ret     E_SUCCESS           Successfully returned session.
     *          E_FPGA_INIT         Failed to initialize the FPGA API.
     *          E_FPGA_SESSION_INIT Tried to create a new session which did not
     *                              initialize successfully.
     */
    Error_t getSession (NiFpga_Session& kRet);

    /**
     * Gets the global FPGA session status.
     *
     * @param   kRet              Session status return.
     *
     * @ret     E_SUCCESS         Successfully returned status.
     *          E_FPGA_NO_SESSION A session is not currently open.
     */
    Error_t getStatus (NiFpga_Status& kRet);

    /**
     * Closes the global FPGA session. A new session one can safely be created
     * with Fpga::getSession().
     *
     * @ret     E_SUCCESS         Successfully closed.
     *          E_FPGA_NO_SESSION A session is not currently open.
     */
    Error_t closeSession ();
}

# endif
