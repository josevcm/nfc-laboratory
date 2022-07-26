/**
    @file LMS64CProtocol.cpp
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#include "Logger.h"
#include "LMS64CProtocol.h"
#include "Si5351C.h"
#include <chrono>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <algorithm>
#include <iso646.h> // alternative operators for visual c++: not, and, or...
#include <ADCUnits.h>
#include <sstream>
using namespace lime;

//! CMD_LMS7002_RST options
const int LMS_RST_DEACTIVATE = 0;
const int LMS_RST_ACTIVATE = 1;
const int LMS_RST_PULSE = 2;

//! arbitrary spi constants used to dispatch calls
#define LMS7002M_SPI_INDEX 0x10
#define ADF4002_SPI_INDEX 0x30

static int convertStatus(const int &status, const LMS64CProtocol::GenericPacket &pkt)
{
    if (status != 0) return -1;
    switch (pkt.status)
    {
    case STATUS_COMPLETED_CMD: return 0;
    case STATUS_UNKNOWN_CMD:
        return ReportError(EPROTONOSUPPORT, "Command not supported");
    default: break;
    }
    return ReportError(EPROTO, status2string(pkt.status));
}

LMS64CProtocol::LMS64CProtocol(void)
{
    //set a sane-default for the rate
    _cachedRefClockRate = 61.44e6/2;
#ifdef REMOTE_CONTROL
    InitRemote();
#endif

}

LMS64CProtocol::~LMS64CProtocol(void)
{
#ifdef REMOTE_CONTROL
    CloseRemote();
#endif
    return;
}

int LMS64CProtocol::DeviceReset(int ind)
{
    if (not this->IsOpen())
    {
        return ReportError(ENOTCONN, "connection is not open");
    }

    GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RST;
    pkt.periphID = ind;
    pkt.outBuffer.push_back (LMS_RST_PULSE);
    int status = this->TransferPacket(pkt);

    return convertStatus(status, pkt);
}

int LMS64CProtocol::TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    //! TODO
    //! For multi-LMS7002M, RFIC # could be encoded with the slave number
    //! And the index would need to be encoded into the packet as well

    if (not this->IsOpen())
    {
        ReportError(ENOTCONN, "connection is not open");
        return -1;
    }

    //perform spi writes when there is no read data
    if (readData == nullptr) switch(addr)
    {
    case LMS7002M_SPI_INDEX: return this->WriteLMS7002MSPI(writeData, size);
    case ADF4002_SPI_INDEX: return this->WriteADF4002SPI(writeData, size);
    }

    //otherwise perform reads into the provided buffer
    if (readData != nullptr) switch(addr)
    {
    case LMS7002M_SPI_INDEX: return this->ReadLMS7002MSPI(writeData, readData, size);
    case ADF4002_SPI_INDEX: return this->ReadADF4002SPI(writeData, readData, size);
    }

    return ReportError(ENOTSUP, "unknown spi address");
}

int LMS64CProtocol::WriteI2C(const int addr, const std::string &data)
{
    if (not this->IsOpen())
    {
        return ReportError(ENOTCONN, "connection is not open");
    }
    return this->WriteSi5351I2C(data);
}

int LMS64CProtocol::ReadI2C(const int addr, const size_t numBytes, std::string &data)
{
    if (not this->IsOpen())
    {
        return ReportError(ENOTCONN, "connection is not open");
    }
    return this->ReadSi5351I2C(numBytes, data);
}

/***********************************************************************
 * LMS7002M SPI access
 **********************************************************************/
int LMS64CProtocol::WriteLMS7002MSPI(const uint32_t *writeData, size_t size, unsigned periphID)
{
    GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    pkt.periphID = periphID;
    for (size_t i = 0; i < size; ++i)
    {
        uint16_t addr = (writeData[i] >> 16) & 0x7fff;
        uint16_t data = writeData[i] & 0xffff;
        pkt.outBuffer.push_back(addr >> 8);
        pkt.outBuffer.push_back(addr & 0xFF);
        pkt.outBuffer.push_back(data >> 8);
        pkt.outBuffer.push_back(data & 0xFF);
    }

    int status = this->TransferPacket(pkt);

    return convertStatus(status, pkt);
}

int LMS64CProtocol::ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID)
{
    GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RD;
    pkt.periphID = periphID;
    for (size_t i = 0; i < size; ++i)
    {
        uint16_t addr = (writeData[i] >> 16) & 0x7fff;
        pkt.outBuffer.push_back(addr >> 8);
        pkt.outBuffer.push_back(addr & 0xFF);
    }

    int status = this->TransferPacket(pkt);

    const size_t numRead = std::min<size_t>(pkt.inBuffer.size()/4, size);
    for (size_t i = 0; i < numRead; ++i)
    {
        int hi = pkt.inBuffer[4*i + 2];
        int lo = pkt.inBuffer[4*i + 3];
        readData[i] = (hi << 8) | lo;
    }

    return convertStatus(status, pkt);
}

/***********************************************************************
 * Si5351 SPI access
 **********************************************************************/
int LMS64CProtocol::WriteSi5351I2C(const std::string &data)
{
    GenericPacket pkt;
    pkt.cmd = CMD_SI5351_WR;

    for (size_t i = 0; i < data.size(); i++)
    {
        pkt.outBuffer.push_back(data.at(i));
    }

    int status = this->TransferPacket(pkt);
    return convertStatus(status, pkt);
}

int LMS64CProtocol::ReadSi5351I2C(const size_t numBytes, std::string &data)
{
    GenericPacket pkt;
    pkt.cmd = CMD_SI5351_RD;

    int status = this->TransferPacket(pkt);

    for (size_t i = 0; i < data.size(); i++)
    {
        pkt.outBuffer.push_back(data.at(i));
    }

    data.clear();
    for (size_t i = 0; i < pkt.inBuffer.size(); ++i)
    {
        data += pkt.inBuffer[i];
    }

    return convertStatus(status, pkt);
}

/***********************************************************************
 * ADF4002 SPI access
 **********************************************************************/
int LMS64CProtocol::WriteADF4002SPI(const uint32_t *writeData, const size_t size)
{
    GenericPacket pkt;
    pkt.cmd = CMD_ADF4002_WR;

    for (size_t i = 0; i < size; i++)
    {
        pkt.outBuffer.push_back((writeData[i] >> 16) & 0xff);
        pkt.outBuffer.push_back((writeData[i] >> 8) & 0xff);
        pkt.outBuffer.push_back((writeData[i] >> 0) & 0xff);
    }

    int status = this->TransferPacket(pkt);
    return convertStatus(status, pkt);
}

int LMS64CProtocol::ReadADF4002SPI(const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    ReportError(ENOTSUP, "ReadADF4002SPI not supported");
    return -1;
}

/***********************************************************************
 * Board SPI access
 **********************************************************************/
int LMS64CProtocol::WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size)
{
    GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_WR;
    for (size_t i = 0; i < size; ++i)
    {
        pkt.outBuffer.push_back(addrs[i] >> 8);
        pkt.outBuffer.push_back(addrs[i] & 0xFF);
        pkt.outBuffer.push_back(data[i] >> 8);
        pkt.outBuffer.push_back(data[i] & 0xFF);
    }

    int status = this->TransferPacket(pkt);

    return convertStatus(status, pkt);
}

int LMS64CProtocol::ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size)
{
    GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_RD;
    for (size_t i = 0; i < size; ++i)
    {
        pkt.outBuffer.push_back(addrs[i] >> 8);
        pkt.outBuffer.push_back(addrs[i] & 0xFF);
    }

    int status = this->TransferPacket(pkt);

    const size_t numRead = std::min<size_t>(pkt.inBuffer.size()/4, size);
    for (size_t i = 0; i < numRead; ++i)
    {
        int hi = pkt.inBuffer[4*i + 2];
        int lo = pkt.inBuffer[4*i + 3];
        data[i] = (hi << 8) | lo;
    }

    return convertStatus(status, pkt);
}

/***********************************************************************
 * Device Information
 **********************************************************************/
DeviceInfo LMS64CProtocol::GetDeviceInfo(void)
{
    LMSinfo lmsInfo = this->GetInfo();
    DeviceInfo devInfo;
    devInfo.deviceName = GetDeviceName(lmsInfo.device);
    devInfo.expansionName = GetExpansionBoardName(lmsInfo.expansion);
    devInfo.firmwareVersion = std::to_string(int(lmsInfo.firmware));
    devInfo.hardwareVersion = std::to_string(int(lmsInfo.hardware));
    devInfo.protocolVersion = std::to_string(int(lmsInfo.protocol));
    devInfo.boardSerialNumber = lmsInfo.boardSerialNumber;

    FPGAinfo gatewareInfo = this->GetFPGAInfo();
    devInfo.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gatewareInfo.boardID));
    devInfo.gatewareVersion = std::to_string(int(gatewareInfo.gatewareVersion));
    devInfo.gatewareRevision = std::to_string(int(gatewareInfo.gatewareRevision));
    devInfo.hardwareVersion = std::to_string(int(gatewareInfo.hwVersion));

    return devInfo;
}

/** @brief Returns connected device information
*/
LMS64CProtocol::LMSinfo LMS64CProtocol::GetInfo()
{
    LMSinfo info;
    info.device = LMS_DEV_UNKNOWN;
    info.expansion = EXP_BOARD_UNKNOWN;
    info.firmware = 0;
    info.hardware = 0;
    info.protocol = 0;
    info.boardSerialNumber = 0;
    GenericPacket pkt;
    pkt.cmd = CMD_GET_INFO;
    int status = TransferPacket(pkt);
    if (status == 0 && pkt.inBuffer.size() >= 5)
    {
        info.firmware = pkt.inBuffer[0];
        info.device = pkt.inBuffer[1] < LMS_DEV_COUNT ? (eLMS_DEV)pkt.inBuffer[1] : LMS_DEV_UNKNOWN;
        info.protocol = pkt.inBuffer[2];
        info.hardware = pkt.inBuffer[3];
        info.expansion = pkt.inBuffer[4] < EXP_BOARD_COUNT ? (eEXP_BOARD)pkt.inBuffer[4] : EXP_BOARD_UNKNOWN;
        info.boardSerialNumber = 0;
        for (int i = 10; i < 18; i++)
        {
	        info.boardSerialNumber <<= 8;
	        info.boardSerialNumber |= pkt.inBuffer[i];
        }
    }
    return info;
}

/** @brief Returns information from FPGA gateware
*/
LMS64CProtocol::FPGAinfo LMS64CProtocol::GetFPGAInfo()
{
    FPGAinfo info;
    info.boardID = 0;
    info.gatewareVersion = 0;
    info.gatewareRevision = 0;
    GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_RD;
    const uint16_t addrs[] = {0x0000, 0x0001, 0x0002, 0x0003};
    for (size_t i = 0; i < 4; ++i)
    {
        pkt.outBuffer.push_back(addrs[i] >> 8);
        pkt.outBuffer.push_back(addrs[i] & 0xFF);
    }
    int status = this->TransferPacket(pkt);
    if (status == 0 && pkt.inBuffer.size() >= sizeof(addrs)*2)
    {
        info.boardID = (pkt.inBuffer[2] << 8) | pkt.inBuffer[3];
        info.gatewareVersion = (pkt.inBuffer[6] << 8) | pkt.inBuffer[7];
        info.gatewareRevision = (pkt.inBuffer[10] << 8) | pkt.inBuffer[11];
        info.hwVersion = pkt.inBuffer[15]&0x7F;
    }
    return info;
}


/** @brief Transfers data between packet and connected device
    @param pkt packet containing output data and to receive incomming data
    @return 0: success, other: failure
*/
int LMS64CProtocol::TransferPacket(GenericPacket& pkt)
{
    std::lock_guard<std::mutex> lock(mControlPortLock);
    int status = 0;
    if(IsOpen() == false) ReportError(ENOTCONN, "connection is not open");

    const int packetLen = ProtocolLMS64C::pktLength;
    int outLen = 0;
    unsigned char* outBuffer = PreparePacket(pkt, outLen);
    unsigned char* inBuffer = new unsigned char[outLen];
    memset(inBuffer, 0, outLen);

    int outBufPos = 0;
    int inDataPos = 0;
    if(outLen == 0)
        outLen = 1;

    for(int i=0; i<outLen; i+=packetLen)
    {
        if (callback_logData)
            callback_logData(true, &outBuffer[outBufPos], packetLen);
        int written = Write(&outBuffer[outBufPos], packetLen);
        if(written != packetLen)
        {
            status = lime::error("TransferPacket: Write failed (ret=%d)", written);
            break;
        }
        outBufPos += packetLen;
        int bread = Read(&inBuffer[inDataPos], packetLen);
        if(bread != packetLen)
        {
            status = lime::error("TransferPacket: Read failed (ret=%d)", bread);
            break;
        }
        if (callback_logData)
            callback_logData(false, &inBuffer[inDataPos], bread);
        inDataPos += bread;
    }
    ParsePacket(pkt, inBuffer, inDataPos);

    delete[] outBuffer;
    delete[] inBuffer;
    return convertStatus(status, pkt);
}

/** @brief Takes generic packet and converts to specific protocol buffer
    @param pkt generic data packet to convert
    @param length returns length of returned buffer
    @param protocol which protocol to use for data
    @return pointer to data buffer, must be manually deleted after use
*/
unsigned char* LMS64CProtocol::PreparePacket(const GenericPacket& pkt, int& length)
{
    unsigned char* buffer = NULL;

    ProtocolLMS64C packet;
    int maxDataLength = packet.maxDataLength;
    packet.cmd = pkt.cmd;
    packet.status = pkt.status;
    packet.periphID = pkt.periphID;
    int byteBlockRatio = 1; //block ratio - how many bytes in one block
    switch( packet.cmd )
    {
    case CMD_PROG_MCU:
    case CMD_GET_INFO:
    case CMD_SI5351_RD:
    case CMD_SI5356_RD:
        byteBlockRatio = 1;
        break;
    case CMD_SI5351_WR:
    case CMD_SI5356_WR:
        byteBlockRatio = 2;
        break;
    case CMD_LMS7002_RD:
    case CMD_BRDSPI_RD:
    case CMD_BRDSPI8_RD:
        byteBlockRatio = 2;
        break;
    case CMD_ADF4002_WR:
        byteBlockRatio = 3;
        break;
    case CMD_LMS7002_WR:
    case CMD_BRDSPI_WR:
    case CMD_ANALOG_VAL_WR:
        byteBlockRatio = 4;
        break;
    default:
        byteBlockRatio = 1;
    }
    if (packet.cmd == CMD_LMS7002_RD || packet.cmd == CMD_BRDSPI_RD)
        maxDataLength = maxDataLength/2;
    if (packet.cmd == CMD_ANALOG_VAL_RD)
        maxDataLength = maxDataLength / 4;
    int blockCount = pkt.outBuffer.size()/byteBlockRatio;
    int bufLen = blockCount/(maxDataLength/byteBlockRatio)
                +(blockCount%(maxDataLength/byteBlockRatio)!=0);
    bufLen *= packet.pktLength;
    if(bufLen == 0)
        bufLen = packet.pktLength;
    buffer = new unsigned char[bufLen];
    memset(buffer, 0, bufLen);
    unsigned int srcPos = 0;
    for(int j=0; j*packet.pktLength<bufLen; ++j)
    {
        int pktPos = j*packet.pktLength;
        buffer[pktPos] = packet.cmd;
        buffer[pktPos+1] = packet.status;
        buffer[pktPos + 3] = packet.periphID;
        if(blockCount > (maxDataLength/byteBlockRatio))
        {
            buffer[pktPos+2] = maxDataLength/byteBlockRatio;
            blockCount -= buffer[pktPos+2];
        }
        else
            buffer[pktPos+2] = blockCount;
        memcpy(&buffer[pktPos+4], packet.reserved, sizeof(packet.reserved));
        int bytesToPack = (maxDataLength/byteBlockRatio)*byteBlockRatio;
        for (int k = 0; k<bytesToPack && srcPos < pkt.outBuffer.size(); ++srcPos, ++k)
            buffer[pktPos + 8 + k] = pkt.outBuffer[srcPos];
    }
    length = bufLen;
    return buffer;
}

/** @brief Parses given data buffer into generic packet
    @param pkt destination packet
    @param buffer received data from board
    @param length received buffer length
    @param protocol which protocol to use for data parsing
    @return 1:success, 0:failure
*/
int LMS64CProtocol::ParsePacket(GenericPacket& pkt, const unsigned char* buffer, const int length)
{
    ProtocolLMS64C packet;
    int inBufPos = 0;
    pkt.inBuffer.resize(packet.maxDataLength*(length / packet.pktLength + (length % packet.pktLength)), 0);
    for(int i=0; i<length; i+=packet.pktLength)
    {
        pkt.cmd = (eCMD_LMS)buffer[i];
        pkt.status = (eCMD_STATUS)buffer[i+1];
        memcpy(&pkt.inBuffer[inBufPos], &buffer[i+8], packet.maxDataLength);
        inBufPos += packet.maxDataLength;
    }
    return 1;
}

int LMS64CProtocol::ProgramWrite(const char *data_src, const size_t length, const int prog_mode, const int device, ProgrammingCallback callback)
{
#ifndef NDEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    //erasing FLASH can take up to 3 seconds before reply is received
    const int progTimeout_ms = 5000;
    char progressMsg[128];
    sprintf(progressMsg, "in progress...");
    bool abortProgramming = false;
    int bytesSent = 0;

    bool needsData = true;
    if(device == FPGA && prog_mode == 2)
        needsData = false;
    if(device == FX3 && (prog_mode == 0 || prog_mode == 1))
        needsData = false;

    if(length == 0 && needsData)
    {
        return ReportError(EIO, "ProgramWrite length should be > 0");
    }

    if (not this->IsOpen())
    {
        return ReportError(ENOTCONN, "connection is not open");
    }

    const int pktSize = 32;
    int data_left = length;
    const int portionsCount = length/pktSize + (length%pktSize > 0) + 1; // +1 programming end packet
    int portionNumber;
    int status = 0;
    eCMD_LMS cmd;
    if(device == HPM || device == FX3)
        cmd = CMD_MEMORY_WR;
    else if(device == FPGA)
        cmd = CMD_ALTERA_FPGA_GW_WR;
    else
    {
        sprintf(progressMsg, "Programming failed! Target device not supported");
        if(callback)
            callback(bytesSent, length, progressMsg);
        return ReportError(ENOTSUP, progressMsg);
    }

    unsigned char ctrbuf[64];
    unsigned char inbuf[64];
    memset(ctrbuf, 0, 64);
    ctrbuf[0] = cmd;
    ctrbuf[1] = 0;
    ctrbuf[2] = 56;

    for (portionNumber = 0; portionNumber<portionsCount && !abortProgramming; ++portionNumber)
    {
        int offset = 8;
        memset(&ctrbuf[offset], 0, 56);
        ctrbuf[offset+0] = prog_mode;
        ctrbuf[offset+1] = (portionNumber >> 24) & 0xFF;
        ctrbuf[offset+2] = (portionNumber >> 16) & 0xFF;
        ctrbuf[offset+3] = (portionNumber >> 8) & 0xFF;
        ctrbuf[offset+4] = portionNumber & 0xFF;
        unsigned char data_cnt = data_left > pktSize ? pktSize : data_left;
        ctrbuf[offset+5] = data_cnt;
        if(cmd == CMD_MEMORY_WR)
        {
            ctrbuf[offset+6] = 0;
            ctrbuf[offset+7] = 0;
            ctrbuf[offset+8] = 0;
            ctrbuf[offset+9] = 0;

            ctrbuf[offset+10] = (device >> 8) & 0xFF;
            ctrbuf[offset+11] = device & 0xFF;
        }
        if(data_src != NULL)
        {
            memcpy(&ctrbuf[offset + 24], data_src, data_cnt);
            data_src += data_cnt;
        }

        if(Write(ctrbuf, sizeof(ctrbuf)) != sizeof(ctrbuf))
        {
            if(callback)
                callback(bytesSent, length, "Programming failed! Write operation failed");
            return ReportError(EIO, "Programming failed! Write operation failed");
        }
        if(Read(inbuf, sizeof(inbuf), progTimeout_ms) != sizeof(ctrbuf))
        {
            if(callback)
                callback(bytesSent, length, "Programming failed! Read operation failed");
            return ReportError(EIO, "Programming failed! Read operation failed");
        }
        data_left -= data_cnt;
        status = inbuf[1];
        bytesSent += data_cnt;

        if(status != STATUS_COMPLETED_CMD)
        {
            sprintf(progressMsg, "Programming failed! %s", status2string(status));
            if(callback)
                callback(bytesSent, length, progressMsg);
            return ReportError(EPROTO, progressMsg);
        }
        if(needsData == false) //only one packet is needed to initiate bitstream from flash
        {
            bytesSent = length;
            break;
        }
        if(callback)
            abortProgramming = callback(bytesSent, length, progressMsg);
    }
    if (abortProgramming == true)
    {
        sprintf(progressMsg, "programming: aborted by user");
        if(callback)
            callback(bytesSent, length, progressMsg);
        return ReportError(ECONNABORTED, "user aborted programming");
    }
    sprintf(progressMsg, "programming: completed");
    if(callback)
        callback(bytesSent, length, progressMsg);
#ifndef NDEBUG
    auto t2 = std::chrono::high_resolution_clock::now();
	if ((device == 2 && prog_mode == 2) == false)
	    lime::log(LOG_LEVEL_INFO, "Programming finished, %li bytes sent! %li ms\n", length, std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
	else
        lime::log(LOG_LEVEL_INFO, "FPGA configuring initiated\n");
#endif
    return 0;
}

int LMS64CProtocol::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_RD;

    for (size_t i=0; i<count; ++i)
        pkt.outBuffer.push_back(ids[i]);

    int status = this->TransferPacket(pkt);
    if (status != 0) return status;

    assert(pkt.inBuffer.size() >= 4 * count);

    for (size_t i = 0; i < count; ++i)
    {
        int unitsIndex = pkt.inBuffer[i * 4 + 1];
        if(units)
        {

            if (unitsIndex&0x0F)
            {
                const char adc_units_prefix[] = {
                    ' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z',
                    'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm'};
                units[i] = adc_units_prefix[unitsIndex&0x0F]+adcUnits2string((unitsIndex & 0xF0)>>4);
            }
            else
                units[i] += adcUnits2string((unitsIndex & 0xF0)>>4);
        }
        if((unitsIndex & 0xF0)>>4 == RAW)
        {
            values[i] = (uint16_t)(pkt.inBuffer[i * 4 + 2] << 8 | pkt.inBuffer[i * 4 + 3]);
        }
        else
        {
            values[i] = (int16_t)(pkt.inBuffer[i * 4 + 2] << 8 | pkt.inBuffer[i * 4 + 3]);
            if((unitsIndex & 0xF0)>>4 == TEMPERATURE)
                values[i] /= 10;
        }
    }
    return 0;
}

int LMS64CProtocol::CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_ANALOG_VAL_WR;

    for (size_t i = 0; i < count; ++i)
    {
        pkt.outBuffer.push_back(ids[i]);
        int powerOf10 = 0;
        if(values[i] > 65535.0 && (units != ""))
            powerOf10 = log10(values[i]/65.536)/3;
        if (values[i] < 65.536 && (units != ""))
            powerOf10 = log10(values[i]/65535.0) / 3;
        int unitsId = 0; // need to convert given units to their enum
        pkt.outBuffer.push_back(unitsId << 4 | powerOf10);
        int value = values[i] / pow(10, 3*powerOf10);
        pkt.outBuffer.push_back(value >> 8);
        pkt.outBuffer.push_back(value & 0xFF);
    }
    return TransferPacket(pkt);
}

int LMS64CProtocol::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_WR;
    for (size_t i=0; i<bufLength; ++i)
        pkt.outBuffer.push_back(buffer[i]);
    return TransferPacket(pkt);
}

int LMS64CProtocol::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_RD;
    int status = TransferPacket(pkt);
    if(status != 0)
        return status;

    for (size_t i=0; i<bufLength; ++i)
        buffer[i] = pkt.inBuffer[i];
    return status;
}

int LMS64CProtocol::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_DIR_WR;
    for (size_t i=0; i<bufLength; ++i)
        pkt.outBuffer.push_back(buffer[i]);
    return TransferPacket(pkt);
}

int LMS64CProtocol::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_GPIO_DIR_RD;
    int status = TransferPacket(pkt);
    if(status != 0)
        return convertStatus(status, pkt);

    for (size_t i=0; i<bufLength; ++i)
        buffer[i] = pkt.inBuffer[i];
    return status;
}

int LMS64CProtocol::ProgramMCU(const uint8_t *buffer, const size_t length, const MCU_PROG_MODE mode, ProgrammingCallback callback)
{
#ifndef NDEBUG
    auto timeStart = std::chrono::high_resolution_clock::now();
#endif
    const uint8_t fifoLen = 32;
    bool success = true;
    bool terminate = false;

    int packetNumber = 0;
    int status = STATUS_UNDEFINED;

    LMS64CProtocol::GenericPacket pkt;
    pkt.cmd = CMD_PROG_MCU;

    if (callback)
        terminate = callback(0, length,"");

    for(uint16_t CntEnd=0; CntEnd<length && !terminate; CntEnd+=32)
    {
        pkt.outBuffer.clear();
        pkt.outBuffer.reserve(fifoLen+2);
        pkt.outBuffer.push_back(mode);
        pkt.outBuffer.push_back(packetNumber++);
        for (uint8_t i=0; i<fifoLen; i++)
            pkt.outBuffer.push_back(buffer[CntEnd + i]);

        TransferPacket(pkt);
        status = pkt.status;
        if (callback)
            terminate = callback(CntEnd+fifoLen,length,"");
#ifndef NDEBUG
        lime::log(LOG_LEVEL_INFO, "MCU programming : %4i/%4li\r", CntEnd+fifoLen, long(length));
#endif
        if(status != STATUS_COMPLETED_CMD)
        {
            std::stringstream ss;
            ss << "Programming MCU: status : not completed, block " << packetNumber << std::endl;
            success = false;
            break;
        }

        if(mode == 3) // if boot mode , send only first packet
        {
            if (callback)
                callback(1, 1, "");
            break;
        }
	};
#ifndef NDEBUG
    auto timeEnd = std::chrono::high_resolution_clock::now();
    lime::log(LOG_LEVEL_INFO, "\nMCU Programming finished, %li ms\n",
            std::chrono::duration_cast<std::chrono::milliseconds>
            (timeEnd-timeStart).count());
#endif
    return success ? 0 : -1;
}

/**	@brief Reads chip version information form LMS7 chip.
*/

