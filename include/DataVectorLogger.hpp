/**
 *
 * The Data Vector Logger is used to log Data Vector data to a file. The Logger 
 * provides 2 modes for logging:
 *
 * 1) CSV
 *
 *     Creates a new or overwrites existing file. Calling log () appends current 
 *     Data Vector values to file using csv formatting. 
 *
 * 2) WATCH
 *
 *     Creates a new file or overwrites existing file. Calling log () overwrites 
 *     file with a human-readable output of the Data Vector's values. Watch 
 *     file is intended to be read using the Linux watch command-line utility.
 *
 * WARNINGS  
 *
 *   #1 While it is fine to instantiate multiple Loggers, calling a single 
 *      Logger from multiple threads is not thread-safe.
 *
 *   #2 If a CSV Logger will be used on an sbRIO or any computer with limited 
 *      non-volatile storage, extra care must be taken to ensure there is enough 
 *      storage. There are currently no safe-guards against this in the Logger.
 *
 */

#ifndef DATA_VECTOR_LOGGER_HPP
#define DATA_VECTOR_LOGGER_HPP

#include <stdint.h>
#include <memory>
#include <fstream>
#include <cstring>
#include <unordered_map>

#include "Errors.hpp"
#include "EnumClassHash.hpp"
#include "DataVector.hpp"

/**************************** DATA VECTOR LOGGER CLASS *****************************/

class DataVectorLogger final 
{

public:

    /**
     * Logging modes.
     */
    enum Mode_t : uint8_t
    {
        CSV,
        WATCH,

        LAST
    };

    /**
     * Create a new DataVectorLogger.
     *
     * @param kMode       Logger mode.
     * @param kPDv        Pointer to Data Vector to print.
     * @param kFileName   File to create and write to. If using a path, must be
     *                    an absolute path.
     * @param kPLoggerRet Pointer to logger created.
     *
     * @ret               E_SUCCESS              Logger successfully created.
     *                    E_DATA_VECTOR_NULL     Data Vector ptr null. 
     *                    E_INVALID_ENUM         Invalid mode.
     *                    E_DATA_VECTOR_READ     Failed to read from DV.
     *                    E_FAILED_TO_OPEN_FILE  Failed to open file.
     *                    E_FAILED_TO_WRITE_FILE Failed to write header to CSV.
     */
    static Error_t createNew (Mode_t kMode, std::shared_ptr<DataVector>& kPDv,
                              std::string kFileName,
                              std::shared_ptr<DataVectorLogger>& kPLoggerRet);

    /**
     * Log to file. Behavior depends on mode Logger initialized with.
     *
     * @ret            E_SUCCESS               Successfully wrote to file.
     *                 E_INVALID_TYPE          Invalid element type.
     *                 E_DATA_VECTOR_READ      Failed to read from DV.
     *                 E_DATA_VECTOR_WRITE     Failed to write to snapshot DV.
     *                 E_FAILED_TO_SEEK        Failed to seek to beginning of
     *                                         file.
     *                 E_FAILED_TO_WRITE_FILE  Failed to write to file.
     */
    Error_t log ();

    /**
     * Destructor. Closes output stream.
     */
    ~DataVectorLogger ();

private:

    /**
     * Character number in line to start writing element value in watch mode.
     */
    static const uint32_t WATCH_ELEM_VALUE_START_POS;

    /**
     * Map from Data Vector region enum to string representation.
     */
    static const std::unordered_map<DataVectorRegion_t, 
                                    std::string, 
                                    EnumClassHash> mRegionToStr;

    /**
     * Map from Data Vector element enum to string representation.
     */
    static const std::unordered_map<DataVectorElement_t, 
                                    std::string, 
                                    EnumClassHash> mElemToStr;

    /**
     * Logger mode.
     */
    Mode_t mMode;

    /**
     * File stream to log to.
     */
    std::ofstream mOutputStream;

    /**
     * Pointer to node's active Data Vector.
     */
    std::shared_ptr<DataVector> mPDataVector;

    /**
     * Buffer used in DV copy operations. This is initialized on Logger 
     * construction to expedite copying.
     */
    std::vector<uint8_t> mCopyBuffer;

    /**
     * Copy of node's active Data Vector for thread synchronization. The active 
     * Data Vector can be changed by another thread during logging, so the first
     * step to logging a snapshot of the Data Vector is to copy the underlying
     * data to this variable.
     */
    std::unique_ptr<DataVector> mPDataVectorSnapshot;

    /**
     * Constructor.
     *
     * @param kMode       Logger mode.
     * @param kPDv        Pointer to Data Vector to print.
     * @param kFileName   File to create and write to.
     * @param kRet        E_SUCCESS              Logger created successfully.
     *                    E_DATA_VECTOR_READ     Failed to read from DV.
     *                    E_FAILED_TO_OPEN_FILE  Failed to open file.
     *                    E_FAILED_TO_WRITE_FILE Failed to write header to CSV.
     */
    DataVectorLogger (Mode_t kMode, std::shared_ptr<DataVector>& kPDv,
                      std::string kFileName, Error_t& kRet);

    /**
     * Write CSV header to file.
     * 
     * @ret   E_SUCCESS              Successfully wrote header.
     *        E_FAILED_TO_WRITE_FILE Failed to write header.
     */
    Error_t writeCsvHeader ();

    /**
     * Write CSV row to file.
     * 
     * @ret   E_SUCCESS              Successfully wrote row.
     *        E_INVALID_TYPE         Invalid element type.
     *        E_DATA_VECTOR_READ     Failed to read from DV.
     *        E_FAILED_TO_WRITE_FILE Failed to write row.
     */
    Error_t writeCsvRow ();

    /**
     * Write human-readable DV to watch file.
     * 
     * @ret   E_SUCCESS              Successfully wrote DV.
     *        E_INVALID_TYPE         Invalid element type.
     *        E_DATA_VECTOR_READ     Failed to read from DV.
     *        E_FAILED_TO_WRITE_FILE Failed to write DV.
     */
    Error_t writeWatch ();

    /**
     * Get string representation of element value.
     *
     * @param kElem                  Element to get value in string form.
     * @param kValueStrRet           Parameter to store string result.
     * 
     * @ret   E_SUCCESS              Successfully got value.
     *        E_INVALID_TYPE         Invalid element type.
     *        E_DATA_VECTOR_READ     Failed to read from DV.
     */
    Error_t getElementValueStr (DataVectorElement_t kElem,  
                                std::string& kValueStrRet);

};

#endif
