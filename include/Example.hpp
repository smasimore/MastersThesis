/**
 * Example include file to establish directory structure and demonstrate test
 * suite.
 */
#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include <stdint.h>
#include "Errors.h"

/**
 * Example function.
 *
 * @param   result      Reference to uint8_t to store result in.
 * 
 * @ret     E_SUCCESS   0 successfully stored in result.
 */
Error_t retZero (uint8_t &result);

#endif
