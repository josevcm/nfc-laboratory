/**
    @file ConnectionSPI.cpp
    @author Lime Microsystems
    @brief Implementation of RPi 3 SPI connection for LimeNET-Micro.
*/

#include <linux/spi/spidev.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <fstream>
#include <wiringPi.h>

#include "Logger.h"
#include "lime_spi.h"
#include "ConnectionSPI.h"
#include "SystemResources.h"

using namespace std;
using namespace lime;

#define SPI_STREAM_SPEED_HZ 50000000
#define SPI_CTRL_SPEED_HZ   1000000

ConnectionSPI* ConnectionSPI::pthis = nullptr;
char ConnectionSPI::last_flags = 0x10;  //no sync
uint64_t ConnectionSPI::rx_timestamp = 0;
std::atomic<bool> ConnectionSPI::program_mode(false);
std::atomic<bool> ConnectionSPI::program_ready(false);
/** @brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionSPI::ConnectionSPI(const unsigned index) :
    fd_stream(-1),
    fd_stream_clocks(-1),
    fd_control_lms(-1),
    fd_control_fpga(-1),
    int_pin(26)
{
    pthis = this;
    program_ready.store(false);
    program_mode.store(false);
    if (Open(index) < 0)
        lime::error("Failed to open SPI device");
    if (index == SPIDEV)
    {
        std::ifstream file("/proc/device-tree/model");
        if (file.good())
        {
            std::string str;
            std::getline(file, str);
            lime::info(str.c_str());
            if (str.find("Module")!=std::string::npos)
                int_pin = 12;
        }

        wiringPiSetup();
        pinMode(int_pin, INPUT);
        pullUpDnControl(int_pin, PUD_OFF);
        wiringPiISR(int_pin, INT_EDGE_RISING, &ConnectionSPI::StreamISR);
    }

    uint8_t id = 0;
    double val = 35487;
    CustomParameterWrite(&id, &val, 1, "");
}
/** @brief Closes connection to chip and deallocates used memory.
*/
ConnectionSPI::~ConnectionSPI()
{
    Close();
}

/** @brief Tries to open connected USB device and find communication endpoints.
    @return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionSPI::Open(const unsigned index)
{
    Close();
    const uint32_t mode = 0;
    const uint8_t bits = 8;
    const uint32_t speed_stream = SPI_STREAM_SPEED_HZ;
    const uint32_t speed_control = SPI_CTRL_SPEED_HZ;

    if (index == SPIDEV)
    {
        if ((fd_stream = open("/dev/spidev0.0", O_RDWR)) != -1)
        {
            if (ioctl(fd_stream, SPI_IOC_WR_MODE, &mode) < 0)
                lime::error("Failed to set SPI write mode");
            if (ioctl(fd_stream, SPI_IOC_RD_MODE, &mode) < 0)
                lime::error("Failed to set SPI read mode");
            if (ioctl(fd_stream, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
                lime::error("Failed to set SPI bits per word (write)");
            if (ioctl(fd_stream, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
                lime::error("Failed to set SPI bits per word (read)");
            if (ioctl(fd_stream, SPI_IOC_WR_MAX_SPEED_HZ, &speed_stream) < 0)
                lime::error("Failed to set SPI write max speed");
            if (ioctl(fd_stream, SPI_IOC_RD_MAX_SPEED_HZ, &speed_stream) < 0)
                lime::error("Failed to set SPI read max speed");
        }

        if ((fd_stream_clocks = open("/dev/spidev0.1", O_RDWR)) != -1)
        {
            if (ioctl(fd_stream_clocks, SPI_IOC_WR_MODE, &mode) < 0)
                lime::error("Failed to set SPI write mode");
            if (ioctl(fd_stream_clocks, SPI_IOC_RD_MODE, &mode) < 0)
                lime::error("Failed to set SPI read mode");
            if (ioctl(fd_stream_clocks, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
                lime::error("Failed to set SPI bits per word (write)");
            if (ioctl(fd_stream_clocks, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
                lime::error("Failed to set SPI bits per word (read)");
            if (ioctl(fd_stream_clocks, SPI_IOC_WR_MAX_SPEED_HZ, &speed_stream) < 0)
                lime::error("Failed to set SPI write max speed");
            if (ioctl(fd_stream_clocks, SPI_IOC_RD_MAX_SPEED_HZ, &speed_stream) < 0)
                lime::error("Failed to set SPI read max speed");
        }
    }
    else if((fd_stream = open("/dev/lime_spi", O_RDWR)) < 0)
        lime::error("Failed to open /dev/lime_spi");

    if ((fd_control_fpga = open("/dev/spidev1.0", O_RDWR)) != -1)
    {
        if (ioctl(fd_control_fpga, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_control_fpga, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_control_fpga, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_control_fpga, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_control_fpga, SPI_IOC_WR_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_control_fpga, SPI_IOC_RD_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI read max speed");
    }

    if ((fd_control_lms = open("/dev/spidev1.1", O_RDWR)) != -1)
    {
        if (ioctl(fd_control_lms, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_control_lms, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_control_lms, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_control_lms, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_control_lms, SPI_IOC_WR_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_control_lms, SPI_IOC_RD_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI read max speed");
    }

    return IsOpen() ? 0 : -1;
}

/** @brief Closes communication to device.
*/
void ConnectionSPI::Close()
{
    if (fd_stream != -1)
        close(fd_stream);
    if (fd_stream_clocks != -1)
        close(fd_stream_clocks);
    if (fd_control_lms != -1)
        close(fd_control_lms);
    if (fd_control_fpga != -1)
        close(fd_control_fpga);
    fd_stream = -1;
    fd_stream_clocks = -1;
    fd_control_lms = -1;
    fd_control_fpga = -1;
}

/** @brief Returns connection status
    @return 1-connection open, 0-connection closed.
*/
bool ConnectionSPI::IsOpen()
{
    if (fd_stream < 0 || fd_control_lms < 0 || fd_control_fpga < 0)
        return false;
    return true;
}

int ConnectionSPI::TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    if (not this->IsOpen())
    {
        ReportError(ENOTCONN, "connection is not open");
        return -1;
    }

    //perform spi writes when there is no read data
    if (readData == nullptr) switch(addr)
    {
        case 0x10: return this->WriteLMS7002MSPI(writeData, size);
        case 0x30: return this->WriteADF4002SPI(writeData, size);
    }

    if (readData != nullptr && addr == 0x10)
        return this->ReadLMS7002MSPI(writeData, readData, size);

    return ReportError(ENOTSUP, "unknown spi address");
}

int ConnectionSPI::TransferSPI(int fd, const void *tx, void *rx, uint32_t len)
{
    spi_ioc_transfer tr = { (unsigned long)tx,
                            (unsigned long)rx,
                            len,
                            SPI_CTRL_SPEED_HZ,
                            0,
                            8 };
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (pthis->callback_logData)
    {
        pthis->callback_logData(true, (const unsigned char*)tx, len);
        pthis->callback_logData(false, (const unsigned char*)rx, len);
    }
    return ret;
}

void ConnectionSPI::SetChipSelect(int cs)
{
    uint32_t addr = 0x12, val = ~(1<<cs);
    WriteRegisters(&addr, &val, 1); //CS from FPGA
}

int ConnectionSPI::WriteADF4002SPI(const uint32_t *data, const size_t size)
{
    //disable DAC
    uint8_t rx[3];
    uint8_t tx[3] = {3, 0, 0};
    SetChipSelect(1);
    if (TransferSPI(fd_control_lms, &tx, &rx, 3) != 3)
        lime::error("Write DAC: SPI transfer error");

    uint8_t readData[3];
    uint8_t writeData[3];
    for (unsigned i = 0; i < size; i++)
    {
    	writeData[0] = (data[i]>>16)&0xff;
    	writeData[1] = (data[i]>>8)&0xff;
    	writeData[2] = data[i]&0xff;
        SetChipSelect(2);
        if (TransferSPI(fd_control_lms, writeData, readData, 3)!=3)
        {
            lime::error("Write ADF: SPI transfer error");
            return -1;
        }
        SetChipSelect(0);
    }
    return 0;
}

/***********************************************************************
 * LMS7002M SPI access
 **********************************************************************/
int ConnectionSPI::WriteLMS7002MSPI(const uint32_t *data, size_t size, unsigned /*periphID*/)
{
    uint32_t* readData = new uint32_t[size];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;

    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = (data[i]>>24)&0xff;
    	writeData[4*i+1] = (data[i]>>16)&0xff;
    	writeData[4*i+2] = (data[i]>>8)&0xff;
    	writeData[4*i+3] = data[i]&0xff;
    }

    SetChipSelect(0);
    if (TransferSPI(fd_control_lms, writeData, readData, size*4)!=size*4)
    {
    	lime::error("Write LMS7: SPI transfer error");
    	ret = -1;
    }
    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::ReadLMS7002MSPI(const uint32_t *data_in, uint32_t *data_out, size_t size, unsigned periphID)
{
    uint8_t* readData = new uint8_t[size*4];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;

    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = (data_in[i]>>24)&0xff;
    	writeData[4*i+1] = (data_in[i]>>16)&0xff;
    	writeData[4*i+2] = 0;
    	writeData[4*i+3] = 0;
    }

    SetChipSelect(0);
    if (TransferSPI(fd_control_lms, writeData, readData, size*4) != size*4)
    {
    	lime::error("Read LMS7: SPI transfer error");
    	ret = -1;
    }

    for (unsigned i = 0; i < size; i++)
    	data_out[i] = readData[4*i+3] | (readData[4*i+2]<<8);

    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::WriteRegisters(const uint32_t *addrs, const uint32_t *vals, const size_t size)
{
    uint32_t* readData = new uint32_t[size];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;
    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = ((addrs[i]>>8)&0xff)|0x80;
    	writeData[4*i+1] = addrs[i]&0xff;
    	writeData[4*i+2] = (vals[i]>>8)&0xff;
    	writeData[4*i+3] = vals[i]&0xff;
    }

    if (TransferSPI(fd_control_fpga, writeData, readData, size*4) != size*4)
    {
    	lime::error("Write FPGA: SPI transfer error");
    	ret = -1;
    }

    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::ReadRegisters(const uint32_t *addrs, uint32_t *vals, const size_t size)
{
    uint8_t* readData = new uint8_t[size*4];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;
    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = (addrs[i]>>8)&0xff;
    	writeData[4*i+1] = addrs[i]&0xff;
    	writeData[4*i+2] = 0;
    	writeData[4*i+3] = 0;
    }
    if (TransferSPI(fd_control_fpga, writeData, readData, size*4) != size*4)
    {
        lime::error("Read FPGA: SPI transfer error");
        ret = -1;
    }

    for (unsigned i = 0; i < size; i++)
    	vals[i] = readData[4*i+3] | (readData[4*i+2]<<8);

    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    for (unsigned i = 0; i < count; i++)
        values[i] = ids[i] == 0 ? dac_value : 0;

    return 0;
}

int ConnectionSPI::CustomParameterWrite(const uint8_t *ids, const double *vals, const size_t count, const std::string& units)
{
   int ret = 0;

   for (unsigned i = 0; i < count; i++)
        if (ids[i] == 0)
        {
            //disable ADF
            uint8_t writeData[12] = {0x1F, 0x81, 0xF3, 0x1F, 0x81, 0xF2, 0, 1, 0xF4, 1, 0x80, 1};
            uint8_t readData[3];
            for (unsigned i = 0; i < sizeof(writeData); i+=3)
            {
                SetChipSelect(2);
                if (TransferSPI(fd_control_lms, writeData+i, readData, 3)!=3)
                {
                   lime::error("Write ADF: SPI transfer error");
                   ret = -1;
                }
                SetChipSelect(0);
            }

            if (TransferSPI(fd_control_lms, writeData, readData, 12)!=12)
                lime::error("Write ADF: SPI transfer error");

            uint8_t  rx[3];
            dac_value = vals[i];
            uint8_t  tx[3] = {0, (dac_value>>8), (dac_value&0xFF)};
            SetChipSelect(1);
            lime::info("DAC write: %d", dac_value);
            if (TransferSPI(fd_control_lms, &tx, &rx, 3) != 3)
            {
                lime::error("Write DAC: SPI transfer error");
                ret = -1;
            }
        }

    return ret;
}

DeviceInfo ConnectionSPI::GetDeviceInfo(void)
{
    DeviceInfo info;
    info.deviceName = "LimeNET-Micro";
    info.protocolVersion = "0";
    info.firmwareVersion = "0";
    info.expansionName = "EXP_BOARD_UNKNOWN";

    const uint32_t addrs[] = {0, 1, 2, 3};
    uint32_t vals[4] = {0};

    ReadRegisters(addrs, vals, 4);
    info.gatewareTargetBoard = GetDeviceName(eLMS_DEV(vals[0]));
    info.gatewareVersion = std::to_string(vals[1]);
    info.gatewareRevision = std::to_string(vals[2]);
    info.hardwareVersion = std::to_string(vals[3]&0xF) + " BOM " + std::to_string(vals[3]>>4);
    return info;
}

int ConnectionSPI::DeviceReset(int ind)
{
    const uint32_t addrs[2] = {0x13, 0x13};
    uint32_t vals[2] = {};
    ReadRegisters(addrs, vals, 1);
    vals[0] = vals[0] & (~0x02);
    vals[1] = vals[0] | 0x02;
    WriteRegisters(addrs, vals, 2);
    return 0;
}

int ConnectionSPI::ResetStreamBuffers()
{
    if (fd_stream_clocks < 0) //lime spi
    {
        if (fd_stream != -1)
            close(fd_stream);
        if((fd_stream = open("/dev/lime_spi", O_RDWR)) < 0)
        {
            lime::error("Failed to open /dev/lime_spi");
            return -1;
        }
        return 0;
    }

    {
        std::lock_guard<std::mutex> lock(mTxStreamLock);
        txQueue = std::queue<FPGA_DataPacket>();
    }
    {
        std::lock_guard<std::mutex> lock(mRxStreamLock);
        rxQueue = std::queue<FPGA_DataPacket>();
    }
    program_mode.store(false);
    last_flags = 0x10; //no sync
    rx_timestamp = 0;
    return 0;
}

int ConnectionSPI::GetBuffersCount() const
{
    return 1;
}

int ConnectionSPI::CheckStreamSize(int size) const
{
    return 1;
}

/**
    @brief Reads data from board
    @param buffer array where to store received data
    @param length number of bytes to read
    @param timeout read timeout in milliseconds
    @return number of bytes received
*/
int ConnectionSPI::ReceiveData(char *buffer, int length, int epIndex, int timeout_ms)
{
    const int count = length/sizeof(FPGA_DataPacket);
    int offset = 0;
    for (int i = 0; i <  count; i++)
    {
        if (WaitForReading(0, timeout_ms)== false)
            return i * sizeof(FPGA_DataPacket);
        FinishDataReading(buffer+offset, sizeof(FPGA_DataPacket), 0);
        offset += sizeof(FPGA_DataPacket);
    }
    return count * sizeof(FPGA_DataPacket);
}

void ConnectionSPI::AbortReading(int epIndex)
{
    return;
}

/**
    @brief  sends data to board
    @param *buffer buffer to send
    @param length number of bytes to send
    @param timeout data write timeout in milliseconds
    @return number of bytes sent
*/
int ConnectionSPI::SendData(const char *buffer, int length, int epIndex, int timeout_ms)
{
    const int count = length/sizeof(FPGA_DataPacket);
    int offset = 0;
    for (int i = 0; i <  count; i++)
    {
        BeginDataSending(buffer+offset, length, 0);
        if (WaitForSending(0, timeout_ms) == false)
            return i * sizeof(FPGA_DataPacket);
        offset += sizeof(FPGA_DataPacket);
    }
    return count * sizeof(FPGA_DataPacket);
}

void ConnectionSPI::AbortSending(int epIndex)
{
    return;
}

int ConnectionSPI::BeginDataReading(char* buffer, uint32_t length, int ep)
{
    return 0;
}

bool ConnectionSPI::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    if (fd_stream_clocks < 0) //lime spi
        return true;
    auto t1 = chrono::high_resolution_clock::now();
    do
    {
        {
            std::lock_guard<std::mutex> lock(mRxStreamLock);
            if (!rxQueue.empty())
                return true;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
        return false;
}

int ConnectionSPI::FinishDataReading(char* buffer, uint32_t length, int contextHandle)
{
    if (fd_stream_clocks < 0) //lime spi
    {
        auto t1 = chrono::high_resolution_clock::now();
        do
        {
            int cnt = read(fd_stream, buffer, sizeof(FPGA_DataPacket));
            if (cnt >= 0)
                return cnt;
            std::this_thread::sleep_for(std::chrono::microseconds(300));
        }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < 3000);
        return 0;
    }

    FPGA_DataPacket* packet = reinterpret_cast<FPGA_DataPacket*>(buffer);
    std::lock_guard<std::mutex> lock(mRxStreamLock);
    if (!rxQueue.empty())
    {
        *packet = rxQueue.front();
        rxQueue.pop();
        return sizeof(FPGA_DataPacket);;
    }
    return 0;
}

int ConnectionSPI::BeginDataSending(const char* buffer, uint32_t length, int ep)
{
    if (fd_stream_clocks < 0) //lime spi
    {
        auto t1 = chrono::high_resolution_clock::now();
        do
        {
            int cnt = write(fd_stream, buffer, sizeof(FPGA_DataPacket));
            if (cnt >= 0)
                return cnt;
            std::this_thread::sleep_for(std::chrono::microseconds(300));
        }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < 3000);
        return 0;
    }
    const FPGA_DataPacket* packet = reinterpret_cast<const FPGA_DataPacket*>(buffer);
    std::lock_guard<std::mutex> lock(mTxStreamLock);
    if (txQueue.size() < 10)
    {
        txQueue.push(*packet);
        return sizeof(FPGA_DataPacket);
    }
    return 0;
}


bool ConnectionSPI::WaitForSending(int contextHandle, uint32_t timeout_ms)
{
    if (fd_stream_clocks < 0) //lime spi
        return true;
    auto t1 = chrono::high_resolution_clock::now();
    do
    {
        {
            std::lock_guard<std::mutex> lock(mTxStreamLock);
            if (txQueue.size() < 10)
                return true;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
    return false;
}

int ConnectionSPI::FinishDataSending(const char* buffer, uint32_t length, int contextHandle)
{
    return contextHandle;
}

void ConnectionSPI::StreamISR()
{
    if (program_mode.load())
    {
        program_ready.store(true);
        return;
    }
    static const FPGA_DataPacket dummy_packet = {0};
    FPGA_DataPacket rx_packet;
    FPGA_DataPacket tx_packet;
    while (1)
    {
        pthis->mTxStreamLock.lock();
        if (pthis->txQueue.empty() || (!(tx_packet.reserved[0]&0x10) && pthis->txQueue.front().counter > rx_timestamp+12000))
        {
            pthis->mTxStreamLock.unlock();
            tx_packet = dummy_packet;
            tx_packet.reserved[0] = last_flags;
        }
        else
        {
            tx_packet = pthis->txQueue.front();
            last_flags = tx_packet.reserved[0];
            pthis->txQueue.pop();
            pthis->mTxStreamLock.unlock();
            if (!(tx_packet.reserved[0]&0x10) && tx_packet.counter < rx_timestamp+6000)
                continue;
        }
        break;
    }

    {
        spi_ioc_transfer tr = { (unsigned long)&tx_packet,
                                (unsigned long)&rx_packet,
                                sizeof(tx_packet),
                                SPI_STREAM_SPEED_HZ,
                                0,
                                8 };
        ioctl(pthis->fd_stream, SPI_IOC_MESSAGE(1), &tr);
    }

    rx_timestamp = rx_packet.counter;

    pthis->mRxStreamLock.lock();

    if (pthis->rxQueue.size() >= 10)
        pthis->rxQueue.pop();
    pthis->rxQueue.push(rx_packet);
    pthis->mRxStreamLock.unlock();

    {
        spi_ioc_transfer tr = { (unsigned long)&dummy_packet,
                                (unsigned long)&rx_packet,
                                2,
                                SPI_STREAM_SPEED_HZ,
                                0,
                                8 };
        ioctl(pthis->fd_stream_clocks, SPI_IOC_MESSAGE(1), &tr);
    }
}

int ConnectionSPI::ProgramWrite(const char *data, size_t length, int prog_mode, int device, ProgrammingCallback cb)
{
    if (device != 2)
    {
        lime::error("Unsupported programming target");
        return -1;
    }
    if (prog_mode != 1 && prog_mode != 2)
    {
        lime::error("Programming mode not supported");
        return -1;
    }

    uint32_t addr = 0x7F;
    uint32_t val = 0x00;
    WriteRegisters(&addr, &val, 1);
    val = 0x01;
    WriteRegisters(&addr, &val, 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (prog_mode != 2)
    {
        if (fd_stream_clocks < 0)
        {
            ioctl(fd_stream, IOCTL_PROGSTART, 0);
        }
        else
        {
            program_mode.store(true);
            program_ready.store(false);
        }
    }

    val = 0x00;
    WriteRegisters(&addr, &val, 1);
    val = 0x01;
    WriteRegisters(&addr, &val, 1);

    if (prog_mode == 2)
        return 0;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    static unsigned char rx_buffer[4096];
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
    memcpy(buffer.data(), data + startUFM, sizeUFM);
    memcpy(buffer.data() + sizeUFM, data + startCFM0, sizeCFM0);

    unsigned data_sent = 0;
    int ret = 0;
    while (data_sent < buffer.size())
    {
        int cnt = 0;
        auto t1 = chrono::high_resolution_clock::now();

        while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < 3000)
        {
            if (fd_stream_clocks < 0) //lime_spi
            {
                cnt = write(fd_stream, &buffer[data_sent], sizeof(FPGA_DataPacket));
                std::this_thread::sleep_for(std::chrono::milliseconds(85));
                if (cnt > 0)
                    break;
                cnt = 0;

            }
            else if (program_ready.load())
            {
                cnt = buffer.size() - data_sent;
                if (cnt > 4096)
                    cnt = 4096;
                {
                    spi_ioc_transfer tr = { (unsigned long)(&buffer[data_sent]),
                                            (unsigned long)&rx_buffer,
                                            cnt,
                                            SPI_STREAM_SPEED_HZ,
                                            0,
                                            8 };
                    ioctl(pthis->fd_stream, SPI_IOC_MESSAGE(1), &tr);
                }
                program_ready.store(false);
                {
                    spi_ioc_transfer tr = { (unsigned long)&rx_buffer,
                                            (unsigned long)&rx_buffer,
                                            2,
                                            SPI_STREAM_SPEED_HZ,
                                            0,
                                            8 };
                    ioctl(pthis->fd_stream_clocks, SPI_IOC_MESSAGE(1), &tr);
                }
                break;
            }
        }
        if (cnt <= 0)
        {
            lime::error("timeout");
            ret = -1;
            break;
        }
        data_sent +=cnt;
        if(cb && cb(data_sent, buffer.size(), ""))
        {
            lime::info("aborted");
            ret = -1;
            break;
        }
    }

    if (fd_stream_clocks < 0)   //lime_spi
        ioctl(fd_stream, IOCTL_PROGSTOP, 0);
    else
        program_mode.store(false);
    return ret;
}


int ConnectionSPI::ProgramUpdate(const bool download, const bool force, IConnection::ProgrammingCallback callback)
{
    const int updateGW = 1;
    const int updateGWr = 3;
    const std::string image = "LimeNET-Micro_lms7_trx_HW_2.1_SPI_r1.3.rpd";

    const uint32_t addrs[] = {1, 2};
    uint32_t vals[2] = {0};

    ReadRegisters(addrs, vals, 2);

    int gwVersion = vals[0];
    int gwRevision = vals[1];

    //download images when missing
    if (download && lime::locateImageResource(image).empty())
    {
        const std::string msg("Downloading: " + image);
        if (callback)
            callback(0, 1, msg.c_str());
        int ret = lime::downloadImageResource(image);
        if (ret != 0)
            return ret; //error set by download call
        if (callback)
            callback(1, 1, "Done!");
    }

    if (!force && gwVersion == updateGW && gwRevision == updateGWr)
    {
        lime::info("Existing gateware is same as update (%d.%d)", updateGW, updateGWr);
    }
    else        //load gateware into flash
    {
        if (callback)
            callback(0, 1, image.c_str());
        //open file
        std::ifstream file;
        const auto path = lime::locateImageResource(image);
        file.open(path.c_str(), std::ios::in | std::ios::binary);
        if (not file.good()) return lime::ReportError("Error opening %s", path.c_str());
        //read file
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> progData(fileSize, 0);
        file.read(progData.data(), fileSize);

        int device = 2; //FPGA
        int progMode = 1; //FLASH
        auto status = this->ProgramWrite(progData.data(), progData.size(), progMode, device, callback);
        if (status != 0)
            return status;
        if (callback)
            callback(1, 1, "Done!");
    }

    return 0;
}



