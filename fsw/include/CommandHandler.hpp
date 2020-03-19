/**
 * The Command Handler receives a command request from the GROUND computer, 
 * checks if it is a legal command, and processes the command. The handler is
 * designed to be run on the Control Node, which also runs the State Machine.
 * There are 3 types of commands supported:
 *
 *  1) CMD_LAUNCH: Handler writes the requested command to the Control Node's 
 *                 command Data Vector element. If configured as a transition 
 *                 condition in the current State, transitions to STATE_LAUNCH.
 *                 Command cleared after 1 Control Node loop.
 *
 *  2) CMD_ABORT:  Handler writes the command to the Control Node's command Data 
 *                 Vector element. If configured as a transition condition in 
 *                 the current State, transitions to the relevant abort state.
 *                 Command cleared after 1 Control Node loop.
 *
 *  3) CMD_WRITE:  Handler writes the command to the Control Node's command Data 
 *                 Vector element. Handler writes CMD_WRITE_VAL to Data Vector's
 *                 CMD_WRITE_ELEM. Command cleared after 1 Control Node loop 
 *                 (this does not affect value written to CMD_WRITE_ELEM).
 *
 * Notes: 
 *     
 *     #1 Only one command can be sent to the Control Node per loop.
 *
 *     #2 For WRITE commands, the GROUND computer sends the write value in a
 *        uint64_t. This is cast to whatever type the write elem has.
 */

#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <stdint.h>
#include <memory>

#include "DataVector.hpp"
#include "Errors.hpp"

/**
 * Supported commands. Defined outside of class to shorten cmd-dependent config 
 * length.
 */
enum Command_t : uint8_t
{
    CMD_NONE,
    CMD_LAUNCH,
    CMD_ABORT,
    CMD_WRITE,

    CMD_LAST
};

class CommandHandler
{

    public:

        /**
         * Config.
         */
        typedef struct Config
        {
            /**
             * DV elem on Control Node to store active command in. Must be 
             * DV_T_UINT8.
             */
            DataVectorElement_t cmd;
            /**
             * DV elem containing most recent command request. Must be 
             * DV_T_UINT8.
             */
            DataVectorElement_t cmdReq;
            /**
             * DV elem containing most recent write command's DV elem to write
             * to. Must be DV_T_UINT32.
             */
            DataVectorElement_t cmdWriteElem;
            /**
             * DV elem containing most recent write command's value to write.
             * Must be DV_T_UINT64.
             */
            DataVectorElement_t cmdWriteVal;
            /**
             * DV elem containing the number of the last requested command. Used
             * to determine if current request has been processed yet. Must be 
             * DV_T_UINT32.
             */
            DataVectorElement_t lastCmdReqNum;
            /**
             * DV elem on Control Node to store number of the last processed 
             * command. Used to determine if current request has been processed
             * yet. Must be DV_T_UINT3
             */
            DataVectorElement_t lastCmdProcNum;
        } Config_t;

        /**
         * Entry point for creating a new Command Handler. Validates the passed 
         * in config.
         *
         * @param   kConf              Config.
         * @param   kPDv               Ptr to initialized Data Vector.
         * @param   kPChRet            Pointer to store resulting Command
         *                             Handler in.
         *
         * @ret    E_SUCCESS           CommandHandler successfully created.
         *         E_DATA_VECTOR_NULL  Data Vector ptr null.
         *         E_INVALID_ELEM      Invalid DV elem in config.
         *         E_DATA_VECTOR_READ  Failed to read elem types from DV.
         *         E_INVALID_TYPE      Invalid DV elem type in config.
        */
        static Error_t createNew (Config_t kConf,
                                  std::shared_ptr<DataVector> kPDv,
                                  std::unique_ptr<CommandHandler>& kPChRet);

        /**
         * Run Command Handler logic once.
         *
         * @ret     E_SUCCESS            CommandHandler ran successfully.
         *          E_DATA_VECTOR_READ   Failed to read command data from DV.
         *          E_DATA_VECTOR_WRITE  Failed to write to DV.
         *          E_INVALID_CMD        Received invalid command request.
         *          E_INVALID_ENUM       Element had invalid type.
         *          E_INVALID_ELEM       Element to write not in DV.
         */
        Error_t run ();

    private:

        /**
         * Data Vector.
         */
        std::shared_ptr<DataVector> mPDv;

        /**
         * Config containing required DV elems.
         */
        Config_t mConfig;

        /**
         * Constructor.
         *
         * @param   kConfig             Config.
         * @param   kPDv                Ptr to initialized Data Vector.
         */
        CommandHandler (Config_t kConfig, 
                        std::shared_ptr<DataVector> kPDataVector);

        /**
         * Check if command is valid.
         *
         * @param  kCmd  Command to check.
         * 
         * @ret    E_SUCCESS      Valid command.
         *         E_INVALID_CMD  Invalid command.
         */
        Error_t isValidCommand (Command_t kCmd);

        /**
         * Execute a write command. kVal will be cast to kElem's type.
         *
         * @param  kElem                DV element to write to.
         * @param  kVal                 Value to write.
         *
         * @ret    E_SUCCESS            Successfully wrote.
         *         E_DATA_VECTOR_READ   Failed to read elem type.
         *         E_DATA_VECTOR_WRITE  Failed to write to DV.
         *         E_INVALID_ENUM       Element had invalid type.
         */
        Error_t executeWriteCmd (DataVectorElement_t kElem, uint64_t kVal);
};

#endif
