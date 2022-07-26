/**
@file Logger.h
@author Lime Microsystems
@brief API for logging library status messages.
*/

#ifndef LIMESUITE_LOGGER_H
#define LIMESUITE_LOGGER_H

#include "LimeSuiteConfig.h"
#include <string>
#include <cstdarg>
#include <cerrno>
#include <stdexcept>

namespace lime
{

enum LogLevel
{
    LOG_LEVEL_CRITICAL = 0, //!< A critical error. The application might not be able to continue running successfully.
    LOG_LEVEL_ERROR    = 1, //!< An error. An operation did not complete successfully, but the application as a whole is not affected.
    LOG_LEVEL_WARNING  = 2, //!< A warning. An operation completed with an unexpected result.
    LOG_LEVEL_INFO     = 3, //!< An informational message, usually denoting the successful completion of an operation.
    LOG_LEVEL_DEBUG    = 4, //!< A debugging message, only shown in Debug configuration.
};

//! Log a critical error message with formatting
static inline void critical(const char *format, ...);

//! Log an error message with formatting
static inline int error(const char *format, ...);

//! Log a warning message with formatting
static inline void warning(const char *format, ...);

//! Log an information message with formatting
static inline void info(const char *format, ...);

//! Log a debug message with formatting
static inline void debug(const char *format, ...);

//! Log a message with formatting and specified logging level
static inline void log(const LogLevel level, const char *format, ...);

/*!
 * Send a message to the registered logger.
 * \param level a possible logging level
 * \param format a printf style format string
 * \param argList an argument list for the formatter
 */
LIME_API void log(const LogLevel level, const char *format, va_list argList);

/*!
 * Typedef for the registered log handler function.
 */
typedef void (*LogHandler)(const LogLevel level, const char *message);

/*!
 * Register a new system log handler.
 * Platforms should call this to replace the default stdio handler.
 */
LIME_API void registerLogHandler(const LogHandler handler);

//! Convert log level to a string name for printing
LIME_API const char *logLevelToName(const LogLevel level);

}

static inline void lime::log(const LogLevel level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    lime::log(level, format, args);
    va_end(args);
}

static inline void lime::critical(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    lime::log(lime::LOG_LEVEL_CRITICAL, format, args);
    va_end(args);
}

static inline int lime::error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    lime::log(lime::LOG_LEVEL_ERROR, format, args);
    va_end(args);
    return -1;
}

static inline void lime::warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    lime::log(lime::LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

static inline void lime::info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    lime::log(lime::LOG_LEVEL_INFO, format, args);
    va_end(args);
}

static inline void lime::debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    lime::log(lime::LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

namespace lime
{

/*!
 * Get the error code to string + any optional message reported.
 */
LIME_API const char *GetLastErrorMessage(void);

/*!
 * Report a typical errno style error.
 * The resulting error message comes from strerror().
 * \param errnum a recognized error code
 * \return a non-zero status code to return
 */
LIME_API int ReportError(const int errnum);

/*!
 * Report an error as an integer code and a formatted message string.
 * \param errnum a recognized error code
 * \param format a format string followed by args
 * \return a non-zero status code to return
 */
inline int ReportError(const int errnum, const char *format, ...);

/*!
 * Report an error as a formatted message string.
 * The reported errnum is 0 - no relevant error code.
 * \param format a format string followed by args
 * \return a non-zero status code to return
 */
inline int ReportError(const char *format, ...);

/*!
 * Report an error as an integer code and message format arguments
 * \param errnum a recognized error code
 * \param format a printf-style format string
 * \param argList the format string args as a va_list
 * \return a non-zero status code to return
 */
LIME_API int ReportError(const int errnum, const char *format, va_list argList);

}

inline int lime::ReportError(const int errnum, const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    int status = lime::ReportError(errnum, format, argList);
    va_end(argList);
    return status;
}

inline int lime::ReportError(const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    int status = lime::ReportError(-1, format, argList);
    va_end(argList);
    return status;
}

#endif //LIMESUITE_LOGGER_H
