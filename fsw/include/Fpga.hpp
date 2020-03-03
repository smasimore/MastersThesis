/**
 * Methods for managing the node's global FPGA session.
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
     * Fpga::closeSession.
     *
     * @param   kRet        Session return.
     *
     * @ret     E_SUCCESS   Successfully returned session.
     *          E_FPGA_INIT Tried to create a new session which did not
     *                      initialize successfully.
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
     * Closes the global FPGA session.
     *
     * @ret     E_SUCCESS         Successfully closed.
     *          E_FPGA_NO_SESSION A session is not currently open.
     */
    Error_t closeSession ();
}

# endif
