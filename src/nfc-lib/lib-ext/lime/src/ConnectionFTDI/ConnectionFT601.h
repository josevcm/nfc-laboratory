/**
@file Connection_uLimeSDR.h
@author Lime Microsystems
@brief Implementation of STREAM board connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include "LMS64CProtocol.h"
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <thread>

#ifndef __unix__
#include "windows.h"
#include "FTD3XXLibrary/FTD3XX.h"
#else
#include <libusb.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

namespace lime{

class ConnectionFT601 : public LMS64CProtocol
{
public:
    /** @brief Wrapper class for holding USB asynchronous transfers contexts
    */
    class USBTransferContext
    {
    public:
        USBTransferContext() : used(false)
        {
#ifndef __unix__
            context = NULL;
#else
            transfer = libusb_alloc_transfer(0);
            bytesXfered = 0;
            done = 0;
#endif
        }
        ~USBTransferContext()
        {
#ifdef __unix__
            libusb_free_transfer(transfer);
#endif
        }
        bool used;
#ifndef __unix__
        PUCHAR context;
        OVERLAPPED inOvLap;
#else
        libusb_transfer* transfer;
        long bytesXfered;
        std::atomic<bool> done;
        std::mutex transferLock;
        std::condition_variable cv;
#endif
    };

    ConnectionFT601(void *arg);
    ConnectionFT601(void *ctx, const ConnectionHandle &handle);

    virtual ~ConnectionFT601(void);

    int Open(const std::string &serial, int vid, int pid);
    void Close();
    bool IsOpen();
    int GetOpenedIndex();

    int Write(const unsigned char *buffer, int length, int timeout_ms = 100) override;
    int Read(unsigned char *buffer, int length, int timeout_ms = 100) override;

    int ProgramWrite(const char *data_src, size_t length, int prog_mode, int device, ProgrammingCallback callback) override;
    
    DeviceInfo GetDeviceInfo(void)override;
    
    int GPIOWrite(const uint8_t *buffer, size_t bufLength) override;
    int GPIORead(uint8_t *buffer, size_t bufLength) override;
    int GPIODirWrite(const uint8_t *buffer, size_t bufLength) override;
    int GPIODirRead(uint8_t *buffer, size_t bufLength) override;

protected:
    int GetBuffersCount() const override;
    int CheckStreamSize(int size) const override;
    int BeginDataReading(char* buffer, uint32_t length, int ep) override;
    bool WaitForReading(int contextHandle, unsigned int timeout_ms) override;
    int FinishDataReading(char* buffer, uint32_t length, int contextHandle) override;
    void AbortReading(int ep) override;

    int BeginDataSending(const char* buffer, uint32_t length, int ep) override;
    bool WaitForSending(int contextHandle, uint32_t timeout_ms) override;
    int FinishDataSending(const char* buffer, uint32_t length, int contextHandle) override;
    void AbortSending(int ep) override;
    
    int ResetStreamBuffers() override;

    eConnectionType GetType(void) {return USB_PORT;}
    
    static const int USB_MAX_CONTEXTS = 16; //maximum number of contexts for asynchronous transfers

    USBTransferContext contexts[USB_MAX_CONTEXTS];
    USBTransferContext contextsToSend[USB_MAX_CONTEXTS];

    bool isConnected;

    static const int streamWrEp;
    static const int streamRdEp;
    static const int ctrlWrEp;
    static const int ctrlRdEp;
#ifndef __unix__
    FT_HANDLE mFTHandle;
    int ReinitPipe(unsigned char ep);
#else
    int FT_SetStreamPipe(unsigned char ep, size_t size);
    int FT_FlushPipe(unsigned char ep);
    uint32_t mUsbCounter;
    libusb_device_handle *dev_handle; //a device handle
    libusb_context *ctx; //a libusb session
#endif
    std::mutex mExtraUsbMutex;
    uint64_t mSerial;
};

class ConnectionFT601Entry : public ConnectionRegistryEntry
{
public:
    ConnectionFT601Entry(void);
    ~ConnectionFT601Entry(void);
    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);
    IConnection *make(const ConnectionHandle &handle);
private:
#ifndef __unix__
    FT_HANDLE* mFTHandle;
#else
    libusb_context *ctx; //a libusb session
    std::thread mUSBProcessingThread;
    void handle_libusb_events();
    std::atomic<bool> mProcessUSBEvents;
#endif
};

}
