/**
    @file ConnectionEVB7COM.h
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS64CProtocol.h>
#include <vector>
#include <string>

#ifndef __unix__
    #include <windows.h>
#endif

namespace lime{

class ConnectionEVB7COM : public LMS64CProtocol
{
public:
    ConnectionEVB7COM(const char *comName, int baudrate);

    ~ConnectionEVB7COM(void);

    bool IsOpen(void);

private:

    int Open(const char *comName, int baudrate);

    void Close(void);

    eConnectionType GetType(void)
    {
        return COM_PORT;
    }

    int Write(const unsigned char *buffer, int length, int timeout_ms = 100);
    int Read(unsigned char *buffer, int length, int timeout_ms = 100);

    #ifndef __unix__
        HANDLE hComm;
        COMMTIMEOUTS m_ctmoNew;
        COMMTIMEOUTS m_ctmoOld;
        OVERLAPPED m_osROverlap;
        OVERLAPPED m_osWOverlap;
        DCB m_dcbCommPort;
    #else
        int hComm; //com port file descriptor
    #endif
};

class ConnectionEVB7COMEntry : public ConnectionRegistryEntry
{
public:
    ConnectionEVB7COMEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    std::vector<std::string> FindAllComPorts();
    std::vector<std::string> FilterDeviceList(const std::vector<std::string> &);
};

}
