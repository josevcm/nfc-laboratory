/**
    @file ConnectionSTREAM.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionFX3.h"
#include <cstring>
#include "Si5351C.h"
#include "FPGA_common.h"
#include "LMS7002M.h"
#include "Logger.h"
#include <ciso646>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

#define CTR_W_REQCODE 0xC1
#define CTR_W_VALUE 0x0000
#define CTR_W_INDEX 0x0000

#define CTR_R_REQCODE 0xC0
#define CTR_R_VALUE 0x0000
#define CTR_R_INDEX 0x0000

using namespace lime;

const uint8_t ConnectionFX3::ctrlBulkOutAddr = 0x0F;
const uint8_t ConnectionFX3::ctrlBulkInAddr = 0x8F;

//control commands to be send via bulk port for boards v1.1 and earlier
const std::set<uint8_t> ConnectionFX3::commandsToBulkCtrlHw1 =
{
    CMD_BRDSPI_WR, CMD_BRDSPI_RD,
    CMD_LMS7002_WR, CMD_LMS7002_RD,
    CMD_LMS7002_RST,
};
//control commands to be send via bulk port for boards v1.2 and later
const std::set<uint8_t> ConnectionFX3::commandsToBulkCtrlHw2 =
{
    CMD_BRDSPI_WR, CMD_BRDSPI_RD,
    CMD_LMS7002_WR, CMD_LMS7002_RD,
    CMD_ANALOG_VAL_WR, CMD_ANALOG_VAL_RD,
    CMD_ADF4002_WR,
    CMD_LMS7002_RST,
    CMD_GPIO_DIR_WR, CMD_GPIO_DIR_RD,
    CMD_GPIO_WR, CMD_GPIO_RD,
};

/**	@brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionFX3::ConnectionFX3(void *arg, const std::string &vidpid, const std::string &serial, const unsigned index)
    : contexts(nullptr), contextsToSend(nullptr)
{
    bulkCtrlAvailable = false;
    bulkCtrlInProgress = false;
    isConnected = false;
#ifndef __unix__
    if(arg == nullptr)
        USBDevicePrimary = new CCyFX3Device();
    else
        USBDevicePrimary = new CCyFX3Device(*(CCyFX3Device*)arg);
	InCtrlEndPt3 = nullptr;
	OutCtrlEndPt3 = nullptr;
	InCtrlBulkEndPt = nullptr;
	OutCtrlBulkEndPt = nullptr;
    for (int i = 0; i < MAX_EP_CNT; i++)
        InEndPt[i] = OutEndPt[i] = nullptr;

#else
    dev_handle = nullptr;
    ctx = (libusb_context *)arg;
#endif
    if (this->Open(vidpid, serial, index) != 0)
        lime::error("Failed to open device");

    commandsToBulkCtrl = commandsToBulkCtrlHw2;

    LMSinfo info = this->GetInfo();

    if (info.device == LMS_DEV_LIMESDR && info.hardware <= 1)
        commandsToBulkCtrl = commandsToBulkCtrlHw1;

    this->VersionCheck();

    //must configure synthesizer before using LimeSDR
    if (info.device == LMS_DEV_LIMESDR && info.hardware < 4)
    {
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
            lime::warning("Failed to upload Si5351C configuration");
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //some settle time
    }
}

/**	@brief Closes connection to chip and deallocates used memory.
*/
ConnectionFX3::~ConnectionFX3()
{
    Close();
#ifndef __unix__
    delete USBDevicePrimary;
#endif
}

/**	@brief Tries to open connected USB device and find communication endpoints.
	@return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionFX3::Open(const std::string &vidpid, const std::string &serial, const unsigned index)
{
    Close();
    bulkCtrlAvailable = false;
#ifndef __unix__
    if(index > USBDevicePrimary->DeviceCount())
        return ReportError(ERANGE, "ConnectionSTREAM: Device index out of range");

    if(USBDevicePrimary->Open(index) == false)
        return ReportError(-1, "ConnectionSTREAM: Failed to open device");

    if (InCtrlEndPt3)
    {
        delete InCtrlEndPt3;
        InCtrlEndPt3 = nullptr;
    }
    InCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

    if (OutCtrlEndPt3)
    {
        delete OutCtrlEndPt3;
        OutCtrlEndPt3 = nullptr;
    }
    OutCtrlEndPt3 = new CCyControlEndPoint(*USBDevicePrimary->ControlEndPt);

    InCtrlEndPt3->ReqCode = CTR_R_REQCODE;
    InCtrlEndPt3->Value = CTR_R_VALUE;
    InCtrlEndPt3->Index = CTR_R_INDEX;
    InCtrlEndPt3->TimeOut = 3000;

    OutCtrlEndPt3->ReqCode = CTR_W_REQCODE;
    OutCtrlEndPt3->Value = CTR_W_VALUE;
    OutCtrlEndPt3->Index = CTR_W_INDEX;
    OutCtrlEndPt3->TimeOut = 3000;

    for (int i = 0; i < USBDevicePrimary->EndPointCount(); i++)
    {
        auto adr = USBDevicePrimary->EndPoints[i]->Address;
        if (adr < ctrlBulkOutAddr)
        {
            OutEndPt[adr] = USBDevicePrimary->EndPoints[i];
            long len = OutEndPt[adr]->MaxPktSize * 64;
            OutEndPt[adr]->SetXferSize(len);
        }
        else if (adr < ctrlBulkInAddr)
        {
            adr &= 0xF;
            InEndPt[adr] = USBDevicePrimary->EndPoints[i];
            long len = InEndPt[adr]->MaxPktSize * 64;
            InEndPt[adr]->SetXferSize(len);
        }
    }

    InCtrlBulkEndPt = nullptr;
    for (int i=0; i<USBDevicePrimary->EndPointCount(); i++)
        if(USBDevicePrimary->EndPoints[i]->Address == ctrlBulkInAddr)
        {
            InCtrlBulkEndPt = USBDevicePrimary->EndPoints[i];
            InCtrlBulkEndPt->TimeOut = 1000;
            bulkCtrlAvailable = true;
            break;
        }
    OutCtrlBulkEndPt = nullptr;
    for (int i=0; i<USBDevicePrimary->EndPointCount(); i++)
        if(USBDevicePrimary->EndPoints[i]->Address == ctrlBulkOutAddr)
        {
            OutCtrlBulkEndPt = USBDevicePrimary->EndPoints[i];
            OutCtrlBulkEndPt->TimeOut = 1000;
            bulkCtrlAvailable = true;
            break;
        }
#else
    const auto splitPos = vidpid.find(":");
    const auto vid = std::stoi(vidpid.substr(0, splitPos), nullptr, 16);
    const auto pid = std::stoi(vidpid.substr(splitPos+1), nullptr, 16);

    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    int usbDeviceCount = libusb_get_device_list(ctx, &devs);

    if (usbDeviceCount < 0) {
        return ReportError(-1, "libusb_get_device_list failed: %s", libusb_strerror(libusb_error(usbDeviceCount)));
    }

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
        return ReportError(-1, "libusb_open failed");
    if(libusb_kernel_driver_active(dev_handle, 0) == 1)   //find out if kernel driver is attached
    {
        lime::info("Kernel Driver Active");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            lime::info("Kernel Driver Detached!");
    }
    int r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device
    if(r < 0)
        return ReportError(-1, "Cannot claim interface - %s", libusb_strerror(libusb_error(r)));

    libusb_device* device = libusb_get_device(dev_handle);
    libusb_config_descriptor* descriptor = nullptr;
    if(libusb_get_active_config_descriptor(device, &descriptor) < 0)
    {
        lime::error("failed to get config descriptor");
    }
    //check for 0x0F and 0x8F endpoints
    if(descriptor->bNumInterfaces > 0)
    {
        libusb_interface_descriptor iface = descriptor->interface[0].altsetting[0];
        for(int i=0; i<iface.bNumEndpoints; ++i)
            if(iface.endpoint[i].bEndpointAddress == ctrlBulkOutAddr ||
               iface.endpoint[i].bEndpointAddress == ctrlBulkInAddr)
            {
                bulkCtrlAvailable = true;
                break;
            }
    }
    libusb_free_config_descriptor(descriptor);
#endif
    isConnected = true;
    contexts = new USBTransferContext[USB_MAX_CONTEXTS];
    contextsToSend = new USBTransferContext[USB_MAX_CONTEXTS];
    if(bulkCtrlAvailable)
    {
        LMS64CProtocol::GenericPacket ctrPkt;
        ctrPkt.cmd = CMD_USB_FIFO_RST;
        ctrPkt.outBuffer.push_back(0x01); //reset bulk endpoints
        if(TransferPacket(ctrPkt) != 0)
            lime::error("Failed to reset USB bulk endpoints");
    }
    return 0;
}
/**	@brief Closes communication to device.
*/
void ConnectionFX3::Close()
{
#ifdef __unix__
    const libusb_version* ver = libusb_get_version();
    // Fix #358 libusb crash when freeing transfers(never used ones) without valid device handle. Bug in libusb 1.0.25 https://github.com/libusb/libusb/issues/1059
    const bool isBuggy_libusb_free_transfer = ver->major==1 && ver->minor==0 && ver->micro == 25;
    if(isBuggy_libusb_free_transfer)
    {
        if(contexts)
            for(int i=0; i<USB_MAX_CONTEXTS; ++i)
                contexts[i].transfer->dev_handle = dev_handle;
        if(contextsToSend)
            for(int i=0; i<USB_MAX_CONTEXTS; ++i)
                contextsToSend[i].transfer->dev_handle = dev_handle;
    }
#endif

    if(contexts)
    {
        delete[] contexts;
        contexts = nullptr;
    }
    if(contextsToSend)
    {
        delete[] contextsToSend;
        contextsToSend = nullptr;
    }

    #ifndef __unix__
    USBDevicePrimary->Close();
    for (int i = 0; i < MAX_EP_CNT; i++)
        InEndPt[i] = OutEndPt[i] = nullptr;
    InCtrlBulkEndPt = nullptr;
    OutCtrlBulkEndPt = nullptr;
    if (InCtrlEndPt3)
    {
        delete InCtrlEndPt3;
        InCtrlEndPt3 = nullptr;
    }
    if (OutCtrlEndPt3)
    {
        delete OutCtrlEndPt3;
        OutCtrlEndPt3 = nullptr;
    }
    #else
    if(dev_handle != 0)
    {
        libusb_release_interface(dev_handle, 0);
        libusb_close(dev_handle);
        dev_handle = 0;
    }
    #endif
    isConnected = false;
}

/**	@brief Returns connection status
	@return 1-connection open, 0-connection closed.
*/
bool ConnectionFX3::IsOpen()
{
    #ifndef __unix__
    return USBDevicePrimary->IsOpen() && isConnected;
    #else
    return isConnected;
    #endif
}

/**	@brief Sends given data buffer to chip through USB port.
	@param buffer data buffer, must not be longer than 64 bytes.
	@param length given buffer size.
    @param timeout_ms timeout limit for operation in milliseconds
	@return number of bytes sent.
*/
int ConnectionFX3::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = length;
    if(IsOpen() == false)
        return 0;

    unsigned char* wbuffer = new unsigned char[length];
    memcpy(wbuffer, buffer, length);
    bulkCtrlInProgress = false;
    #ifndef __unix__
    if(bulkCtrlAvailable
        && commandsToBulkCtrl.find(buffer[0]) != commandsToBulkCtrl.end())
    {
        bulkCtrlInProgress = true;
        OutCtrlBulkEndPt->XferData(wbuffer, len);
    }
    else if(OutCtrlEndPt3)
        OutCtrlEndPt3->Write(wbuffer, len);
    else
        len = 0;
    #else
    if(bulkCtrlAvailable
        && commandsToBulkCtrl.find(buffer[0]) != commandsToBulkCtrl.end())
    {
        bulkCtrlInProgress = true;
        int actual = 0;
        libusb_bulk_transfer(dev_handle, ctrlBulkOutAddr, wbuffer, length, &actual, timeout_ms);
        len = actual;
    }
    else
        len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR,CTR_W_REQCODE ,CTR_W_VALUE, CTR_W_INDEX, wbuffer, length, timeout_ms);
    #endif
    delete[] wbuffer;
    return len;
}

/**	@brief Reads data coming from the chip through USB port.
	@param buffer pointer to array where received data will be copied, array must be
	big enough to fit received data.
	@param length number of bytes to read from chip.
    @param timeout_ms timeout limit for operation in milliseconds
	@return number of bytes received.
*/
int ConnectionFX3::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mExtraUsbMutex);
    long len = length;
    if(IsOpen() == false)
        return 0;

#ifndef __unix__
    if(bulkCtrlAvailable && bulkCtrlInProgress)
    {
        InCtrlBulkEndPt->XferData(buffer, len);
        bulkCtrlInProgress = false;
    }
    else if(InCtrlEndPt3)
        InCtrlEndPt3->Read(buffer, len);
    else
        len = 0;
#else
    if(bulkCtrlAvailable && bulkCtrlInProgress)
    {
        int actual = 0;
        int r = libusb_bulk_transfer(dev_handle, ctrlBulkInAddr, buffer, len, &actual, timeout_ms);
        if (r == LIBUSB_ERROR_TIMEOUT) {
            /* fix a bug in the kernel with AMD chipsets -- see myriadrf/LimeSuite#287 */
            libusb_bulk_transfer(dev_handle, ctrlBulkInAddr, buffer, len, &actual, timeout_ms);
        }
        len = actual;
        bulkCtrlInProgress = false;
    }
    else
        len = libusb_control_transfer(dev_handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN ,CTR_R_REQCODE ,CTR_R_VALUE, CTR_R_INDEX, buffer, len, timeout_ms);
#endif
    return len;
}

#ifdef __unix__
/**	@brief Function for handling libusb callbacks
*/
void callback_libusbtransfer(libusb_transfer *trans)
{
	USBTransferContext *context = reinterpret_cast<USBTransferContext*>(trans->user_data);
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
        lime::error("USB TRANSFER ERROR");
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        break;
    case LIBUSB_TRANSFER_TIMED_OUT:
        context->bytesXfered = trans->actual_length;
        context->done.store(true);
        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        lime::error("USB transfer overflow");
        break;
    case LIBUSB_TRANSFER_STALL:
        lime::error("USB transfer stalled");
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        lime::error("USB transfer no device");
        break;
	}
	lck.unlock();
	context->cv.notify_one();
}
#endif

/**
	@brief Starts asynchronous data reading from board
	@param *buffer buffer where to store received data
	@param length number of bytes to read
	@param streamBulkInAddr endpoint index?
	@return handle of transfer context
*/
int ConnectionFX3::BeginDataReading(char *buffer, uint32_t length, int ep)
{
    const unsigned char streamBulkInAddr = 0x81;
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
    if (InEndPt[streamBulkInAddr & 0xF])
    {
        contexts[i].EndPt = InEndPt[streamBulkInAddr & 0xF];
        contexts[i].context = contexts[i].EndPt->BeginDataXfer((unsigned char*)buffer, length, contexts[i].inOvLap);
    }
	return i;
    #else
    libusb_transfer *tr = contexts[i].transfer;
    libusb_fill_bulk_transfer(tr, dev_handle, streamBulkInAddr, (unsigned char*)buffer, length, callback_libusbtransfer, &contexts[i], 0);
    contexts[i].done = false;
    contexts[i].bytesXfered = 0;
    int status = libusb_submit_transfer(tr);
    if(status != 0)
    {
        lime::error("BEGIN DATA READING %s", libusb_error_name(status));
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
@return  true - wait finished, false - still waiting for transfer to complete
*/
bool ConnectionFX3::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
    #ifndef __unix__
    int status = 0;
    status = contexts[contextHandle].EndPt->WaitForXfer(contexts[contextHandle].inOvLap, timeout_ms);
	return status;
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
	@return negative values failure, positive number of bytes received
*/
int ConnectionFX3::FinishDataReading(char *buffer, uint32_t length, int contextHandle)
{
    if(contextHandle >= 0 && contexts[contextHandle].used == true)
    {
    #ifndef __unix__
    int status = 0;
    long len = length;
    status = contexts[contextHandle].EndPt->FinishDataXfer((unsigned char*)buffer, len, contexts[contextHandle].inOvLap, contexts[contextHandle].context);
    contexts[contextHandle].used = false;
    contexts[contextHandle].reset();
    return len;
    #else
	length = contexts[contextHandle].bytesXfered;
	contexts[contextHandle].used = false;
	contexts[contextHandle].reset();
	return length;
    #endif
    }
    else
        return 0;
}

/**
	@brief Aborts reading operations
*/
void ConnectionFX3::AbortReading(int ep)
{
#ifndef __unix__
    for (int i = 0; i < MAX_EP_CNT; i++)
        if (InEndPt[i] && InEndPt[i]->Address == 0x81)
	        InEndPt[i]->Abort();
#else
    for(int i=0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contexts[i].used && contexts[i].transfer->endpoint == 0x81)
            libusb_cancel_transfer( contexts[i].transfer );
    }
#endif
    for(int i=0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contexts[i].used)
        {
            WaitForReading(i, 250);
            FinishDataReading(nullptr, 0, i);
        }
    }
}

/**
	@brief Starts asynchronous data Sending to board
	@param *buffer buffer to send
	@param length number of bytes to send
	@param streamBulkOutAddr endpoint index?
	@return handle of transfer context
*/
int ConnectionFX3::BeginDataSending(const char *buffer, uint32_t length, int ep)
{
    const unsigned char streamBulkOutAddr = 0x01;
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
    if (OutEndPt[streamBulkOutAddr])
    {
        contextsToSend[i].EndPt = OutEndPt[streamBulkOutAddr];
        contextsToSend[i].context = contextsToSend[i].EndPt->BeginDataXfer((unsigned char*)buffer, length, contextsToSend[i].inOvLap);
    }
	return i;
    #else
    libusb_transfer *tr = contextsToSend[i].transfer;
    contextsToSend[i].done = false;
    contextsToSend[i].bytesXfered = 0;
    libusb_fill_bulk_transfer(tr, dev_handle, streamBulkOutAddr, (unsigned char*)buffer, length, callback_libusbtransfer, &contextsToSend[i], 0);
    int status = libusb_submit_transfer(tr);
    if(status != 0)
    {
        lime::error("BEGIN DATA SENDING %s", libusb_error_name(status));
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
bool ConnectionFX3::WaitForSending(int contextHandle, unsigned int timeout_ms)
{
    if(contextHandle >= 0 && contextsToSend[contextHandle].used == true )
    {
#   ifndef __unix__
	int status = 0;
	status = contextsToSend[contextHandle].EndPt->WaitForXfer(contextsToSend[contextHandle].inOvLap, timeout_ms);
	return status;
#   else
    //blocking not to waste CPU
    std::unique_lock<std::mutex> lck(contextsToSend[contextHandle].transferLock);
    return contextsToSend[contextHandle].cv.wait_for(lck, chrono::milliseconds(timeout_ms), [&](){return contextsToSend[contextHandle].done.load();});
#   endif
    }
    return true;  //there is nothing to wait for (signal wait finished)
}

/**
	@brief Finishes asynchronous data sending to board
	@param buffer array where to store received data
	@param length number of bytes to read, function changes this value to number of bytes acctually received
	@param contextHandle handle of which context to finish
	@return false failure, true number of bytes sent
*/
int ConnectionFX3::FinishDataSending(const char *buffer, uint32_t length, int contextHandle)
{
    if(contextHandle >= 0 && contextsToSend[contextHandle].used == true)
    {
#ifndef __unix__
        long len = length;
        contextsToSend[contextHandle].EndPt->FinishDataXfer((unsigned char*)buffer, len, contextsToSend[contextHandle].inOvLap, contextsToSend[contextHandle].context);
        contextsToSend[contextHandle].used = false;
        contextsToSend[contextHandle].reset();
        return len;
#else
	length = contextsToSend[contextHandle].bytesXfered;
	contextsToSend[contextHandle].used = false;
        contextsToSend[contextHandle].reset();
	return length;
#endif
    }
    else
        return 0;
}

/**
	@brief Aborts sending operations
*/
void ConnectionFX3::AbortSending(int ep)
{
#ifndef __unix__
    for (int i = 0; i < MAX_EP_CNT; i++)
        if (OutEndPt[i] && OutEndPt[i]->Address == 0x01)
            OutEndPt[i]->Abort();
#else
    for (int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contextsToSend[i].used && contextsToSend[i].transfer->endpoint == 0x01)
            libusb_cancel_transfer(contextsToSend[i].transfer);
    }
#endif
    for (int i = 0; i<USB_MAX_CONTEXTS; ++i)
    {
        if(contextsToSend[i].used)
        {
            WaitForSending(i, 250);
            FinishDataSending(nullptr, 0, i);
        }
    }
}

int ConnectionFX3::GetBuffersCount() const
{
    return USB_MAX_CONTEXTS;
}

int ConnectionFX3::CheckStreamSize(int size)const
{
    return size;
}

int ConnectionFX3::SendData(const char* buffer, int length, int epIndex, int timeout)
{
    const unsigned char ep = 0x01;
    int context = BeginDataSending((char*)buffer, length, ep);
    if (WaitForSending(context, timeout)==false)
        AbortSending(ep);
    return FinishDataSending((char*)buffer, length , context);
}

int ConnectionFX3::ReceiveData(char* buffer, int length, int epIndex, int timeout)
{
    const unsigned char ep = 0x81;
    int context = BeginDataReading(buffer, length, ep);
    if (WaitForReading(context, timeout) == false)
        AbortReading(ep);
    return FinishDataReading(buffer, length, context);
}

int ConnectionFX3::ProgramWrite(const char *buffer, const size_t length, const int programmingMode, const int device, ProgrammingCallback callback)
{
    if (device == LMS64CProtocol::FX3 && programmingMode == 1)
    {
#ifdef __unix__
        libusb_device_descriptor desc;
        int ret = libusb_get_device_descriptor(libusb_get_device(dev_handle), &desc);
        if(ret<0)
            lime::error("failed to get device description");
        else if (desc.idProduct == 243)
#else
        if (USBDevicePrimary->ProductID == 243)
#endif
        {
#ifdef __unix__
            return fx3_usbboot_download((unsigned char*)buffer,length);
#else
            char* filename = "fx3fw_image_tmp.img";
            int ret = 0;
            std::ofstream myfile(filename, ios::out | ios::binary | ios::trunc);
            if (!myfile.is_open())
            {
                ReportError("FX3 FW:Unable to create temporary file");
                return -1;
            }
            myfile.write(buffer,length);
            if (myfile.fail())
            {
                ReportError("FX3 FW:Unable to write to temporary file");
                ret = -1;
            }
            myfile.close();

            if (ret != -1)
            {
                if ((ret=USBDevicePrimary->DownloadFw(filename, FX3_FWDWNLOAD_MEDIA_TYPE::RAM))!=0)
                    ReportError("FX3: Failed to upload FW to RAM");
            }

            std::remove(filename);
            return ret;
#endif
        }
            else
            {
                ReportError("FX3 bootloader NOT detected");
                return -1;
            }
    }
    return LMS64CProtocol::ProgramWrite(buffer,length,programmingMode,device,callback);
}

int ConnectionFX3::ResetStreamBuffers()
{
    //USB FIFO reset
    LMS64CProtocol::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0x00);
    return TransferPacket(ctrPkt);
}

#ifdef __unix__

#define MAX_FWIMG_SIZE  (512 * 1024)		// Maximum size of the firmware binary.
#define GET_LSW(v)	((unsigned short)((v) & 0xFFFF))
#define GET_MSW(v)	((unsigned short)((v) >> 16))

#define VENDORCMD_TIMEOUT	(5000)		// Timeout for each vendor command is set to 5 seconds.


int ConnectionFX3::ram_write(unsigned char *buf, unsigned int ramAddress, int len)
{
    const int MAX_WRITE_SIZE = (2 * 1024);		// Max. size of data that can be written through one vendor command.
	int r;
	int index = 0;
	int size;

	while ( len > 0 )
    {
		size = (len > MAX_WRITE_SIZE) ? MAX_WRITE_SIZE : len;
		r = libusb_control_transfer(dev_handle, 0x40, 0xA0, GET_LSW(ramAddress), GET_MSW(ramAddress),&buf[index], size, VENDORCMD_TIMEOUT);
		if ( r != size )
		{
			lime::error("Vendor write to FX3 RAM failed");
			return -1;
		}
		ramAddress += size;
		index      += size;
		len        -= size;
	}
	return 0;
}

int ConnectionFX3::fx3_usbboot_download(unsigned char *fwBuf, int filesize)
{
	unsigned int  *data_p;
	unsigned int i, checksum;
	unsigned int address, length;
	int r, index;

	if ( filesize > MAX_FWIMG_SIZE ) {
		ReportError("File size exceeds maximum firmware image size\n");
		return -2;
	}

	if ( strncmp((char *)fwBuf,"CY",2) ) {
		ReportError("Image does not have 'CY' at start. aborting\n");
		return -4;
	}

	if ( fwBuf[2] & 0x01 ) {
		ReportError("Image does not contain executable code\n");
		return -5;
	}

	if ( !(fwBuf[3] == 0xB0) ) {
		ReportError("Not a normal FW binary with checksum\n");
		return -6;
	}

	// Run through each section of code, and use vendor commands to download them to RAM.
	index    = 4;
	checksum = 0;
	while ( index < filesize )
	{
		data_p  = (unsigned int *)(fwBuf + index);
		length  = data_p[0];
		address = data_p[1];
		if (length != 0)
		{
			for (i = 0; i < length; i++)
				checksum += data_p[2 + i];
			r = ram_write(fwBuf + index + 8, address, length * 4);
			if (r != 0)
			{
				ReportError("Failed to download data to FX3 RAM\n");
				return -3;
			}
		}
		else
		{
			if (checksum != data_p[2]) {
				ReportError ("Checksum error in firmware binary\n");
				return -4;
			}

			r = libusb_control_transfer(dev_handle, 0x40, 0xA0, GET_LSW(address), GET_MSW(address), NULL,0, VENDORCMD_TIMEOUT);
			if ( r != 0 )
				lime::error("Ignored error in control transfer: %d", r);
			break;
		}
		index += (8 + length * 4);
	}

	return 0;
}
#endif
