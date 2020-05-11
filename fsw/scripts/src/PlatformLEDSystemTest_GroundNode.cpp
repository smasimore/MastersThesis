/**
 * See PlatformLEDSystemTest_Config.hpp for instructions on running test.
 */

#include <unistd.h>

#include "DataVector.hpp"
#include "DataVectorLogger.hpp"
#include "CommandHandler.hpp"
#include "PlatformLEDSystemTest_GroundNode.hpp"

/*********************************** MAIN *************************************/

void PlatformLEDSystemTest_GroundNode::main (int, char**)
{
    // 1) Init Ground Node's Data Vector.
    std::shared_ptr<DataVector> pDv = nullptr;
    Errors::exitOnError (DataVector::createNew (
                             PlatformLEDSystemTest_Config::mGndDvConfig, 
                             pDv), 
                         "DV init");
    
    // 2) Init Data Vector to copy telemetry into.
    std::shared_ptr<DataVector> pTelemDv = nullptr;
    Errors::exitOnError (DataVector::createNew (
                             PlatformLEDSystemTest_Config::mCnDvConfig, 
                             pTelemDv), 
                         "Telem DV init");

    // 3) Init telemetry logger.
    const std::string LOG_FILE = "/home/sarah/led_system_test.csv";
    std::shared_ptr<DataVectorLogger> pLogger = nullptr;
    Errors::exitOnError (DataVectorLogger::createNew (
                             DataVectorLogger::Mode_t::CSV,
                             pTelemDv, LOG_FILE, pLogger),
                         "Logger init");

    // 4) Init Network Manager.
    std::shared_ptr<NetworkManager> pNm = nullptr;
    Errors::exitOnError (NetworkManager::createNew (
                             PlatformLEDSystemTest_Config::mGndNmConfig, 
                             pDv, pNm), 
                         "NM init");

    // 5) Init Time Module.
    Time* pTime = nullptr;
    Errors::exitOnError (Time::getInstance (pTime), "Time init");

    // 6) Init static buffers for tx/rx'ing.
    uint32_t telemRecvSizeBytes = 0;
    uint32_t gndSendRegSizeBytes = 0;
    Errors::exitOnError (pTelemDv->getDataVectorSizeBytes (telemRecvSizeBytes),
                         "Get telem DV size");
    Errors::exitOnError (pDv->getRegionSizeBytes (DV_REG_GROUND_TO_CN, 
                                                  gndSendRegSizeBytes),
                         "Get region size");
    std::vector<uint8_t> telemRecvBuf (telemRecvSizeBytes);
    std::vector<uint8_t> regSendBuf   (gndSendRegSizeBytes);

    // 7) Loop.
    while (1)
    {
        // 7a) Receive telem and write to telemetry Data Vector.
        Errors::exitOnError (pNm->recvBlock (NODE_CONTROL, telemRecvBuf),
                             "Recv telem");
        Errors::exitOnError (pTelemDv->writeDataVector (telemRecvBuf),
                             "DV write");

        // 7b) Log telem to file.
        Errors::exitOnError (pLogger->log (), "Log");

        // 7c) Get rocket's current state.
        uint32_t state = STATE_A;
        Errors::exitOnError (pTelemDv->read (DV_ELEM_STATE, state), "DV read");

        // 7d) If in STATE_A and haven't sent command yet, send LAUNCH 
        //     command after 3s has elapsed.
        static bool sentLaunchCmd = false;
        if ((StateId_t) state == STATE_A && sentLaunchCmd == false)
        {
            static Time::TimeNs_t stateAStartTimeNs = 0;
            if (stateAStartTimeNs == 0)
            {
                Errors::exitOnError (pTime->getTimeNs (stateAStartTimeNs), 
                                     "Time read");
            }

            Time::TimeNs_t currTimeNs = 0;
            Errors::exitOnError (pTime->getTimeNs (currTimeNs), 
                                 "Time read");

            // Check if 3s has elapsed in STATE_A.
            if (currTimeNs - stateAStartTimeNs >= 3 * Time::NS_IN_S)
            {
                // Set LAUNCH cmd and increment request number.
                Errors::exitOnError (pDv->write (DV_ELEM_CMD_REQ, 
                                                 (uint8_t) CMD_LAUNCH), 
                                     "DV write");
                Errors::exitOnError (pDv->increment (DV_ELEM_CMD_REQ_NUM), 
                                     "DV increment");

                // Copy send Region to buffer.
                Errors::exitOnError (pDv->readRegion (DV_REG_GROUND_TO_CN,
                                                      regSendBuf),
                                     "DV Region read");

                // Send Region to Control Node once.
                Errors::exitOnError (pNm->send (NODE_CONTROL, regSendBuf),
                                     "Send Region");
                sentLaunchCmd = true;
            }
        }

        // 7e) If in STATE_D and haven't sent command yet, send ABORT
        //     command after 3s has elapsed.
        static bool sentAbortCmd = false;
        if ((StateId_t) state == STATE_D && sentAbortCmd == false)
        {
            static Time::TimeNs_t stateDStartTimeNs = 0;
            if (stateDStartTimeNs == 0)
            {
                Errors::exitOnError (pTime->getTimeNs (stateDStartTimeNs), 
                                     "Time read");
            }

            Time::TimeNs_t currTimeNs = 0;
            Errors::exitOnError (pTime->getTimeNs (currTimeNs), 
                                 "Time read");

            // Check if 3s has elapsed in STATE_D.
            if (currTimeNs - stateDStartTimeNs >= 3 * Time::NS_IN_S)
            {
                // Set ABORT cmd and increment request number.
                Errors::exitOnError (pDv->write (DV_ELEM_CMD_REQ, 
                                                 (uint8_t) CMD_ABORT), 
                                     "DV write");
                Errors::exitOnError (pDv->increment (DV_ELEM_CMD_REQ_NUM), 
                                     "DV increment");

                // Copy send Region to buffer.
                Errors::exitOnError (pDv->readRegion (DV_REG_GROUND_TO_CN,
                                                      regSendBuf),
                                     "DV Region read");

                // Send Region to Control Node once.
                Errors::exitOnError (pNm->send (NODE_CONTROL, regSendBuf),
                                     "Send Region");
                sentAbortCmd = true;
            }
        }
    }
}
