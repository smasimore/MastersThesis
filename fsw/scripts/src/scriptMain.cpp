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

//#include "ProfileCopyBuffer.hpp"
//#include "ProfileLock.hpp"
//#include "RecoveryIgniterTest.hpp"
//#include "ClockSyncTest_Client.hpp"
//#include "ClockSyncTest_Server.hpp"
//#include "ProfileFpgaApi.hpp"
//#include "ProfileEthernetRtt_ControlNode.hpp"
//#include "ProfileEthernetRtt_DeviceNode.hpp"
//#include "ProfilePlatformComms_ControlNode.hpp"
//#include "ProfilePlatformComms_DeviceNode.hpp"
//#include "ProfilePlatformJitter_ControlNode.hpp"
//#include "ProfilePlatformJitter_DeviceNode.hpp"
//#include "ProfilePlatformRxnTime_ControlNode.hpp"
//#include "ProfilePlatformRxnTime_DeviceNode.hpp"
//#include "ProfilePlatformOverhead_ControlNode.hpp"
//#include "ProfilePlatformOverhead_DeviceNode.hpp"
#include "PlatformLEDSystemTest_ControlNode.hpp"
#include "PlatformLEDSystemTest_DeviceNode.hpp"
#include "PlatformLEDSystemTest_GroundNode.hpp"

int main (int ac, char** av)
{
    // RecoveryIgniterTest::main (ac, (const char**) av);
    // ProfileCopyBuffer::main (ac, av);
    // ProfileLock::main (ac, av);
    // ClockSyncTest_Client::main (ac, av);
    // ClockSyncTest_Server::main (ac, av);
    // ProfileFpgaApi::main (ac, av);
    // ProfileEthernetRtt_ControlNode::main (ac, av);
    // ProfileEthernetRtt_DeviceNode::main (ac, av);
    // ProfilePlatformComms_ControlNode::main (ac, av);
    // ProfilePlatformComms_DeviceNode::main (ac, av);
    // ProfilePlatformJitter_ControlNode::main (ac, av);
    // ProfilePlatformJitter_DeviceNode::main (ac, av);
    // ProfilePlatformRxnTime_ControlNode::main (ac, av);
    // ProfilePlatformRxnTime_DeviceNode::main (ac, av);
    // ProfilePlatformOsverhead_ControlNode::main (ac, av);
    // ProfilePlatformOverhead_DeviceNode::main (ac, av);
	 PlatformLEDSystemTest_ControlNode::main (ac, av);
	// PlatformLEDSystemTest_DeviceNode::main (ac, av);
	// PlatformLEDSystemTest_GroundNode::main (ac, av);
}
