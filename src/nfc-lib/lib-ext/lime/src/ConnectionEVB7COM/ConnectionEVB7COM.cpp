/**
    @file ConnectionEVB7COM.cpp
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#include "ConnectionEVB7COM.h"
#include "Logger.h"

#include "string.h"
#ifdef __unix__
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "Logger.h"
#endif // LINUX

static const int COM_RETRY_INTERVAL = 20; //ms
static const int COM_TOTAL_TIMEOUT = 300; //ms
static const int COM_BUFFER_LENGTH = 1024; //max buffer size for data

using namespace lime;

ConnectionEVB7COM::ConnectionEVB7COM(const char *comName, int baudrate)
{
#ifndef __unix__
    hComm = INVALID_HANDLE_VALUE;
#else
    hComm = -1;
#endif

    if (this->Open(comName, baudrate) != 0)
    {
        this->Close();
        lime::error("Failed to open COM port");
    }
}

ConnectionEVB7COM::~ConnectionEVB7COM(void)
{
    this->Close();
}

void ConnectionEVB7COM::Close(void)
{
#ifndef __unix__
    if (hComm != INVALID_HANDLE_VALUE)
    {
        SetCommTimeouts(hComm, &m_ctmoOld);
        CloseHandle(hComm);
    }
    hComm = INVALID_HANDLE_VALUE;
#else
    if( hComm >= 0)
    {
        close(hComm);
    }
    hComm = -1;
#endif
}

bool ConnectionEVB7COM::IsOpen(void)
{
#ifndef __unix__
    if (hComm != INVALID_HANDLE_VALUE)
        return true;
#else
    if (hComm != -1)
        return true;
#endif
    return false;
}

int ConnectionEVB7COM::Open(const char *comName, int baudrate)
{

	if (strlen(comName) == 0) return ReportError("empty comm name");
#ifndef __unix__
    int errorCode = 0;
	// Initialize Overlap structures
	m_osROverlap.Internal = 0;
	m_osROverlap.InternalHigh = 0;
	m_osROverlap.Offset = 0;
	m_osROverlap.OffsetHigh = 0;
	m_osROverlap.hEvent = CreateEvent(NULL, false, false, NULL);

	m_osWOverlap.Internal = 0;
	m_osWOverlap.InternalHigh = 0;
	m_osWOverlap.Offset = 0;
	m_osWOverlap.OffsetHigh = 0;
	m_osWOverlap.hEvent = CreateEvent(NULL, false, false, NULL);

	// Initialize DSB structure
	memset(&m_dcbCommPort, 0, sizeof(m_dcbCommPort));

    m_dcbCommPort.BaudRate = 9600;
	m_dcbCommPort.fBinary = 1;
	m_dcbCommPort.fParity = 0;
	m_dcbCommPort.fOutxCtsFlow = 0;
	m_dcbCommPort.fOutxDsrFlow = 0;
	m_dcbCommPort.fDtrControl = 0;
	m_dcbCommPort.fDsrSensitivity = 0;
	m_dcbCommPort.fTXContinueOnXoff = 0;
	m_dcbCommPort.fOutX = 0;
	m_dcbCommPort.fInX = 0;
	m_dcbCommPort.fErrorChar = 0;
	m_dcbCommPort.fNull = 0;
	m_dcbCommPort.fRtsControl = 0;
	m_dcbCommPort.fAbortOnError = 0;
	m_dcbCommPort.fDummy2 = 0;
	// m_dcbCommPort.wReserved = 0;
	m_dcbCommPort.XonLim = 512;
	m_dcbCommPort.XoffLim = 512;
	m_dcbCommPort.ByteSize = 8;
	m_dcbCommPort.Parity = 0;
	m_dcbCommPort.StopBits = 0;
	//m_dcbCommPort.StopBits = 1;
	m_dcbCommPort.XonChar = 17;
	m_dcbCommPort.XoffChar = 19;
	m_dcbCommPort.ErrorChar = 0;
	m_dcbCommPort.EofChar = 26;
	m_dcbCommPort.EvtChar = 0;
	m_dcbCommPort.wReserved1 = 0;
	m_dcbCommPort.DCBlength = sizeof(DCB);

	// Initialize Timeout's
	m_ctmoNew.ReadIntervalTimeout = 50;
	m_ctmoNew.ReadTotalTimeoutMultiplier = 0;
	m_ctmoNew.ReadTotalTimeoutConstant = 100; // 1;
	m_ctmoNew.WriteTotalTimeoutMultiplier = 0;
	m_ctmoNew.WriteTotalTimeoutConstant = 100;

	// Open COM port
	std::string stmp;
	stmp = "\\\\.\\";
	stmp.append(comName);
	hComm = CreateFileA(stmp.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hComm != INVALID_HANDLE_VALUE)
	{
		// Set Events
		if (!SetCommMask(hComm, 0))
            errorCode = GetLastError();

		// Set Timeouts
		GetCommTimeouts(hComm, &m_ctmoOld);
		if (!SetCommTimeouts(hComm, &m_ctmoNew))
            errorCode = GetLastError();

		// Set DCB
		if (!SetCommState(hComm, &m_dcbCommPort))
            errorCode = GetLastError();
	}
	else
	{
        errorCode = GetLastError();
	};

	// Check the results
    if(errorCode != NOERROR)
	{
		//unsigned long err = GetLastError();
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
        return ReportError(errorCode, "Error setting up COM connection");
	}
	else
	{
		PurgeComm(hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
		return NOERROR;
	}
#else
    hComm = open(comName, O_RDWR | O_NOCTTY | O_SYNC);
    if(hComm < 0)
    {
        return ReportError("failed opening COM port");
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if( tcgetattr(hComm, &tty) != 0)
    {
//        MessageLog::getInstance()->write("Connection Manager: error from tcgetattr\n", LOG_ERROR);
        return ReportError(errno, "error from tcgetattr");
    }
    int speed = B9600;
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_iflag &= ~ICRNL;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0; // read non blocking
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);

    if(tcsetattr(hComm, TCSANOW, &tty) != 0)
    {
//        MessageLog::getInstance()->write("Connection manager: error from tcsetattr\n", LOG_ERROR);
        return ReportError(errno, "error from tcgetattr");
    }
#endif
    return 0;
}

/** @brief Sends data through COM port
    @param buffer data buffer to send
    @param length size of data buffer
    @param timeout_ms timeout limit for operation in milliseconds
    @return Number of bytes sent
*/
int ConnectionEVB7COM::Write(const unsigned char *buffer, int length, int timeout_ms)
{
    if(timeout_ms == 0)
    {
        timeout_ms = COM_TOTAL_TIMEOUT;
    }
    int retryCount = 0;
    const int maxRetries = (timeout_ms/COM_RETRY_INTERVAL) > 1 ? (timeout_ms/COM_RETRY_INTERVAL) : 1;
    bool status = false;
#ifndef __unix__
	unsigned long bytesWriten = 0;
	m_osWOverlap.InternalHigh = 0;

	for(int i = 0; i<maxRetries && status == false; ++i)
    {
        if (!WriteFile(hComm, buffer, length , &bytesWriten, NULL))
        {
            status = false;
        }
        else
            status = true;
        ++retryCount;
    }
#else
    long bytesWriten = 0;
    for(int i = 0; i<maxRetries && bytesWriten == 0; ++i)
    {
        bytesWriten = write(hComm, buffer, length);
        if(bytesWriten <= 0)
        {
//            if(bytesWriten < 0)
//                MessageLog::getInstance()->write("COM PORT: error writing data\n", LOG_ERROR);
//            if(bytesWriten == 0)
//                MessageLog::getInstance()->write("COM PORT: data bytes sent 0\n", LOG_WARNING);
            status = false;
        }
        else
            status = true;
        ++retryCount;
    }
#endif
    if(bytesWriten == length)
        status = true;

    if(status == false)
        ReportError(EIO, "Failed to write data");

    return bytesWriten;
}

/** @brief Reads data from COM port
    @param buffer pointer to data buffer for receiving
    @param length number of bytes to read
    @param timeout_ms timeout limit for operation in milliseconds
    @return Number of bytes received
*/
int ConnectionEVB7COM::Read(unsigned char *buffer, int length, int timeout_ms)
{
    if(timeout_ms == 0)
    {
        timeout_ms = COM_TOTAL_TIMEOUT;
    }
    int retryCount = 0;
    const int maxRetries = (timeout_ms/COM_RETRY_INTERVAL) > 1 ? (timeout_ms/COM_RETRY_INTERVAL) : 1;
    bool status = false;

    memset(buffer, 0, length);
	long bytesReaded = 0;
	unsigned long totalBytesReaded = 0;
	char cRawData[COM_BUFFER_LENGTH];
	unsigned long bytesToRead = length;

	memset(cRawData, '\0', sizeof(cRawData[0])*COM_BUFFER_LENGTH);

	for(int i=0; i<maxRetries && status == false; ++i)
    {
        memset(cRawData, '\0', sizeof(cRawData[0])*COM_BUFFER_LENGTH);
#ifndef __unix__

        DWORD bytesReceived = 0;
        if ( !ReadFile(hComm, cRawData, bytesToRead, &bytesReceived, NULL) )
        {
            status = false;
        }

        bytesReaded = bytesReceived;
#else
        bytesReaded = read(hComm, cRawData, bytesToRead);
        if(bytesReaded <= 0)
        {
//            if(bytesReaded < 0)
//                MessageLog::getInstance()->write("COM PORT: error reading data\n", LOG_ERROR);
//            if(bytesReaded == 0)
//                MessageLog::getInstance()->write("COM PORT: reading 0 bytes\n", LOG_WARNING);
            status = false;
        }
#endif
        retryCount++;

        for(int j=0; j<bytesReaded; ++j)
        {
            buffer[totalBytesReaded+j] = cRawData[j];
        }

        totalBytesReaded += bytesReaded;
        if(totalBytesReaded == bytesToRead)
            status = true;
    }

//    ss << " read(" << totalBytesReaded << "): ";
//    for(unsigned int i=0; i<64; ++i)
//        ss << int2hex(buffer[i], 1) << " ";
//    ss <<  " - retries: " << retryCount-1 << endl;
    //MessageLog::getInstance()->write(ss.str(), LOG_DATA);

//	if(retryCount == maxRetries)
//        MessageLog::getInstance()->write("COM PORT: read data timeout\n", LOG_WARNING);
//
//    if(totalBytesReaded > length)
//        MessageLog::getInstance()->write("COM PORT: read data corrupted, received length > requested length\n", LOG_ERROR);
	return totalBytesReaded;
}
