/**
    @file ConnectionHandle.h
    @author Lime Microsystems
    @brief Represent a connection to a device
*/

#pragma once

#include "LimeSuiteConfig.h"
#include <string>

namespace lime{
/*!
 * A connection handle identifies a particular connection.
 * Handles are used by the connection registry to return
 * enumeration results and to instantiate connections.
 *
 * When using ConnectionHandle to specify an enumeration hint,
 * simply fill in only the fields that the user is interested in.
 * Each default field (empty string, -1 index) will be ignored
 * by the connection registry entry discovery implementation.
 */
class LIME_API ConnectionHandle
{
public:

    ConnectionHandle(void);

    //! Create from serialized string with key=value pairs
    ConnectionHandle(const std::string &args);

    /*!
     * The name of the IConnection support module.
     * Example EVB7COM, EVB7STREAM, NOVENA7.
     */
    std::string module;

    /*!
     * The connection media type.
     * Example: SPI, COM, USB.
     */
    std::string media;

    /*!
     * The name of the device. Example: STREAM, NOVENA.
     */
    std::string name;

    /*!
     * An address such a URL, IP address, or similar.
     */
    std::string addr;

    /*!
     * A serial number which is unique to a particular device.
     */
    std::string serial;

    /*!
     * An internal index which may enumerate device handles.
     */
    long long index;

    /*!
     * Serialize this connection handle into a string format.
     * This string format can be used to represent the handle.
     */
    std::string serialize(void) const;

    /*!
     * Get a displayable string for this handle.
     * @return a string that may be printed
     */
    std::string ToString(void) const;
};

}
//! Check two connection handles for equality
bool operator==(const lime::ConnectionHandle &lhs, const lime::ConnectionHandle &rhs);
