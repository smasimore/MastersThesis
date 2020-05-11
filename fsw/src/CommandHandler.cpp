#include "CommandHandler.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t CommandHandler::createNew (Config_t kConf,
                                   std::shared_ptr<DataVector> kPDv,
                                   std::unique_ptr<CommandHandler>& kPChRet)
{
    // Verify DV not null.
    if (kPDv == nullptr)
    {
        return E_DATA_VECTOR_NULL;
    }

    // Verify elements exist.
    if (kPDv->elementExists (kConf.cmd)            != E_SUCCESS ||
        kPDv->elementExists (kConf.cmdReq)         != E_SUCCESS ||
        kPDv->elementExists (kConf.cmdWriteElem)   != E_SUCCESS ||
        kPDv->elementExists (kConf.cmdWriteVal)    != E_SUCCESS ||
        kPDv->elementExists (kConf.cmdReqNum)  != E_SUCCESS ||
        kPDv->elementExists (kConf.lastCmdProcNum) != E_SUCCESS)
    {
        return E_INVALID_ELEM;
    }

    // Verify element types.
    DataVectorElementType_t cmdType     = DV_T_LAST;
    DataVectorElementType_t cmdReqType  = DV_T_LAST;
    DataVectorElementType_t cmdElemType = DV_T_LAST;
    DataVectorElementType_t cmdValType  = DV_T_LAST;
    DataVectorElementType_t lastReqType = DV_T_LAST;
    DataVectorElementType_t lastPrType  = DV_T_LAST;
    if (kPDv->getElementType (kConf.cmd, cmdType)               != E_SUCCESS ||
        kPDv->getElementType (kConf.cmdReq, cmdReqType)         != E_SUCCESS ||
        kPDv->getElementType (kConf.cmdWriteElem, cmdElemType)  != E_SUCCESS ||
        kPDv->getElementType (kConf.cmdWriteVal, cmdValType)    != E_SUCCESS ||
        kPDv->getElementType (kConf.cmdReqNum, lastReqType) != E_SUCCESS ||
        kPDv->getElementType (kConf.lastCmdProcNum, lastPrType) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }
    if (cmdType     != DV_T_UINT8  ||
        cmdReqType  != DV_T_UINT8  ||
        cmdElemType != DV_T_UINT32 ||
        cmdValType  != DV_T_UINT64 ||
        lastReqType != DV_T_UINT32 ||
        lastPrType  != DV_T_UINT32)
    {
        return E_INVALID_TYPE;
    }

    // Create Command Handler.
    kPChRet.reset (new CommandHandler (kConf, kPDv));

    return E_SUCCESS;
}

Error_t CommandHandler::run ()
{
    // 1) Read command request and last command request number from GROUND, as
    //    well as last command processed number.
    uint8_t  cmdReq         = CMD_NONE;
    uint32_t cmdElem        = DV_ELEM_LAST;
    uint64_t cmdVal         = 0;
    uint32_t cmdReqNum      = 0;
    uint32_t lastCmdProcNum = 0;
    if (mPDv->read (mConfig.cmdReq,         cmdReq)         != E_SUCCESS ||
        mPDv->read (mConfig.cmdWriteElem,   cmdElem)        != E_SUCCESS ||
        mPDv->read (mConfig.cmdWriteVal,    cmdVal)         != E_SUCCESS ||
        mPDv->read (mConfig.cmdReqNum,      cmdReqNum)      != E_SUCCESS ||
        mPDv->read (mConfig.lastCmdProcNum, lastCmdProcNum) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // 2) Check validity of command.
    if (isValidCommand ((Command_t) cmdReq) != E_SUCCESS)
    {
        return E_INVALID_CMD;
    }

    // 3) If received a new command, handle. Otherwise, clear current command.
    if (lastCmdProcNum < cmdReqNum)
    {
        // Update last command processed number. This ensures the next time the
        // handler is called, the command will be cleared (unless a new command
        // is requested).
        if (mPDv->write (mConfig.lastCmdProcNum, cmdReqNum) != E_SUCCESS)
        {
            return E_DATA_VECTOR_WRITE;
        }    
        
        // Write new command request to Control Node's command element.
        if (mPDv->write (mConfig.cmd, cmdReq))
        {
            return E_DATA_VECTOR_WRITE;
        }

        // Handle command.
        Error_t ret = E_SUCCESS;
        switch ((Command_t) cmdReq)
        {
            case CMD_NONE:
            case CMD_LAUNCH:
            case CMD_ABORT:
                // Do nothing. Handled in State Machine.
                break;
            case CMD_WRITE:
                ret = executeWriteCmd ((DataVectorElement_t) cmdElem, cmdVal);
                if (ret != E_SUCCESS)
                {
                    return ret;
                }
                break;
            default:
                // This should never happen due to validity check above.
                return E_INVALID_ENUM;
        }
    }
    else
    {
        // Clear active command.
        if (mPDv->write (mConfig.cmd, (uint8_t) CMD_NONE) != E_SUCCESS)
        {
            return E_DATA_VECTOR_WRITE;
        }
    }

    return E_SUCCESS;
}

/*************************** PRIVATE FUNCTIONS ******************************/

CommandHandler::CommandHandler (Config_t kConfig, 
                                std::shared_ptr<DataVector> kPDv) :
    mPDv    (kPDv),
    mConfig (kConfig) {}

Error_t CommandHandler::isValidCommand (Command_t kCmd)
{
    if (kCmd >= CMD_LAST)
    {
        return E_INVALID_CMD;;
    }

    return E_SUCCESS;
}

Error_t CommandHandler::executeWriteCmd (DataVectorElement_t kElem, 
                                         uint64_t kVal)
{
    // Verify element exists.
    if (mPDv->elementExists (kElem) != E_SUCCESS)
    {
        return E_INVALID_ELEM;
    }

    // Get element's type.
    DataVectorElementType_t type = DV_T_LAST;
    if (mPDv->getElementType (kElem, type) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    // Write value.
    Error_t ret = E_SUCCESS;
    switch (type)
    {
        case DV_T_UINT8:
            ret = mPDv->write (kElem, (uint8_t) kVal);
            break;
        case DV_T_UINT16:
            ret = mPDv->write (kElem, (uint16_t) kVal);
            break;
        case DV_T_UINT32:
            ret = mPDv->write (kElem, (uint32_t) kVal);
            break;
        case DV_T_UINT64:
            ret = mPDv->write (kElem, (uint64_t) kVal);
            break;
        case DV_T_INT8:
            ret = mPDv->write (kElem, (int8_t) kVal);
            break;
        case DV_T_INT16:
            ret = mPDv->write (kElem, (int16_t) kVal);
            break;
        case DV_T_INT32:
            ret = mPDv->write (kElem, (int32_t) kVal);
            break;
        case DV_T_INT64:
            ret = mPDv->write (kElem, (int64_t) kVal);
            break;
        case DV_T_FLOAT:
            ret = mPDv->write (kElem, (float) kVal);
            break;
        case DV_T_DOUBLE:
            ret = mPDv->write (kElem, (double) kVal);
            break;
        case DV_T_BOOL:
            ret = mPDv->write (kElem, (bool) kVal);
            break;
        default:
            // This should never occur due to how DV is initialized.
            return E_INVALID_ENUM;
    }

    if (ret != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}
