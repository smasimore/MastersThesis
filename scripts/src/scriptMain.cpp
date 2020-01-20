/**
 * Entry point for Script build configuration. Import your script's header file
 * here and call it from main().
 *
 *
 * CREATING AND RUNNING A SCRIPT
 *
 * 1. Write your all-in-one-cpp-file script like you normally would, but create
 *    an accompanying header that prototypes the entry point for the script
 *    inside of a relevant namespace, e.g. MyScript::main (int, char**);
 * 2. Implement the entry point in the script cpp file.
 * 3. Import the header at the top of this file (scriptMain.cpp) and call the
 *    script entry point from the main method below.
 * 4. Build and run the Script build configuration.
 */

#include "IgniterTest.hpp"
#include "ProfileCopyBuffer.hpp"
#include "ProfileLock.hpp"

int main (int ac, char** av)
{
    // IgniterTest::main (ac, av);
    // ProfileCopyBuffer::main (ac, av);
    // ProfileLock::main (ac, av);
}
