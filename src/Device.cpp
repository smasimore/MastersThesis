#include "Device.hpp"

/*************************** PROTECTED FUNCTIONS ******************************/

Device::Device (NiFpga_Session& kSession, 
                std::shared_ptr<DataVector> kPDataVector) :
    mSession     (kSession),
    mPDataVector (kPDataVector) {}
