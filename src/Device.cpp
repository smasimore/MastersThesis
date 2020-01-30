#include "Device.hpp"

/*************************** PROTECTED FUNCTIONS ******************************/

Device::Device (NiFpga_Session& kSession, 
                std::shared_ptr<StateVector> kPStateVector) :
    mSession (kSession),
    mPStateVector (kPStateVector) {}
