/**
    @file ConnectionRemote.cpp
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#include "ConnectionRemote.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace lime;


//! make a static-initialized entry in the registry
void __loadConnectionRemoteEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static ConnectionRemoteEntry EVB7COMEntry;
}

ConnectionRemoteEntry::ConnectionRemoteEntry(void):
    ConnectionRegistryEntry("Z_Remote") //Z just to appear last on the list
{
    return;
}

std::vector<ConnectionHandle> ConnectionRemoteEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> result;

    ConnectionHandle handle;
    handle.media = "TCP";
    handle.name = "Remote";
    handle.addr = hint.addr;
    result.push_back(handle);

    return result;
}

IConnection *ConnectionRemoteEntry::make(const ConnectionHandle &handle)
{
    return new ConnectionRemote(handle.addr.c_str());
}
