/**
@file VersionInfo.h
@author Lime Microsystems
@brief API for querying version and build information.
*/

#ifndef LIMESUITE_VERSION_INFO_H
#define LIMESUITE_VERSION_INFO_H

#include "LimeSuiteConfig.h"
#include <string>

/*!
 * API version number which can be used as a preprocessor check.
 * The format of the version number is encoded as follows:
 * <b>(major << 16) | (minor << 8) | (8 bit increment)</b>.
 * Where the increment can be used to indicate implementation
 * changes, fixes, or API additions within a minor release series.
 *
 * The macro is typically used in an application as follows:
 * \code
 * #if defined(LIME_SUITE_API_VERSION) && (LIME_SUITE_API_VERSION >= 0x20161200)
 * // Use a newer feature from the LimeSuite library API
 * #endif
 * \endcode
 */
#define LIME_SUITE_API_VERSION 0x20190100

namespace lime
{
    /*!
     * Get the library version as a dotted string.
     * The format is major.minor.patch.build-extra.
     */
    LIME_API std::string GetLibraryVersion(void);

    /*!
     * Get the date of the build in "%Y-%M-%d" format.
     */
    LIME_API std::string GetBuildTimestamp(void);

    /*!
     * Get the LimeSuite library API version as a string.
     * The format of the version string is <b>major.minor.increment</b>,
     * where the digits are taken directly from <b>LIME_SUITE_API_VERSION</b>.
     */
    LIME_API std::string GetAPIVersion(void);

    /*!
     * Get the ABI/so version of the library.
     */
    LIME_API std::string GetABIVersion(void);

}

#endif //LIMESUITE_VERSION_INFO_H
