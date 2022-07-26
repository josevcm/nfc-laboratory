/**
    @file ConnectionSPI.cpp
    @author Lime Microsystems
    @brief Implementation of SPI board connection.
*/
#ifdef __unix__
#include <unistd.h>
#endif
#include "ConnectionSPI.h"
using namespace lime;
#include <fstream>


void __loadConnectionSPIEntry(void)
{
    static ConnectionSPIEntry SPIEntry;
}


ConnectionSPIEntry::ConnectionSPIEntry(void):
    ConnectionRegistryEntry("SPI")
{
}

ConnectionSPIEntry::~ConnectionSPIEntry(void)
{
}

std::vector<ConnectionHandle> ConnectionSPIEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> handles;
    ConnectionHandle handle;
    handle.media = "SPI";

    if ((access("/dev/spidev1.0", F_OK ) != -1)
     && (access("/dev/spidev1.1", F_OK ) != -1)
     && (access("/dev/spidev1.2", F_OK ) != -1))
    {
        if (access("/dev/lime_spi", F_OK ) != -1)
        {
            handle.name = "Rasp PI 3 SPI (lime spi)";
            handle.index = ConnectionSPI::LIMESPI;
            handles.push_back(handle);
        }
        else if ((access("/dev/spidev0.0", F_OK ) != -1)
              && (access("/dev/spidev0.1", F_OK ) != -1))
        {
            handle.name = "Rasp PI 3 SPI (spidev)";
            handle.index = ConnectionSPI::SPIDEV;
            handles.push_back(handle);
        }

    }
    return handles;
}

IConnection *ConnectionSPIEntry::make(const ConnectionHandle &handle)
{
    return new ConnectionSPI(handle.index);
}
