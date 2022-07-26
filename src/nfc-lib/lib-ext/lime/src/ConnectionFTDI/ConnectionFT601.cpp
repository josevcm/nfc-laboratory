/**
@file Connection_uLimeSDR.cpp
@author Lime Microsystems
@brief Implementation of uLimeSDR board connection.
*/

#include "ConnectionFT601.h"
#include <cstring>
#include <iostream>
#include <vector>

#include <thread>
#include <chrono>
#include <FPGA_common.h>
#include <ciso646>
#include "Logger.h"

using namespace std;
using namespace lime;

const int ConnectionFT601::streamWrEp = 0x03;
const int ConnectionFT601::streamRdEp = 0x83;
const int ConnectionFT601::ctrlWrEp = 0x02;
const int ConnectionFT601::ctrlRdEp = 0x82;

ConnectionFT601::ConnectionFT601(void *arg)
{
    isConnected = false;
#ifndef __unix__
    mFTHandle = NULL;
#else
    dev_handle = 0;
    mUsbCounter = 0;
    ctx = (libusb_context *)arg;
#endif
}

/**	@brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionFT601::ConnectionFT601(void *arg, const ConnectionHandle &handle)
{
    isConnected = false;
    int pid = -1;
    int vid = -1;
    mSerial = std::strtoll(handle.serial.c_str(),nullptr,16);
#ifndef __unix__
    mFTHandle = NULL;
#else
    const auto pidvid = handle.addr;
    const auto splitPos = pidvid.find(":");
    pid = std::stoi(pidvid.substr(0, splitPos));
    vid = std::stoi(pidvid.substr(splitPos+1));
    dev_handle = 0;
    mUsbCounter = 0;
    ctx = (libusb_context *)arg;
#endif
    if (this->Open(handle.serial, vid, pid) != 0)
        lime::error("Failed to open device");
}

/**	@brief Closes connection to chip and deallocates used memory.
*/
ConnectionFT601::~ConnectionFT601()
{
    Close();
}
#ifdef __unix__
int ConnectionFT601::FT_FlushPipe(unsigned char ep)
{
    int actual = 0;
    unsigned char wbuffer[20]={0};

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[4] = ep;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
        return -1;

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[4] = ep;
    wbuffer[5] = 0x03;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
        return -1;
    return 0;
}

int ConnectionFT601::FT_SetStreamPipe(unsigned char ep, size_t size)
{
    int actual = 0;
    unsigned char wbuffer[20]={0};

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[4] = ep;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
        return -1;

    mUsbCounter++;
    wbuffer[0] = (mUsbCounter)&0xFF;
    wbuffer[1] = (mUsbCounter>>8)&0xFF;
    wbuffer[2] = (mUsbCounter>>16)&0xFF;
    wbuffer[3] = (mUsbCounter>>24)&0xFF;
    wbuffer[5] = 0x02;
    wbuffer[8] = (size)&0xFF;
    wbuffer[9] = (size>>8)&0xFF;
    wbuffer[10] = (size>>16)&0xFF;
    wbuffer[11] = (size>>24)&0xFF;
    libusb_bulk_transfer(dev_handle, 0x01, wbuffer, 20, &actual, 1000);
    if (actual != 20)
        return -1;
    return 0;
}
#endif

/**	@brief Tries to open connected USB device and find communication endpoints.
@return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionFT601::Open(const std::string &serial, int vid, int pid)
{
#ifndef __unix__
    DWORD devCount;
    FT_STATUS ftStatus = FT_OK;
    DWORD dwNumDevices = 0;
    // Open a device
    ftStatus = FT_Create((void*)serial.c_str(), FT_OPEN_BY_SERIAL_NUMBER, &mFTHandle);
    if (FT_FAILED(ftStatus))
    {
        ReportError(ENODEV, "Failed to list USB Devices");
        return -1;
    }
    FT_AbortPipe(mFTHandle, streamRdEp);
    FT_AbortPipe(mFTHandle, ctrlRdEp);
    FT_AbortPipe(mFTHandle, ctrlWrEp);
    FT_AbortPipe(mFTHandle, streamWrEp);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, ctrlRdEp, 64);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, ctrlWrEp, 64);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamRdEp, sizeof(FPGA_DataPacket));
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamWrEp, sizeof(FPGA_DataPacket));
    FT_SetPipeTimeout(mFTHandle, ctrlWrEp, 500);
    FT_SetPipeTimeout(mFTHandle, ctrlRdEp, 500);
    FT_SetPipeTimeout(mFTHandle, streamRdEp, 0);
    FT_SetPipeTimeout(mFTHandle, streamWrEp, 0);
    isConnected = true;
    return 0;
#else

    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0)
        return ReportError(-1, "libusb_get_device_list failed: %s", libusb_strerror(libusb_error(usbDeviceCount)));

    for(int i=0; i<usbDeviceCount; ++i)
    {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if(r<0) {
            lime::error("failed to get device description");
            continue;
        }
        if (desc.idProduct != pid) continue;
        if (desc.idVendor != vid) continue;
        if(libusb_open(devs[i], &dev_handle) != 0) continue;

        std::string foundSerial;
        if (desc.iSerialNumber > 0)
        {
            char data[255];
            r = libusb_get_string_descriptor_ascii(dev_handle,desc.iSerialNumber,(unsigned char*)data, sizeof(data));
            if(r<0)
                lime::error("failed to get serial number");
            else
                foundSerial = std::string(data, size_t(r));
        }

        if (serial == foundSerial) break; //found it
        libusb_close(dev_handle);
        dev_handle = nullptr;
    }
    libusb_free_device_list(devs, 1);

    if(dev_handle == nullptr)
        return ReportError(ENODEV, "libusb_open failed");

    if(libusb_kernel_driver_active(dev_handle, 1) == 1)   //find out if kernel driver is attached
    {
        lime::debug("Kernel Driver Active");
        if(libusb_detach_kernel_driver(dev_handle, 1) == 0) //detach it
            lime::debug("Kernel Driver Detached!");
    }
    int r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if (r < 0)
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(libusb_error(r)));

    if ((r = libusb_claim_interface(dev_handle, 1))<0) //claim interface 1 of device
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(libusb_error(r)));
    lime::debug("Claimed Interface");

    if (libusb_reset_device(dev_handle)!=0)
        return ReportError(-1, "USB reset failed", libusb_strerror(libusb_error(r)));

    FT_FlushPipe(ctrlRdEp);  //clear ctrl ep rx buffer
    FT_SetStreamPipe(ctrlRdEp,64);
    FT_SetStreamPipe(ctrlWrEp,64);
    isConnected = true;
    return 0;
#endif
}

/**	@brief Closes communication to device.
*/
void ConnectionFT601::Close()
{
#ifndef __unix__
    FT_Close(mFTHandle);
#else
    if(dev_handle != 0)
    {
        FT_FlushPipe(streamRdEp);
        FT_FlushPipe(ctrlRdEp);
        libusb_release_interface(dev_handle, 1);
        libusb_close(dev_handle);
        dev_handle = 0;
    }
#endif
    isConnected = false;
}

/**	@brief Returns connection status
@return 1-connection open, 0-connection closed.
*/
bool ConnectionFT601::IsOpen()
{
    return isConnected;
}

#ifndef __unix__
int ConnectionFT601::ReinitPipe(unsigned char ep)
{
    FT_AbortPipe(mFTHandle, ep);
    FT_FlushPipe(mFTHandle, ep);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, ep, 64);
    return 0;
}
#endif

/**	@brief Sends given data buffer to chip through USB port.
@param buffer data buffer, must not be longer than 64 bytes.
@param length given buffer size.
@param timeout_ms timeout limit for operation in milliseconds
@return number of bytes sent.
*/
int ConnectionFT601::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = 0;
    if (IsOpen() == false)
        return 0;

#ifndef __unix__
    ULONG ulBytesWrite = 0;
    FT_STATUS ftStatus = FT_OK;
    OVERLAPPED	vOverlapped = { 0 };
    FT_InitializeOverlapped(mFTHandle, &vOverlapped);
    ftStatus = FT_WritePipe(mFTHandle, ctrlWrEp, (unsigned char*)buffer, length, &ulBytesWrite, &vOverlapped);
    if (ftStatus != FT_IO_PENDING)
    {
        FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
        ReinitPipe(ctrlWrEp);
        return -1;
    }

    DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
    if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT)
    {
        if (GetOverlappedResult(mFTHandle, &vOverlapped, &ulBytesWrite, FALSE) == FALSE)
        {
            ReinitPipe(ctrlWrEp);
            ulBytesWrite = -1;
        }
    }
    else
    {
        ReinitPipe(ctrlWrEp);
        ulBytesWrite = -1;
    }
    FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
    return ulBytesWrite;
#else
    unsigned char* wbuffer = new unsigned char[length];
    memcpy(wbuffer, buffer, length);
    int actual = 0;
    libusb_bulk_transfer(dev_handle, ctrlWrEp, wbuffer, length, &actual, timeout_ms);
    len = actual;
    delete[] wbuffer;
    return len;
#endif
}

/**	@brief Reads data coming from the chip through USB port.
@param buffer pointer to array where received data will be copied, array must be
big enough to fit received data.
@param length number of bytes to read from chip.
@param timeout_ms timeout limit for operation in milliseconds
@return number of bytes received.
*/

int ConnectionFT601::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = length;
    if(IsOpen() == false)
        return 0;
#ifndef __unix__
    ULONG ulBytesRead = 0;
    FT_STATUS ftStatus = FT_OK;
    OVERLAPPED	vOverlapped = { 0 };
    FT_InitializeOverlapped(mFTHandle, &vOverlapped);
    ftStatus = FT_ReadPipe(mFTHandle, ctrlRdEp, buffer, length, &ulBytesRead, &vOverlapped);
    if (ftStatus != FT_IO_PENDING)
    {
        FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
        ReinitPipe(ctrlRdEp);
        return -1;;
    }

    DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
    if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT)
    {
        if (GetOverlappedResult(mFTHandle, &vOverlapped, &ulBytesRead, FALSE)==FALSE)
        {
            ReinitPipe(ctrlRdEp);
            ulBytesRead = -1;
        }
    }
    else
    {
        ReinitPipe(ctrlRdEp);
        ulBytesRead = -1;
    }
    FT_ReleaseOverlapped(mFTHandle, &vOverlapped);
    return ulBytesRead;
#else
    int actual = 0;
    libusb_bulk_transfer(dev_handle, ctrlRdEp, buffer, len, &actual, timeout_ms);
    len = actual;
#endif
    return len;
}

#ifdef __unix__
/**	@brief Function for handling libusb callbacks
*/
static void callback_libusbtransfer(libusb_transfer *trans)
{
    ConnectionFT601::USBTransferContext *context = reinterpret_cast<ConnectionFT601::USBTransferContext*>(trans->user_data);
    std::unique_lock<std::mutex> lck(context->transferLock);
    switch(trans->status)
    {
        case LIBUSB_TRANSFER_CANCELLED:
            context->bytesXfered = trans->actual_length;
            context->done.store(true);
            break;
        case LIBUSB_TRANSFER_COMPLETED:
            context->bytesXfered = trans->actual_length;
            context->done.store(true);
            break;
        case LIBUSB_TRANSFER_ERROR:
            lime::error("TRANSFER ERROR");
            context->bytesXfered = trans->actual_length;
            context->done.store(true);
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
            lime::error("USB transfer timed out");
            context->done.store(true);
            break;
        case LIBUSB_TRANSFER_OVERFLOW:
            lime::error("transfer overflow\n");
            break;
        case LIBUSB_TRANSFER_STALL:
            lime::error("transfer stalled");
            break;
        case LIBUSB_TRANSFER_NO_DEVICE:
            lime::error("transfer no device");
            break;
    }
    lck.unlock();
    context->cv.notify_one();
}
#endif

int ConnectionFT601::GetBuffersCount() const
{
    return USB_MAX_CONTEXTS;
}

int ConnectionFT601::CheckStreamSize(int size)const
{
    return size;
}

/**
@brief Starts asynchronous data reading from board
@param *buffer buffer where to store received data
@param length number of bytes to read
@return handle of transfer context
*/
int ConnectionFT601::BeginDataReading(char *buffer, uint32_t length, int ep)
{
    int i = 0;
    bool contextFound = false;
    //find not used context
    for(i = 0; i<USB_MAX_CONTEXTS; i++)
    {
        if(!contexts[i].used)
        {
            contextFound = true;
            break;
        }
    }
    if(!contextFound)
    {
        lime::error("No contexts left for reading data");
        return -1;
    }
    contexts[i].used = true;

#ifndef __unix__
    FT_InitializeOverlapped(mFTHandle, &contexts[i].inOvLap);
	ULONG ulActual;
    FT_STATUS ftStatus = FT_OK;
    ftStatus = FT_ReadPipe(mFTHandle, streamRdEp, (unsigned char*)buffer, length, &ulActual, &contexts[i].inOvLap);
    if (ftStatus != FT_IO_PENDING)
    {
        lime::error("ERROR BEGIN DATA READING %d", ftStatus);
        contexts[i].used = false;
        return -1;
    }
#else
    libusb_transfer *tr = contexts[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, streamRdEp, (unsigned char*)buffer, length, callback_libusbtransfer, &contexts[i], 0);
    contexts[i].done = false;
    contexts[i].bytesXfered = 0;
    int status = libusb_submit_transfer(tr);
    if(status != 0)
    {
        lime::error("ERROR BEGIN DATA READING %s", libusb_error_name(status));
        contexts[i].used = false;
        return -1;
    }
#endif
    return i;
}

/**
@brief Waits for asynchronous data reception
@param contextHandle handle of which context data to wait
@param timeout_ms number of miliseconds to wait
@return true - wait finished, false - still waiting for transfer to complete
*/
bool ConnectionFT601::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
#ifndef __unix__
        DWORD dwRet = WaitForSingleObject(contexts[contextHandle].inOvLap.hEvent, timeout_ms);
            if (dwRet == WAIT_OBJECT_0)
                return 1;
#else
        //blocking not to waste CPU
        std::unique_lock<std::mutex> lck(contexts[contextHandle].transferLock);
        return contexts[contextHandle].cv.wait_for(lck, chrono::milliseconds(timeout_ms), [&](){return contexts[contextHandle].done.load();});
#endif
    }
    return true;  //there is nothing to wait for (signal wait finished)
}

/**
@brief Finishes asynchronous data reading from board
@param buffer array where to store received data
@param length number of bytes to read
@param contextHandle handle of which context to finish
@return false failure, true number of bytes received
*/
int ConnectionFT601::FinishDataReading(char *buffer, uint32_t length, int contextHandle)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
#ifndef __unix__
	ULONG ulActualBytesTransferred;
        FT_STATUS ftStatus = FT_OK;

        ftStatus = FT_GetOverlappedResult(mFTHandle, &contexts[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);
        if (ftStatus != FT_OK)
            length = 0;
        else
            length = ulActualBytesTransferred;
        FT_ReleaseOverlapped(mFTHandle, &contexts[contextHandle].inOvLap);
        contexts[contextHandle].used = false;
        return length;
#else
        length = contexts[contextHandle].bytesXfered;
        contexts[contextHandle].used = false;
        return length;
#endif
    }
    return 0;
}

/**
@brief Aborts reading operations
*/
void ConnectionFT601::AbortReading(int ep)
{
#ifndef __unix__
    FT_AbortPipe(mFTHandle, streamRdEp);
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contexts[i].used == true)
        {
            FT_ReleaseOverlapped(mFTHandle, &contexts[i].inOvLap);
            contexts[i].used = false;
        }
    }
    FT_FlushPipe(mFTHandle, streamRdEp);
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamRdEp, sizeof(FPGA_DataPacket));
#else

    for(int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contexts[i].used)
	{
            if (WaitForReading(i, 100))
                FinishDataReading(nullptr, 0, i);
            else
            	libusb_cancel_transfer(contexts[i].transfer);
	}
    }
    for(int i=0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contexts[i].used)
        {
            WaitForReading(i, 100);
            FinishDataReading(nullptr, 0, i);
        }
    }
#endif
}

/**
@brief Starts asynchronous data Sending to board
@param *buffer buffer to send
@param length number of bytes to send
@return handle of transfer context
*/
int ConnectionFT601::BeginDataSending(const char *buffer, uint32_t length, int ep)
{
    int i = 0;
    //find not used context
    bool contextFound = false;
    for(i = 0; i<USB_MAX_CONTEXTS; i++)
    {
        if(!contextsToSend[i].used)
        {
            contextFound = true;
            break;
        }
    }
    if(!contextFound)
        return -1;
    contextsToSend[i].used = true;

#ifndef __unix__
	FT_STATUS ftStatus = FT_OK;
	ULONG ulActualBytesSend;
    FT_InitializeOverlapped(mFTHandle, &contextsToSend[i].inOvLap);
	ftStatus = FT_WritePipe(mFTHandle, streamWrEp, (unsigned char*)buffer, length, &ulActualBytesSend, &contextsToSend[i].inOvLap);
	if (ftStatus != FT_IO_PENDING)
    {
        lime::error("ERROR BEGIN DATA SENDING %d", ftStatus);
        contexts[i].used = false;
        return -1;
    }
#else
    libusb_transfer *tr = contextsToSend[i].transfer;
    contextsToSend[i].done = false;
    contextsToSend[i].bytesXfered = 0;
    libusb_fill_bulk_transfer(tr, dev_handle, streamWrEp, (unsigned char*)buffer, length, callback_libusbtransfer, &contextsToSend[i], 0);
    int status = libusb_submit_transfer(tr);
    if(status != 0)
    {
        lime::error("ERROR BEGIN DATA SENDING %s", libusb_error_name(status));
        contextsToSend[i].used = false;
        return -1;
    }
#endif
    return i;
}

/**
@brief Waits for asynchronous data sending
@param contextHandle handle of which context data to wait
@param timeout_ms number of miliseconds to wait
@return true - wait finished, false - still waiting for transfer to complete
*/
bool ConnectionFT601::WaitForSending(int contextHandle, unsigned int timeout_ms)
{
    if(contextHandle >= 0 && contextsToSend[contextHandle].used == true)
    {
#ifndef __unix__
        DWORD dwRet = WaitForSingleObject(contextsToSend[contextHandle].inOvLap.hEvent, timeout_ms);
            if (dwRet == WAIT_OBJECT_0)
                return 1;
#else
        //blocking not to waste CPU
        std::unique_lock<std::mutex> lck(contextsToSend[contextHandle].transferLock);
        return contextsToSend[contextHandle].cv.wait_for(lck, chrono::milliseconds(timeout_ms), [&](){return contextsToSend[contextHandle].done.load();});
#endif
    }
    return true; //there is nothing to wait for (signal wait finished)
}

/**
@brief Finishes asynchronous data sending to board
@param buffer array where to store received data
@param length number of bytes to read
@param contextHandle handle of which context to finish
@return false failure, true number of bytes sent
*/
int ConnectionFT601::FinishDataSending(const char *buffer, uint32_t length, int contextHandle)
{
    if(contextHandle >= 0 && contextsToSend[contextHandle].used == true)
    {
#ifndef __unix__
        ULONG ulActualBytesTransferred ;
        FT_STATUS ftStatus = FT_OK;
        ftStatus = FT_GetOverlappedResult(mFTHandle, &contextsToSend[contextHandle].inOvLap, &ulActualBytesTransferred, FALSE);
        if (ftStatus != FT_OK)
            length = 0;
        else
        length = ulActualBytesTransferred;
        FT_ReleaseOverlapped(mFTHandle, &contextsToSend[contextHandle].inOvLap);
	    contextsToSend[contextHandle].used = false;
	    return length;
#else
        length = contextsToSend[contextHandle].bytesXfered;
        contextsToSend[contextHandle].used = false;
        return length;
#endif
    }
    else
        return 0;
}

/**
@brief Aborts sending operations
*/
void ConnectionFT601::AbortSending(int ep)
{
#ifndef __unix__
    FT_AbortPipe(mFTHandle, streamWrEp);
    for (int i = 0; i < USB_MAX_CONTEXTS; ++i)
    {
        if (contextsToSend[i].used == true)
        {
            FT_ReleaseOverlapped(mFTHandle, &contextsToSend[i].inOvLap);
            contextsToSend[i].used = false;
        }
    }
    FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamWrEp, sizeof(FPGA_DataPacket));
#else
    for(int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contextsToSend[i].used)
        {
            if (WaitForSending(i, 100))
                FinishDataSending(nullptr, 0, i);
            else
                libusb_cancel_transfer(contextsToSend[i].transfer);
        }
    }
    for (int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contextsToSend[i].used)
        {
            WaitForSending(i, 100);
            FinishDataSending(nullptr, 0, i);
        }
    }
#endif
}

int ConnectionFT601::ResetStreamBuffers()
{
#ifndef __unix__
    if (FT_AbortPipe(mFTHandle, streamRdEp)!=FT_OK)
        return -1;
    if (FT_AbortPipe(mFTHandle, streamWrEp)!=FT_OK)
        return -1;
    if (FT_FlushPipe(mFTHandle, streamRdEp)!=FT_OK)
        return -1;
    if (FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamRdEp, sizeof(FPGA_DataPacket)) != 0)
        return -1;
    if (FT_SetStreamPipe(mFTHandle, FALSE, FALSE, streamWrEp, sizeof(FPGA_DataPacket)) != 0)
        return -1;
#else
    if (FT_FlushPipe(streamWrEp)!=0)
        return -1;
    if (FT_FlushPipe(streamRdEp)!=0)
        return -1;
    if (FT_SetStreamPipe(streamWrEp,sizeof(FPGA_DataPacket))!=0)
        return -1;
    if (FT_SetStreamPipe(streamRdEp,sizeof(FPGA_DataPacket))!=0)
        return -1;
#endif
    return 0;
}

int ConnectionFT601::ProgramWrite(const char *data_src, size_t length, int prog_mode, int device, ProgrammingCallback callback)
{
    int ret;

    if (device != LMS64CProtocol::FPGA)
    {
        lime::error("Unsupported programming target");
        return -1;
    }
    if (prog_mode == 0)
    {
        lime::error("Programming to RAM is not supported");
        return -1;
    }

    if (prog_mode == 2)
        return LMS64CProtocol::ProgramWrite(data_src, length, prog_mode, device, callback);

    FPGAinfo fpgainfo = GetFPGAInfo();

    if (fpgainfo.hwVersion < 3)
    {
        // LimeSDR-Mini v1.X
        if (fpgainfo.gatewareVersion != 0)
        {
            LMS64CProtocol::ProgramWrite(nullptr, 0, 2, 2, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
        const int sizeUFM = 0x8000;
        const int sizeCFM0 = 0x42000;
        const int startUFM = 0x1000;
        const int startCFM0 = 0x4B000;

        if (length != startCFM0 + sizeCFM0)
        {
            lime::error("Invalid image file");
            return -1;
        }
        std::vector<char> buffer(sizeUFM + sizeCFM0);
        memcpy(buffer.data(), data_src + startUFM, sizeUFM);
        memcpy(buffer.data() + sizeUFM, data_src + startCFM0, sizeCFM0);

        ret = LMS64CProtocol::ProgramWrite(buffer.data(), buffer.size(), prog_mode, device, callback);
        LMS64CProtocol::ProgramWrite(nullptr, 0, 2, 2, nullptr);
    }
    else
    {
        // LimeSDR-Mini v2.X
        ret = LMS64CProtocol::ProgramWrite(data_src, length, prog_mode, device, callback);
        LMS64CProtocol::ProgramWrite(nullptr, 0, 2, 2, nullptr);
    }

    return ret;
}

DeviceInfo ConnectionFT601::GetDeviceInfo(void)
{
    DeviceInfo info = LMS64CProtocol::GetDeviceInfo();
    info.boardSerialNumber = mSerial;
    return info;
}

int ConnectionFT601::GPIOWrite(const uint8_t *buffer, size_t len)
{
    if ((!buffer)||(len==0))
        return -1;
    const uint32_t addr = 0xC6;
    const uint32_t value = (len == 1) ? buffer[0] : buffer[0] | (buffer[1]<<8);
    return WriteRegisters(&addr, &value, 1);
}

int ConnectionFT601::GPIORead(uint8_t *buffer, size_t len)
{
    if ((!buffer)||(len==0))
        return -1;
    const uint32_t addr = 0xC2;
    uint32_t value;
    int ret = ReadRegisters(&addr, &value, 1);
    buffer[0] = value;
    if (len > 1)
        buffer[1] = (value >> 8);
    return ret;
}

int ConnectionFT601::GPIODirWrite(const uint8_t *buffer, size_t len )
{
    if ((!buffer)||(len==0))
        return -1;
    const uint32_t addr = 0xC4;
    const uint32_t value = (len == 1) ? buffer[0] : buffer[0] | (buffer[1]<<8);
    return WriteRegisters(&addr, &value, 1);
}

int ConnectionFT601::GPIODirRead(uint8_t *buffer, size_t len)
{
    if ((!buffer)||(len==0))
        return -1;
    const uint32_t addr = 0xC4;
    uint32_t value;
    int ret = ReadRegisters(&addr, &value, 1);
    buffer[0] = value;
    if (len > 1)
        buffer[1] = (value >> 8);
    return ret;
}
