/**
@file SystemResources.h
@author Lime Microsystems
@brief APIs for locating system resources.
*/

#ifndef LIMESUITE_SYSTEM_RESOURCES_H
#define LIMESUITE_SYSTEM_RESOURCES_H

#include "LimeSuiteConfig.h"
#include <string>
#include <vector>

namespace lime
{

/*!
 * Get the root installation directory for the library.
 * Use the configured installation prefix for the library,
 * or override with the LIME_SUITE_ROOT environment variable.
 */
LIME_API std::string getLimeSuiteRoot(void);

/*!
 * Get the full path to the user's home directory
 */
LIME_API std::string getHomeDirectory(void);

/*!
 * Get the full path to the application data directory.
 * On a unix system this is typically $HOME/.local/share.
 * On a windows system this is typically %USERPROFILE%\AppData\Roaming.
 * Also supports the APPDATA environment variable to override the default value.
 */
LIME_API std::string getAppDataDirectory(void);

/*!
 * Get the full path to the library's configuration data directory.
 */
LIME_API std::string getConfigDirectory(void);

/*!
 * Get a list of directories to search for image resources.
 * Directories are returned in the order of search priority.
 * Priority goes to the LIME_SUITE_IMAGE_PATH environment variable,
 * and then images in the user's application data directory,
 * and finally images in the installation root directory.
 */
LIME_API std::vector<std::string> listImageSearchPaths(void);

/*!
 * Get the full file path to an image resource given only the file name.
 * @param name a unique name for the resource file including file extension
 * @return the full filesystem path to the resource if it exists or empty
 */
LIME_API std::string locateImageResource(const std::string &name);

/*!
 * Download an image resource given only the file name.
 * The resource will be downloaded in the user's application data directory.
 * @param name a unique name for the resource file including file extension
 * @return 0 for success or error code upon error
 */
LIME_API int downloadImageResource(const std::string &name);

}

#endif //LIMESUITE_SYSTEM_RESOURCES_H
