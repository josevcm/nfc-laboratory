/**
    @file ConnectionSTREAM.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionXillybus.h"
#ifndef __unix__
#include "Windows.h"
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include "Si5351C.h"
#include <FPGA_common.h>
#include <LMS7002M.h>
#include <ciso646>
#include "Logger.h"

#include <thread>
#include <chrono>

using namespace std;

using namespace lime;

const std::vector<ConnectionXillybus::EPConfig> ConnectionXillybus::deviceConfigs = {
#ifndef __unix__
    {
        "LimeSDR-PCIe",
        "\\\\.\\xillybus_read_8",
        "\\\\.\\xillybus_write_8",
        {"\\\\.\\xillybus_read_32", "\\\\.\\xillybus_read_32", "\\\\.\\xillybus_read_32"},
        {"\\\\.\\xillybus_write_32", "\\\\.\\xillybus_write_32", "\\\\.\\xillybus_write_32"}
    },
    {
        "LimeSDR-QPCIe",
        "\\\\.\\xillybus_control0_read_32",
        "\\\\.\\xillybus_control0_write_32",
        {"\\\\.\\xillybus_stream0_read_32", "\\\\.\\xillybus_stream1_read_32", "\\\\.\\xillybus_stream2_read_32"},
        {"\\\\.\\xillybus_stream0_write_32", "\\\\.\\xillybus_stream1_write_32", "\\\\.\\xillybus_stream2_write_32"}
    },
{
        "LimeSDR-PCIe (0)",
        "\\\\.\\xillybus_control0_read_8",
        "\\\\.\\xillybus_control0_write_8",
        { "\\\\.\\xillybus_stream0_read_32", "", "" },
        { "\\\\.\\xillybus_stream0_write_32", "", "" }
},
{
        "LimeSDR-PCIe (1)",
        "\\\\.\\xillybus_control1_read_8",
        "\\\\.\\xillybus_control1_write_8",
        { "\\\\.\\xillybus_stream1_read_32", "", "" },
        { "\\\\.\\xillybus_stream1_write_32", "", "" }
}
#else
    {
        "LimeSDR-PCIe",
        "/dev/xillybus_read_8",
        "/dev/xillybus_write_8",
        {"/dev/xillybus_read_32", "", ""},
        {"/dev/xillybus_write_32", "", ""}
    },
    {
        "LimeSDR-QPCIe",
        "/dev/xillybus_control0_read_32",
        "/dev/xillybus_control0_write_32",
        {"/dev/xillybus_stream0_read_32", "/dev/xillybus_stream1_read_32", "/dev/xillybus_stream2_read_32"},
        {"/dev/xillybus_stream0_write_32", "/dev/xillybus_stream1_write_32", "/dev/xillybus_stream2_write_32"}
    },
    {
        "LimeSDR-PCIe (0)",
        "/dev/xillybus_control0_read_8",
        "/dev/xillybus_control0_write_8",
        { "/dev/xillybus_stream0_read_32", "", "" },
        { "/dev/xillybus_stream0_write_32", "", "" }
    },
    {
        "LimeSDR-PCIe (1)",
        "/dev/xillybus_control1_read_8",
        "/dev/xillybus_control1_write_8",
        { "/dev/xillybus_stream1_read_32", "", "" },
        { "/dev/xillybus_stream1_write_32", "", "" }
    }
#endif
};

/** @brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionXillybus::ConnectionXillybus(const unsigned index)
{
    m_hardwareName = "";
#ifndef __unix__
    hWrite = INVALID_HANDLE_VALUE;
    hRead = INVALID_HANDLE_VALUE;
    for (int i = 0; i < MAX_EP_CNT; i++)
        hWriteStream[i] = hReadStream[i] = INVALID_HANDLE_VALUE;
#else
    hWrite = -1;
    hRead = -1;
    for (int i = 0; i < MAX_EP_CNT; i++)
        hWriteStream[i] = hReadStream[i] = -1;
#endif
    Open(index);
    isConnected = true;

    std::shared_ptr<Si5351C> si5351module(new Si5351C());
    si5351module->Initialize(this);
    si5351module->SetPLL(0, 25000000, 0);
    si5351module->SetPLL(1, 25000000, 0);
    si5351module->SetClock(0, 27000000, true, false);
    si5351module->SetClock(1, 27000000, true, false);
    for (int i = 2; i < 8; ++i)
        si5351module->SetClock(i, 27000000, false, false);
    Si5351C::Status status = si5351module->ConfigureClocks();
    if (status != Si5351C::SUCCESS)
    {
        lime::warning("Failed to configure Si5351C");
        return;
    }
    status = si5351module->UploadConfiguration();
    if (status != Si5351C::SUCCESS)
        lime::warning("Failed to configure Si5351C");
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); //some settle time
}

/** @brief Closes connection to chip and deallocates used memory.
*/
ConnectionXillybus::~ConnectionXillybus()
{
    Close();
}

/** @brief Tries to open connected USB device and find communication endpoints.
    @return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionXillybus::Open(const unsigned index)
{
    Close();
    writeCtrlPort = deviceConfigs[index].ctrlWrite;
    readCtrlPort = deviceConfigs[index].ctrlRead;
#ifndef __unix__
    hWrite = CreateFileA(writeCtrlPort.c_str(), GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
    hRead = CreateFileA(readCtrlPort.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
    if (hWrite == INVALID_HANDLE_VALUE || hRead == INVALID_HANDLE_VALUE)
    {
        ReportError("Unable to access control port");
        CloseHandle(hWrite);
        CloseHandle(hRead);
        hWrite = INVALID_HANDLE_VALUE;
        hRead = INVALID_HANDLE_VALUE;
        return -1;
    }
#endif
    isConnected = true;
    for (int i = 0; i < MAX_EP_CNT; i++)
    {
        readStreamPort[i] = deviceConfigs[index].streamRead[i];
        writeStreamPort[i] = deviceConfigs[index].streamWrite[i];
    }
    return 0;
}

/** @brief Closes communication to device.
*/
void ConnectionXillybus::Close()
{
    isConnected = false;
#ifndef __unix__
    if (hWrite != INVALID_HANDLE_VALUE)
        CloseHandle(hWrite);
    hWrite = INVALID_HANDLE_VALUE;
    if (hRead != INVALID_HANDLE_VALUE)
        CloseHandle(hRead);
    hRead = INVALID_HANDLE_VALUE;
    for (int i = 0; i < MAX_EP_CNT; i++)
    {
        if (hWriteStream[i] != INVALID_HANDLE_VALUE)
            CloseHandle(hWriteStream[i]);
        hWriteStream[i] = INVALID_HANDLE_VALUE;
        if (hReadStream[i] != INVALID_HANDLE_VALUE)
            CloseHandle(hReadStream[i]);
        hReadStream[i] = INVALID_HANDLE_VALUE;
    }
#else
    CloseControl();
    for (int i = 0; i < MAX_EP_CNT; i++)
    {
        if( hWriteStream[i] >= 0)
            close(hWriteStream[i]);
        hWriteStream[i] = -1;
        if( hReadStream[i] >= 0)
            close(hReadStream[i]);
        hReadStream[i] = -1;
    }
#endif
}

/** @brief Returns connection status
    @return 1-connection open, 0-connection closed.
*/
bool ConnectionXillybus::IsOpen()
{
    return isConnected;
}

#ifdef __unix__
int ConnectionXillybus::OpenControl()
{
    int timeout_cnt = 100;
    usleep(200);  //avoids random bad packet(s) that was occurring when performing lots of consecutive transfers ~1000+
    while (--timeout_cnt)
    {
       if ((hWrite = open(writeCtrlPort.c_str(), O_WRONLY | O_NOCTTY | O_NONBLOCK))!=-1)
           break;
       usleep(1000);
    }
    while (timeout_cnt--)
    {
       if ((hRead = open(readCtrlPort.c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK))!=-1)
           break;
       usleep(1000);
    }

    if (hWrite == -1 || hRead ==-1)
        return ReportError(errno);
    return 0;
}

void ConnectionXillybus::CloseControl()
{
    close(hWrite);
    close(hRead);
    hWrite = -1;
    hRead = -1;
}

int ConnectionXillybus::TransferPacket(GenericPacket &pkt)
{
    int status;
    std::lock_guard<std::mutex> lock(mTransferLock);
    if (OpenControl() == 0)
        status = LMS64CProtocol::TransferPacket(pkt);
    else
        status = -1;
    CloseControl();
    return status;
}
int ConnectionXillybus::ProgramWrite(const char *data_src, const size_t length, const int prog_mode, const int device, ProgrammingCallback callback)
{
    int status;
    std::lock_guard<std::mutex> lock(mTransferLock);
    if (OpenControl() == 0)
        status = LMS64CProtocol::ProgramWrite(data_src, length, prog_mode, device, callback);
    else
        status = -1;
    CloseControl();
    return status;
}
#endif
/** @brief Sends given data buffer to chip through USB port.
    @param buffer data buffer, must not be longer than 64 bytes.
    @param length given buffer size.
    @param timeout_ms timeout limit for operation in milliseconds
    @return number of bytes sent.
*/
int ConnectionXillybus::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
#ifndef __unix__
    if (hWrite == INVALID_HANDLE_VALUE)
        return -1;
#endif
    int totalBytesWritten = 0;
    int bytesToWrite = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
#ifndef __unix__
        DWORD bytesSent = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        WriteFile(hWrite, buffer + totalBytesWritten, bytesToWrite, &bytesSent, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesWritten;
        }
        std::this_thread::yield();
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, 500);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hWrite, &vOverlapped, &bytesSent, FALSE) == FALSE)
            {
                bytesSent = 0;
            }
        }
        else
        {
            CancelIo(hWrite);
            bytesSent = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesSent;
        if ((bytesSent  = write(hWrite, buffer+ totalBytesWritten, bytesToWrite))<0)
        {
            if(errno == EINTR || errno == EAGAIN)
                 continue;
            ReportError(errno);
            return totalBytesWritten;
        }
#endif
        totalBytesWritten += bytesSent;
        if (totalBytesWritten < length)
            bytesToWrite -= bytesSent;
        else
            break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
#ifdef __unix__
    //Flush data to FPGA
    while (write(hWrite, NULL, 0) < 0)
    {
        if (errno == EINTR)
            continue;
        ReportError(errno);
        break;
    }
#else
    if (totalBytesWritten != length)
    {
        CloseHandle(hWrite);
        hWrite = CreateFileA(writeCtrlPort.c_str(), GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
        if (hWrite == INVALID_HANDLE_VALUE)
        {
            CloseHandle(hRead);
            hWrite = INVALID_HANDLE_VALUE;
        }
     }
#endif
    return totalBytesWritten;
}

/** @brief Reads data coming from the chip through USB port.
    @param buffer pointer to array where received data will be copied, array must be
    big enough to fit received data.
    @param length number of bytes to read from chip.
    @param timeout_ms timeout limit for operation in milliseconds
    @return number of bytes received.
*/
int ConnectionXillybus::Read(unsigned char *buffer, const int length, int timeout_ms)
{
#ifndef __unix__
    if (hRead == INVALID_HANDLE_VALUE)
        return -1;
#endif
    int totalBytesReaded = 0;
    int bytesToRead = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
 #ifndef __unix__
        DWORD bytesReceived = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        ReadFile(hRead, buffer + totalBytesReaded, bytesToRead, &bytesReceived, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesReaded;
        }
        std::this_thread::yield();
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, 1000);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hRead, &vOverlapped, &bytesReceived, TRUE) == FALSE)
            {
                bytesReceived = 0;
            }
        }
        else
        {
            CancelIo(hRead);
            bytesReceived = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesReceived;
        if ((bytesReceived = read(hRead, buffer+ totalBytesReaded, bytesToRead))<0)
        {
           if(errno == EINTR || errno == EAGAIN)
               continue;
           ReportError(errno);
           return totalBytesReaded;
        }
#endif
        totalBytesReaded += bytesReceived;
        if (totalBytesReaded < length)
           bytesToRead -= bytesReceived;
        else
           break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);

#ifndef __unix__
    if (totalBytesReaded != length)
    {
        CloseHandle(hRead);
        hRead = CreateFileA(readCtrlPort.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
        if (hRead == INVALID_HANDLE_VALUE)
        {
            CloseHandle(hRead);
            hRead = INVALID_HANDLE_VALUE;
        }
    }
#endif
    return totalBytesReaded;
}

int ConnectionXillybus::GetBuffersCount() const
{
    return 1;
}

int ConnectionXillybus::CheckStreamSize(int size) const
{
    return size < 4 ? 4 : size;
}

/**
    @brief Reads data from board
    @param buffer array where to store received data
    @param length number of bytes to read
    @param timeout read timeout in milliseconds
    @return number of bytes received
*/
int ConnectionXillybus::ReceiveData(char *buffer, int length, int epIndex, int timeout_ms)
{
#ifndef __unix__
    if (hReadStream[epIndex] == INVALID_HANDLE_VALUE)
    {
        hReadStream[epIndex] = CreateFileA(readStreamPort[epIndex].c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
        if (hReadStream[epIndex] == INVALID_HANDLE_VALUE)
        {
            ReportError("Failed to open Xillybus");
            return -1;
        }
    }
#else
    if (hReadStream[epIndex] == -1)
    {
       if (( hReadStream[epIndex] = open(readStreamPort[epIndex].c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK))==-1)
       {
            ReportError(errno);
            return 0;
       }
    }
#endif

    int totalBytesReaded = 0;
    int bytesToRead = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
 #ifndef __unix__
        DWORD bytesReceived = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        ReadFile(hReadStream[epIndex], buffer + totalBytesReaded, bytesToRead, &bytesReceived, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesReaded;
        }
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hReadStream[epIndex], &vOverlapped, &bytesReceived, TRUE) == FALSE)
            {
                bytesReceived = 0;
            }
        }
        else
        {
            CancelIo(hReadStream[epIndex]);
            bytesReceived = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesReceived;
        if ((bytesReceived = read(hReadStream[epIndex], buffer+ totalBytesReaded, bytesToRead))<0)
        {
            if(errno == EINTR || errno == EAGAIN)
                continue;
            ReportError(errno);
            return totalBytesReaded;
        }
#endif
        totalBytesReaded += bytesReceived;
        if (totalBytesReaded < length)
            bytesToRead -= bytesReceived;
        else
            break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);

    return totalBytesReaded;
}

/**
    @brief Aborts reading operations
*/
void ConnectionXillybus::AbortReading(int epIndex)
{
#ifndef __unix__
    if (hReadStream[epIndex] != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hReadStream[epIndex]);
	hReadStream[epIndex] = INVALID_HANDLE_VALUE;
    }
#else
    if (hReadStream[epIndex] >= 0)
    {
        close(hReadStream[epIndex]);
        hReadStream[epIndex] =-1;
    }
#endif
}

/**
    @brief  sends data to board
    @param *buffer buffer to send
    @param length number of bytes to send
    @param timeout data write timeout in milliseconds
    @return number of bytes sent
*/
int ConnectionXillybus::SendData(const char *buffer, int length, int epIndex, int timeout_ms)
{
#ifndef __unix__
    if (hWriteStream[epIndex] == INVALID_HANDLE_VALUE)
    {
        hWriteStream[epIndex] = CreateFileA(writeStreamPort[epIndex].c_str(), GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
        if (hWriteStream[epIndex] == INVALID_HANDLE_VALUE)
        {
            ReportError("Failed to open Xillybus");
            return -1;
        }
    }
#else
    if (hWriteStream[epIndex] == -1)
    {
       if ((hWriteStream[epIndex] = open(writeStreamPort[epIndex].c_str(), O_WRONLY | O_NOCTTY | O_NONBLOCK))==-1)
       {
            ReportError(errno);
            return 0;
       }
    }
#endif
    int totalBytesWritten = 0;
    int bytesToWrite = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
#ifndef __unix__
        DWORD bytesSent = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        WriteFile(hWriteStream[epIndex], buffer + totalBytesWritten, bytesToWrite, &bytesSent, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesWritten;
        }
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hWriteStream[epIndex], &vOverlapped, &bytesSent, TRUE) == FALSE)
            {
                bytesSent = 0;
            }
        }
        else
        {
            CancelIo(hWriteStream[epIndex]);
            bytesSent = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesSent;
        if ((bytesSent  = write(hWriteStream[epIndex], buffer+ totalBytesWritten, bytesToWrite))<0)
        {
            if(errno == EINTR || errno == EAGAIN)
                continue;
            ReportError(errno);
            return totalBytesWritten;
        }
#endif
        totalBytesWritten += bytesSent;
        if (totalBytesWritten < length)
            bytesToWrite -= bytesSent;
        else
            break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
    //Flush data to FPGA
#ifdef __unix__
    while (write(hWriteStream[epIndex], NULL, 0)<0)
    {
        if (errno == EINTR)
            continue;
        ReportError(errno);
        break;
    }
#endif
    return totalBytesWritten;
}

/**
	@brief Aborts sending operations
*/
void ConnectionXillybus::AbortSending(int epIndex)
{
#ifndef __unix__
    if (hWriteStream[epIndex] != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hWriteStream[epIndex]);
        hWriteStream[epIndex] = INVALID_HANDLE_VALUE;
    }
#else
    if (hWriteStream[epIndex] >= 0)
    {
        close(hWriteStream[epIndex]);
        hWriteStream[epIndex] = -1;
    }
#endif
}

int ConnectionXillybus::BeginDataReading(char* buffer, uint32_t length, int ep)
{
    return ep;
}
bool ConnectionXillybus::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    return true;
}
int ConnectionXillybus::FinishDataReading(char* buffer, uint32_t length, int contextHandle)
{
    return ReceiveData(buffer, length, contextHandle, 3000);
}

int ConnectionXillybus::BeginDataSending(const char* buffer, uint32_t length, int ep)
{
    return SendData(buffer, length,  ep, 3000);
}
bool ConnectionXillybus::WaitForSending(int contextHandle, uint32_t timeout_ms)
{
    return true;
}
int ConnectionXillybus::FinishDataSending(const char* buffer, uint32_t length, int contextHandle)
{
    return contextHandle;
}
