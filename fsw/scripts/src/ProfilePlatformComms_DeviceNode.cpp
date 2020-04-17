/**
 * See ProfilePlatform_Config.hpp for instructions on running test.
 */

#include <algorithm>
#include <unistd.h>

#include "DeviceNode.hpp"
#include "ProfilePlatformComms_DeviceNode.hpp"

/********************************* CONFIGS ************************************/

/**
 * Controller and Device initialization function.
 */
static Error_t initializeCtrlsAndDevs (
                         std::shared_ptr<DataVector> kPDv,                       
                         NiFpga_Session& kFpgaSession,                           
                         std::vector<std::unique_ptr<Controller>>& kPCtrls,         
                         std::vector<std::unique_ptr<Device>>& kPSensorDevs,        
                         std::vector<std::unique_ptr<Device>>& kPActuatorDevs)
{
    return E_SUCCESS;
}

/*********************************** MAIN *************************************/

void ProfilePlatformComms_DeviceNode::main (int, char**)
{
    DeviceNode::entry (
            ProfilePlatform_Config::mDnNmConfig, 
            ProfilePlatform_Config::mDnDvConfig,  
            (DeviceNode::fInitializeCtrlsAndDevs_t) initializeCtrlsAndDevs,
            false);
}
