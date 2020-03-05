#include <iostream>
#include <stdlib.h>

#include "DataVector.hpp"
#include "Errors.hpp"

void Errors::exitOnError (Error_t kError, std::string kMsg)
{
    if (kError != E_SUCCESS)
    {
        std::cout << "\n---- ERROR ----" << std::endl;
        std::cout << "Error Number: " << kError << std::endl;
        std::cout << kMsg << std::endl;
        std::cout << "---------------" << std::endl;
        exit (EXIT_FAILURE);
    }
}

void Errors::incrementOnError (Error_t kError, 
                               std::shared_ptr<DataVector>& kPDv,
                               DataVectorElement_t kElem)
{
    if (kError != E_SUCCESS)
    {
        std::cout << "\n---- ERROR LOGGED ----" << std::endl;
        std::cout << "Error Number: " << kError << std::endl;
        std::cout << "----------------------" << std::endl;
       kPDv->increment (kElem); 
    }
}

