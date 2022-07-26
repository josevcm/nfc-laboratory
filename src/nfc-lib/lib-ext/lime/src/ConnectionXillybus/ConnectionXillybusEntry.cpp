/**
    @file ConnectionSTREAMEntry.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/
#ifdef __unix__
#include <unistd.h>
#endif
#include "ConnectionXillybus.h"
using namespace lime;
#include <fstream>


//! make a static-initialized entry in the registry
void __loadConnectionXillybusEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static ConnectionXillybusEntry XillybusEntry;
}


ConnectionXillybusEntry::ConnectionXillybusEntry(void):
    ConnectionRegistryEntry("PCIEXillybus")
{
}

ConnectionXillybusEntry::~ConnectionXillybusEntry(void)
{
}

std::vector<ConnectionHandle> ConnectionXillybusEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> handles;
    ConnectionHandle handle;
    handle.media = "PCI-E";


    for (unsigned i =0; i < ConnectionXillybus::deviceConfigs.size(); i++)
    {
#ifndef __unix__
        HANDLE fh = CreateFileA(ConnectionXillybus::deviceConfigs[i].ctrlWrite.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh != INVALID_HANDLE_VALUE || ::GetLastError() == ERROR_BUSY)
#else
        if(access(ConnectionXillybus::deviceConfigs[i].ctrlWrite.c_str(), F_OK ) != -1 )
#endif
        {
            if (hint.index < 0 || hint.index == i)
            {
                handle.name = ConnectionXillybus::deviceConfigs[i].name;
                handle.index = i;   
                handles.push_back(handle);
            }
#ifndef __unix__
            CloseHandle(fh);
#endif
        }
    }
    return handles;
}

IConnection *ConnectionXillybusEntry::make(const ConnectionHandle &handle)
{
    return new ConnectionXillybus(handle.index);
}
