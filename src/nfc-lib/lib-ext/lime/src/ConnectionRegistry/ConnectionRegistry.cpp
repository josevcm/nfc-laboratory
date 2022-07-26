/**
    @file   ConnectionRegistry.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of connection registry
*/

#include "ConnectionRegistry.h"
#include "IConnection.h"
#include <mutex>
#include <map>
#include <memory>
#include <iostream>
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace lime;

void __loadAllConnections(void);

/*******************************************************************
 * Registry data structures
 ******************************************************************/
static std::mutex &registryMutex(void)
{
    static std::mutex mutex;
    return mutex;
}

static std::map<std::string, ConnectionRegistryEntry *> registryEntries;


/*******************************************************************
 * Registry implementation
 ******************************************************************/
std::vector<ConnectionHandle> ConnectionRegistry::findConnections(const ConnectionHandle &hint)
{
    __loadAllConnections();
    std::lock_guard<std::mutex> lock(registryMutex());

    std::vector<ConnectionHandle> results;
    for (const auto &entry : registryEntries)
    {
        //filter by module name when specified
        if (not hint.module.empty() and hint.module != entry.first) continue;

        for (auto handle : entry.second->enumerate(hint))
        {
            //insert the module name, which can be filtered on in makeConnection()
            handle.module = entry.first;
            results.push_back(handle);
        }
    }
    return results;
}

IConnection *ConnectionRegistry::makeConnection(const ConnectionHandle &handle)
{
    __loadAllConnections();
    std::lock_guard<std::mutex> lock(registryMutex());

    //use the identifier as a hint to perform a discovery
    //only identifiers from the discovery function itself is used in the factory
    for (const auto &entry : registryEntries)
    {
        //filter by module name when specified
        if (not handle.module.empty() and handle.module != entry.first) continue;

        const auto r = entry.second->enumerate(handle);
        if (r.empty()) continue;

        auto realHandle = r.front(); //just pick the first
        realHandle.module = entry.first;

        return entry.second->make(realHandle);

    }

    return nullptr;
}

void ConnectionRegistry::freeConnection(IConnection *conn)
{
    //some client code may end up freeing a null connection
    if (conn == nullptr) return;

    std::lock_guard<std::mutex> lock(registryMutex());

    delete conn;
}

std::vector<std::string> ConnectionRegistry::moduleNames(void)
{
    __loadAllConnections();
    std::vector<std::string> names;
    std::lock_guard<std::mutex> lock(registryMutex());
    for (const auto &entry : registryEntries)
    {
        names.push_back(entry.first);
    }
    return names;
}

/*******************************************************************
 * Entry implementation
 ******************************************************************/
ConnectionRegistryEntry::ConnectionRegistryEntry(const std::string &name):
    _name(name)
{
    std::lock_guard<std::mutex> lock(registryMutex());
    registryEntries[_name] = this;
}

ConnectionRegistryEntry::~ConnectionRegistryEntry(void)
{
    std::lock_guard<std::mutex> lock(registryMutex());
    registryEntries.erase(_name);
}
