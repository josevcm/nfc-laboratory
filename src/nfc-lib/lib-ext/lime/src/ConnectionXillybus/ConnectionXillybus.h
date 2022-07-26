/**
    @file ConnectionSTREAM.h
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include "LMS64CProtocol.h"
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <thread>

#ifndef __unix__
#include "windows.h"
#else
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

namespace lime{

class ConnectionXillybus : public LMS64CProtocol
{
public:
    ConnectionXillybus(const unsigned index);
    ~ConnectionXillybus(void);

    int Open(const unsigned index);
    void Close();
    bool IsOpen();
    int GetOpenedIndex();

    int Write(const unsigned char *buffer, int length, int timeout_ms = 100) override;
    int Read(unsigned char *buffer, int length, int timeout_ms = 100) override;
#ifdef __unix__
    int TransferPacket(GenericPacket &pkt) override;
    int ProgramWrite(const char *data_src, const size_t length, const int prog_mode, const int device, ProgrammingCallback callback)override;
#endif
protected:
    int GetBuffersCount() const override;
    int CheckStreamSize(int size) const override;

    int ReceiveData(char* buffer, int length, int epIndex, int timeout = 100) override;
    int SendData(const char* buffer, int length, int epIndex, int timeout = 100) override;

    int BeginDataReading(char* buffer, uint32_t length, int ep) override;
    bool WaitForReading(int contextHandle, unsigned int timeout_ms) override;
    int FinishDataReading(char* buffer, uint32_t length, int contextHandle) override;
    void AbortReading(int epIndex);

    int BeginDataSending(const char* buffer, uint32_t length, int ep) override;
    bool WaitForSending(int contextHandle, uint32_t timeout_ms) override;
    int FinishDataSending(const char* buffer, uint32_t length, int contextHandle) override;
    void AbortSending(int epIndex);
private:
    friend class ConnectionXillybusEntry;
    static const int MAX_EP_CNT = 3;
    struct EPConfig
    {
        std::string name;
        std::string ctrlRead;
        std::string ctrlWrite;
        std::string streamRead[MAX_EP_CNT];
        std::string streamWrite[MAX_EP_CNT];
    };

    static const std::vector<EPConfig> deviceConfigs;
    eConnectionType GetType(void) {return PCIE_PORT;}

    std::string m_hardwareName;
    int m_hardwareVer;

    bool isConnected;
    std::mutex mTransferLock;
#ifndef __unix__
    HANDLE hWrite;
    HANDLE hRead;
    HANDLE hWriteStream[MAX_EP_CNT];
    HANDLE hReadStream[MAX_EP_CNT];
#else
    int OpenControl();
    void CloseControl();
    int hWrite;
    int hRead;
    int hWriteStream[MAX_EP_CNT];
    int hReadStream[MAX_EP_CNT];
#endif
    std::string writeCtrlPort;
    std::string readCtrlPort;
    std::string writeStreamPort[MAX_EP_CNT];
    std::string readStreamPort[MAX_EP_CNT];
};

class ConnectionXillybusEntry : public ConnectionRegistryEntry
{
public:
    ConnectionXillybusEntry(void);

    ~ConnectionXillybusEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    #ifndef __unix__
    std::string DeviceName(unsigned int index);
    #else
    #endif
};

}
