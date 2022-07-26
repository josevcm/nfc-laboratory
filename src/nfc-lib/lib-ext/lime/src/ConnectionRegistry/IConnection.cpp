/**
    @file   IConnection.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  Implementation of IConnection interface
*/

#include "IConnection.h"
#include "Logger.h"
#include "LMSBoards.h"

using namespace lime;

IConnection::IConnection(void)
{
    callback_logData = nullptr;;
}

IConnection::~IConnection(void)
{
    return;
}

const ConnectionHandle &IConnection::GetHandle(void) const
{
    return _handle;
}

bool IConnection::IsOpen(void)
{
    return false;
}

DeviceInfo IConnection::GetDeviceInfo(void)
{
    DeviceInfo info;
    //initialize to UNKNOWN board type
    //causes lms7_device::CreateDevice() to use LMS7_Generic
    info.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
    info.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    return info;
}

/***********************************************************************
 * Serial API
 **********************************************************************/

int IConnection::TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    return ReportError("TransactSPI not supported");
}

int IConnection::WriteI2C(const int addr, const std::string &data)
{
    return ReportError("WriteI2C not supported");
}

int IConnection::ReadI2C(const int addr, const size_t numBytes, std::string &data)
{
    return ReportError("ReadI2C not supported");
}

/***********************************************************************
 * LMS7002M Driver callbacks
 **********************************************************************/

int IConnection::DeviceReset(int ind)
{
    return ReportError("DeviceReset not supported");
}

/***********************************************************************
 * Stream API
 **********************************************************************/

int IConnection::ReceiveData(char* buffer, int length, int epIndex, int timeout)
{
    return 0;
}
int IConnection::SendData(const char* buffer, int length, int epIndex, int timeout)
{
    return 0;
}

int IConnection::BeginDataSending(const char* buffer, uint32_t length, int ep)
{
    return 0;
}
bool IConnection::WaitForSending(int contextHandle, uint32_t timeout_ms)
{
    return true;
}
int IConnection::FinishDataSending(const char* buffer, uint32_t length, int contextHandle)
{
    return 0;
}

int IConnection::BeginDataReading(char* buffer, uint32_t length, int ep)
{
    return 0;
}
bool IConnection::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    return true;
}
int IConnection::FinishDataReading(char* buffer, uint32_t length, int contextHandle)
{
    return 0;
}

/** @brief Sets callback function which gets called each time data is sent or received
*/
void IConnection::SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback)
{
    callback_logData = callback;
}

int IConnection::GetBuffersCount()const
{
 return 0;
}

int IConnection::CheckStreamSize(int size)const
{
    return 0;
}

int IConnection::ResetStreamBuffers()
{
    return 0;
}
/***********************************************************************
 * Programming API
 **********************************************************************/

int IConnection::ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int index, ProgrammingCallback callback)
{
    ReportError(ENOTSUP, "ProgramWrite not supported");
    return -1;
}

int IConnection::ProgramMCU(const uint8_t *buffer, const size_t length, const MCU_PROG_MODE mode, ProgrammingCallback callback)
{
    ReportError(ENOTSUP, "ProgramMCU not supported");
    return -1;
}

int IConnection::ProgramUpdate(const bool download, const bool force, ProgrammingCallback callback)
{
    ReportError(ENOTSUP, "ProgramUpdate not supported");
    return -1;
}

/***********************************************************************
 * GPIO API
 **********************************************************************/

int IConnection::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIOWrite not supported");
    return -1;
}

int IConnection::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIORead not supported");
    return -1;
}

int IConnection::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIOWrite not supported");
    return -1;
}

int IConnection::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    ReportError(ENOTSUP, "GPIORead not supported");
    return -1;
}

/***********************************************************************
 * Register API
 **********************************************************************/

int IConnection::WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size)
{
    ReportError(ENOTSUP, "WriteRegisters not supported");
    return -1;
}

int IConnection::ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size)
{
    ReportError(ENOTSUP, "ReadRegisters not supported");
    return -1;
}

int IConnection::WriteRegister(const uint32_t addr, const uint32_t data)
{
    return this->WriteRegisters(&addr, &data, 1);
}

/***********************************************************************
 * Aribtrary settings API
 **********************************************************************/

int IConnection::CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units)
{
    ReportError(ENOTSUP, "CustomParameterWrite not supported");
    return -1;
}

int IConnection::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    ReportError(ENOTSUP, "CustomParameterRead not supported");
    return -1;
}
