/**
    @file ConnectionRemote.h
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS64CProtocol.h>
#include <vector>
#include <string>
#include "IConnection.h"

namespace lime{

class ConnectionRemote : public LMS64CProtocol
{
public:
    struct Packet
    {
        uint8_t cmd;
        uint8_t data[64];
    };

    ConnectionRemote(const char *comName);
    ~ConnectionRemote(void);
    int TransferPacket(GenericPacket &pkt) override;
    bool IsOpen(void);
    eConnectionType GetType(void) {return CONNECTION_UNDEFINED;};
    //int TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size) override;

protected:
    int socketFd;
    int Connect(const char* ip, uint16_t port);

    //! virtual write function to be implemented by the base class
    int Write(const unsigned char *buffer, int length, int timeout_ms = 100) override;

    //! virtual read function to be implemented by the base class
    int Read(unsigned char *buffer, int length, int timeout_ms = 100) override;

    int GetBuffersCount() const override;
    int CheckStreamSize(int size) const override;
private:
    int Open();
    void Close(void);
    std::mutex mTransferLock;
    std::string remoteIP;
};

class ConnectionRemoteEntry : public ConnectionRegistryEntry
{
public:
    ConnectionRemoteEntry(void);
    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);
    IConnection *make(const ConnectionHandle &handle);
};

}
