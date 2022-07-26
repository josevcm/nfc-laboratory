/**
    @file IConnection.h
    @author Lime Microsystems
    @brief Interface class for connection types
*/

#ifndef ICONNECTION_H
#define ICONNECTION_H

#include "LimeSuiteConfig.h"
#include "ConnectionHandle.h"
#include <string>
#include <vector>
#include <cstring> //memset
#include <functional>
#include <stdint.h>

namespace lime{

/*!
 * Information about the set of available hardware on a device.
 * This includes available ICs, streamers, and version info.
 *
 * This structure provides SPI slave addresses for one or more RFICs
 * and slave addresses or I2C addresses for commonly supported ICs.
 * A -1 for an address number indicates that it is not available.
 */
struct LIME_API DeviceInfo
{
    DeviceInfo(void){};

    //! The displayable name for the device
    std::string deviceName;

    /*! The displayable name for the expansion card
     * Ex: if the RFIC is on a daughter-card
     */
    std::string expansionName;

    //! The firmware version as a string
    std::string firmwareVersion;

    //! Gateware version as a string
    std::string gatewareVersion;
    //! Gateware revision as a string
    std::string gatewareRevision;
    //! Which board should use this gateware
    std::string gatewareTargetBoard;

    //! The hardware version as a string
    std::string hardwareVersion;

    //! The protocol version as a string
    std::string protocolVersion;

    //! A unique board serial number
    uint64_t boardSerialNumber;
};

/*!
 * IConnection is the interface class for a device with 1 or more Lime RFICs.
 * The LMS7002M driver class calls into IConnection to interface with the hardware
 * to implement high level functions on top of low-level SPI and GPIO.
 * Device developers will implement a custom IConnection for their hardware
 * as an abstraction for streaming and low-level SPI and configuration access.
 */
class LIME_API IConnection
{
public:

    //! IConnection constructor
    IConnection(void);

    //! IConnection destructor
    virtual ~IConnection(void);

    //! Get the connection handle that was used to create this connection
    const ConnectionHandle &GetHandle(void) const;

    /*!
     * Is this connection open?
     * The constructor should attempt to connect but may fail,
     * or the connection may go down at a later time.
     * @return true when the connection is available
     */
    virtual bool IsOpen(void);

    /*!
     * Get information about a device
     * for displaying helpful information
     * or for making device-specific decisions.
     */
    virtual DeviceInfo GetDeviceInfo(void);

    /***********************************************************************
     * Serial API
     **********************************************************************/

    /*!
     * @brief Bulk SPI write/read transaction.
     *
     * The transactSPI function is capable of bulk writes and bulk reads
     * of SPI registers in an arbitrary IC (up to 32-bits per transaction).
     *
     * The readData parameter may be NULL to indicate a write-only operation,
     * the underlying implementation may be able to optimize out the readback.
     *
     * @param addr the SPI device address
     * @param writeData SPI bits to write out
     * @param [out] readData stores readback data
     * @param size the number of SPI transactions
     * @return the transaction success state
     */
    virtual int TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size);

    virtual int WriteLMS7002MSPI(const uint32_t *writeData, size_t size,unsigned periphID = 0)=0;
    virtual int ReadLMS7002MSPI(const uint32_t *writeData, uint32_t *readData, size_t size, unsigned periphID = 0)=0;

    /*!
     * Write to an available I2C slave.
     * @param addr the address of the slave
     * @param data an array of bytes write out
     * @return the transaction success state
     */
    virtual int WriteI2C(const int addr, const std::string &data);

    /*!
     * Read from an available I2C slave.
     * The data parameter can be used to pass optional write data.
     * Some implementations can combine a write + read transaction.
     * If the device contains multiple I2C masters,
     * the address bits can encode which master.
     * \param addr the address of the slave
     * \param numBytes the number of bytes to read
     * \param [inout] data an array of bytes read from the slave
     * @return the transaction success state
     */
    virtual int ReadI2C(const int addr, const size_t numBytes, std::string &data);

    /***********************************************************************
     * LMS7002M Driver callbacks
     **********************************************************************/

    /*!
     * Perform reset sequence on the device.
     * Typically this will reset the RFIC using a GPIO,
     * and possibly other ICs located on the device.
     */
    virtual int DeviceReset(int ind=0);

    /***********************************************************************
     * Stream API
     **********************************************************************/

    /**	@brief Read raw stream data from device streaming port
    @param buffer       read buffer pointer
    @param length       number of bytes to read
    @param epIndex      endpoint identifier?
    @param timeout_ms   timeout in milliseconds
    */
    virtual int ResetStreamBuffers();
    virtual int GetBuffersCount()const;
    virtual int CheckStreamSize(int size)const;
    virtual int ReceiveData(char* buffer, int length, int epIndex, int timeout = 100);
    virtual int SendData(const char* buffer, int length, int epIndex, int timeout = 100);
    
    virtual int BeginDataSending(const char* buffer, uint32_t length, int ep);
    virtual bool WaitForSending(int contextHandle, uint32_t timeout_ms);
    virtual int FinishDataSending(const char* buffer, uint32_t length, int contextHandle);
    virtual void AbortSending(int ep){};
    
    virtual int BeginDataReading(char* buffer, uint32_t length, int ep);
    virtual bool WaitForReading(int contextHandle, unsigned int timeout_ms);
    virtual int FinishDataReading(char* buffer, uint32_t length, int contextHandle);
    virtual void AbortReading(int ep){};
    
    /***********************************************************************
     * Programming API
     **********************************************************************/

    /*!
     * Callback from programming processes
     * @param bsent number of bytes transferred
     * @param btotal total number of bytes to send
     * @param progressMsg string describing current progress state
     * @return 0-continue programming, 1-abort operation
     */
    typedef std::function<bool(int bsent, int btotal, const char* progressMsg)> ProgrammingCallback;

    /** @brief Uploads program to selected device
        @param buffer binary program data
        @param length buffer length
        @param programmingMode to RAM, to FLASH, to EEPROM, etc..
        @param index target device number
        @param callback callback for progress reporting or early termination
        @return the operation success state

        Can be used to program MCU, FPGA, write external on board memory.
        This could be a quite long operation, use callback to get progress info or to terminate early
    */
    virtual int ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int index, ProgrammingCallback callback = 0);

    enum MCU_PROG_MODE
    {
        RESET = 0,
        EEPROM_AND_SRAM,
        SRAM,
        BOOT_SRAM_FROM_EEPROM
    };
    /** @brief Uploads program to MCU
        @param buffer binary program data
        @param length buffer length
        @param mode MCU programing mode RESET, EEPROM_AND_SRAM, SRAM, BOOT_SRAM_FROM_EEPROM
        @param callback callback for progress reporting or early termination
        @return 0-success

        This could be a quite long operation, use callback to get progress info or to terminate early
    */
    virtual int ProgramMCU(const uint8_t *buffer, const size_t length, const MCU_PROG_MODE mode, ProgrammingCallback callback = 0);

    /*!
     * Download up-to-date images files and flash the board when applicable.
     *
     * Program update is an implementation specific function that may handle
     * several different use cases depending upon what options are applicable:
     *
     * - If a board relies on certain firmware and gateware images,
     *   these images will be downloaded if not present on the system.
     *
     * - If the board has a programmable flash for firmware and gateware,
     *   then up-to-date images will be written to the flash on the board.
     *
     * @param download true to enable downloading missing images
     * @param force true to flash even if new image is the same or older version
     * @param callback callback for progress reporting or early termination
     * @return 0-success
     */
    virtual int ProgramUpdate(const bool download = true, const bool force = true, ProgrammingCallback callback = 0);

    /***********************************************************************
     * GPIO API
     **********************************************************************/

    /**	@brief Writes GPIO values to device
    @param buffer for source of GPIO values LSB first, each bit sets GPIO state
    @param bufLength buffer length
    @return the operation success state
    */
    virtual int GPIOWrite(const uint8_t *buffer, const size_t bufLength);

    /**	@brief Reads GPIO values from device
    @param buffer destination for GPIO values LSB first, each bit represent GPIO state
    @param bufLength buffer length to read
    @return the operation success state
    */
    virtual int GPIORead(uint8_t *buffer, const size_t bufLength);

    /**	@brief Write GPIO direction control values to device.
    @param buffer with GPIO direction configuration (0 input, 1 output)
    @param bufLength buffer length
    @return the operation success state
    */
    virtual int GPIODirWrite(const uint8_t *buffer, const size_t bufLength);

    /**	@brief Read GPIO direction configuration from device
    @param buffer to put GPIO direction configuration (0 input, 1 output)
    @param bufLength buffer length to read
    @return the operation success state
    */
    virtual int GPIODirRead(uint8_t *buffer, const size_t bufLength);

    /***********************************************************************
     * Register API
     **********************************************************************/

    /**	@brief Bulk write device registers.
     * WriteRegisters() writes multiple registers and supports 32-bit addresses and data.
     * WriteRegisters() can support multiple devices by dispatching based on the address.
     * @param addrs an array of 32-bit register addresses
     * @param data an array of 32-bit register data
     * @param size the number of entries in addrs and data
     * @return the operation success state
     */
    virtual int WriteRegisters(const uint32_t *addrs, const uint32_t *data, const size_t size);

    //! Write a single device register
    int WriteRegister(const uint32_t addr, const uint32_t data);

    /**	@brief Bulk read device registers.
     * ReadRegisters() writes multiple registers and supports 32-bit addresses and data.
     * ReadRegisters() can support multiple devices by dispatching based on the address.
     * @param addrs an array of 32-bit register addresses
     * @param [out] data an array of 32-bit register data
     * @param size the number of entries in addrs and data
     * @return the operation success state
     */
    virtual int ReadRegisters(const uint32_t *addrs, uint32_t *data, const size_t size);

    //! Read a single device register
    template <typename ReadType>
    int ReadRegister(const uint32_t addr, ReadType &data);

    /***********************************************************************
     * Aribtrary settings API
     **********************************************************************/

    /** @brief Sets custom on board control to given value units
	@param ids indexes of selected controls
	@param values new control values
	@param count number of values to write
	@param units (optional) when not null specifies value units (e.g V, A, Ohm, C... )
	@return the operation success state
    */
    virtual int CustomParameterWrite(const uint8_t *ids, const double *values, const size_t count, const std::string& units);

    /** @brief Returns value of custom on board control
	@param ids indexes of controls to read
	@param values retrieved control values
	@param count number of values to read
	@param units (optional) when not null returns value units (e.g V, A, Ohm, C... )
	@return the operation success state
    */
    virtual int CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units);

    /** @brief Sets callback function which gets called each time data is sent or received
    */
    void SetDataLogCallback(std::function<void(bool, const unsigned char*, const unsigned int)> callback);
protected:
    std::function<void(bool, const unsigned char*, const unsigned int)> callback_logData;

private:
    friend class ConnectionRegistry;
    ConnectionHandle _handle;
};

template <typename ReadType>
int IConnection::ReadRegister(const uint32_t addr, ReadType &data)
{
    uint32_t data32 = 0;
    int st = this->ReadRegisters(&addr, &data32, 1);
    data = ReadType(data32);
    return st;
}
}
#endif

