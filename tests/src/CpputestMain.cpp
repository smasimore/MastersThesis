#include "CppUTest/CommandLineTestRunner.h"
#include <iostream>

int main(int ac, char** av)
{
	std::cout << "Running all tests" << std::endl;
    return CommandLineTestRunner::RunAllTests(ac, av);
}
