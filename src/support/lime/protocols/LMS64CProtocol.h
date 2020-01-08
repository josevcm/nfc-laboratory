/**
    @file LMS64CProtocol.h
    @author Lime Microsystems
    @brief Implementation of LMS64C protocol.
*/

#pragma once

#include <mutex>
#include <thread>

#include "ConnectionRegistry/IConnection.h"
#include "protocols/LMS64CCommands.h"
#include "protocols/LMSBoards.h"

namespace lime{

/*!
 * Implement the LMS64CProtocol.
 * The LMS64CProtocol is an IConnection that implements
 * configuration and spi access over the LMS64C Protocol.
 * Connections using LMS64C may inherit from LMS64C.
 */
class LIME_API LMS64CProtocol : public virtual IConnection
{
public:
    LMS64CProtocol(void);

    virtual ~LMS64CProtocol(void);

    virtual DeviceInfo GetDeviceInfo(void);

    //! DeviceReset implemented by LMS64C
    int DeviceReset(int ind=0);

    //! TransactSPI implemented by LMS64C
    int TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)override;

    //! WriteI2C implemented by LMS64C
    int WriteI2C(const int addr, const std::string &data);

    //! ReadI2C implemented by LMS64C
    int ReadI2C(const int addr, const size_t numBytes, std::string &data);

    //! WriteRegisters (BRDSPI) implemented by LMS64C
    int WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size);

    //! ReadRegisters (BRDSPI) implemented by LMS64C
    int ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size);

    /// Supported connection types.
    enum eConnectionType
    {
        CONNECTION_UNDEFINED = -1,
        COM_PORT = 0,
        USB_PORT = 1,
        SPI_PORT = 2,
        PCIE_PORT = 3,
        //insert new types here
        CONNECTION_TYPES_COUNT //used only for memory allocation
    };

    enum eLMS_PROTOCOL
    {
        LMS_PROTOCOL_UNDEFINED = 0,
        LMS_PROTOCOL_DIGIC,
        LMS_PROTOCOL_LMS64C,
        LMS_PROTOCOL_NOVENA,
    };

    struct GenericPacket
    {
        GenericPacket()
        {
            cmd = CMD_GET_INFO;
            status = STATUS_UNDEFINED;
            periphID = 0;
        }

        eCMD_LMS cmd;
        eCMD_STATUS status;
        unsigned periphID;
        std::vector<unsigned char> outBuffer;
        std::vector<unsigned char> inBuffer;
    };

    struct ProtocolDIGIC
    {
        static const int pktLength = 64;
        static const int maxDataLength = 60;
        ProtocolDIGIC() : cmd(0), i2cAddr(0), blockCount(0) {};
        unsigned char cmd;
        unsigned char i2cAddr;
        unsigned char blockCount;
        unsigned char reserved;
        unsigned char data[maxDataLength];
    };

    struct ProtocolLMS64C
    {
        static const int pktLength = 64;
        static const int maxDataLength = 56;
        ProtocolLMS64C() :cmd(0),status(STATUS_UNDEFINED),blockCount(0)
        {
             memset(reserved, 0, 4);
        };
        unsigned char cmd;
        unsigned char status;
        unsigned char blockCount;
        unsigned char periphID;
        unsigned char reserved[4];
        unsigned char data[maxDataLength];
    };

    struct ProtocolNovena
    {
        static const int pktLength = 128;
        static const int maxDataLength = 128;
        ProtocolNovena() :cmd(0),status(0) {};
        unsigned char cmd;
        unsigned char status;
        unsigned char blockCount;
        unsigned char data[maxDataLength];
    };

    /*!
     * Transfer a packet over the underlying transport layer.
     * TransferPacket performs a request/response
     * using the GenericPacket data structure.
     * Some implementations will cast to LMS64CProtocol
     * and directly use the TransferPacket() API call.
     */
    virtual int TransferPacket(GenericPacket &pkt);

    struct LMSinfo
    {
        eLMS_DEV device;
        eEXP_BOARD expansion;
        int firmware;
        int hardware;
        int protocol;
        uint64_t boardSerialNumber;
    };

    LMSinfo GetInfo();

    struct FPGAinfo
    {
        int boardID;
        int gatewareVersion;
        int gatewareRevision;
        int hwVersion;
    };

    FPGAinfo GetFPGAInfo();
    void VersionCheck();
    int ProgramUpdate(const bool download, const bool force, IConnection::ProgrammingCallback callback) override;

    //! implement in base class
    virtual eConnectionType GetType(void) = 0;

    //! virtual write function to be implemented by the base class
    virtual int Write(const unsigned char *buffer, int length, int timeout_ms = 100) = 0;

    //! virtual read function to be implemented by the base class
    virtual int Read(unsigned char *buffer, int length, int timeout_ms = 100) = 0;

    enum ProgramWriteTarget
    {
        HPM,
        FX3, //
        FPGA, // prog_modes: 0-bitstream to FPGA, 1-to FLASH, 2-bitstream from FLASH

        PROGRAM_WRITE_TARGET_COUNT
    };

    virtual int ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int device, ProgrammingCallback callback = nullptr);

    virtual int CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units) override;
    virtual int CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units) override;

    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength) override;
    virtual int GPIODirRead(uint8_t *buffer, const size_t bufLength) override;

    int ProgramMCU(const uint8_t *buffer, const size_t length, const MCU_PROG_MODE mode, ProgrammingCallback callback) override;
    int WriteLMS7002MSPI(const uint32_t *writeData, size_t size,unsigned periphID = 0) override;
    int ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID = 0) override;
protected:
#ifdef REMOTE_CONTROL
    void InitRemote();
    void CloseRemote();
    void ProcessConnections();
    bool remoteOpen;
    int socketFd;
    std::thread remoteThread;
#endif
private:
    int WriteSi5351I2C(const std::string &data);
    int ReadSi5351I2C(const size_t numBytes, std::string &data);

    int WriteADF4002SPI(const uint32_t *writeData, const size_t size);
    int ReadADF4002SPI(const uint32_t *writeData, uint32_t *readData, const size_t size);

    unsigned char* PreparePacket(const GenericPacket &pkt, int &length, const eLMS_PROTOCOL protocol);
    int ParsePacket(GenericPacket &pkt, const unsigned char* buffer, const int length, const eLMS_PROTOCOL protocol);
    std::mutex mControlPortLock;
    double _cachedRefClockRate;
};
}
