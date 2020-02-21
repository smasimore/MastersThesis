#include <iostream>
#include <stdlib.h>

#include "Errors.hpp"

void Errors::exitOnError (Error_t kError, std::string kMsg)
{
    if (kError != E_SUCCESS)
    {
        std::cout << "\n---- ERROR ----" << std::endl;
        std::cout << "Error Number: " << kError << std::endl;
        std::cout << kMsg << std::endl;
        exit (EXIT_FAILURE);
    }
}

