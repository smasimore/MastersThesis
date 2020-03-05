#include "CppUTest/CommandLineTestRunner.h"
#include <iostream>

int main(int ac, char** av)
{
     // Turn off memory leak detection due to undiagnosed memory leak caused
     // by FPGA C API usage. This is a known NI issue and will only cause memory 
     // issues in production code if the FPGA is initialized more than once.
     //
     // http://www.ni.com/product-documentation/55093/en/#660205_by_Date
    MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();

    std::cout << "Running all FPGA tests" << std::endl;
    return CommandLineTestRunner::RunAllTests(ac, av);
}
