/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#include <libusb.h>
#include <unistd.h>

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/Queue.h>

#include <hw/usb/Usb.h>

#include <memory>
#include <utility>

namespace hw {

void transferHandler(struct libusb_transfer *t);

struct TransferInfo
{
   Usb::Impl *device;
   Usb::Transfer *transfer;
   libusb_transfer *usbTransfer;
};

struct UsbContext
{
   rt::Logger *log = rt::Logger::getLogger("hw.UsbContext");

   libusb_context *ctx = nullptr;

   UsbContext()
   {
      if (int result = 0; (result = libusb_init(&ctx)) < 0)
      {
         log->error("error initializing libusb: {}", {libusb_error_name(result)});
      }
   }

   ~UsbContext()
   {
      if (ctx)
         libusb_exit(ctx);
   }

   operator libusb_context *() const
   {
      return ctx;
   }
};

std::shared_ptr<UsbContext> ctx;

struct Usb::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.UsbDevice");

   int result = 0;
   bool shutdown = false;
   std::mutex threadMutex;

   libusb_device_handle *hdl = nullptr;
   libusb_speed speed = LIBUSB_SPEED_UNKNOWN;

   Descriptor descriptor {};

   std::list<TransferInfo *> transfers;

   explicit Impl(Descriptor desc) : descriptor(std::move(desc))
   {
      log->debug("created UsbDevice");

      if (!ctx)
         ctx = std::make_shared<UsbContext>();
   }

   ~Impl()
   {
      log->debug("destroy UsbDevice");

      close();
   }

   bool open()
   {
      libusb_device **devs;

      // ensure device is closed
      close();

      // enumerate usb devices
      if ((result = static_cast<int>(libusb_get_device_list(*ctx, &devs))) < 0)
      {
         log->error("error getting USB device list: {}", {lastError()});
         return false;
      }

      size_t count = result;

      // scan devices to find the one we are looking for
      for (ssize_t i = 0; i < count; i++)
      {
         libusb_device *dev = devs[i];
         libusb_device_descriptor desc {};

         if ((result = libusb_get_device_descriptor(dev, &desc)) != LIBUSB_SUCCESS)
         {
            log->error("failed to get device descriptor: {}", {lastError()});
            continue;
         }

         if (desc.idVendor != descriptor.vid || desc.idProduct != descriptor.pid)
            continue;

         if (libusb_get_bus_number(dev) != descriptor.bus || libusb_get_device_address(dev) != descriptor.address)
            continue;

         if ((result = libusb_get_device_speed(dev)) < 0)
         {
            log->error("failed to get device speed: {}", {lastError()});
            continue;
         }

         speed = static_cast<libusb_speed>(result);

         if ((result = libusb_open(dev, &hdl)) != LIBUSB_SUCCESS)
         {
            log->error("failed to open device: {}", {lastError()});
            return false;
         }

         break;
      }

      log->debug("starting event handling thread");

      shutdown = false;

      std::thread([=]() {

#ifdef _WIN32
         SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#else
         sched_param param {20};
         pthread_setschedparam(pthread_self(), SCHED_RR, &param);
#endif

         std::lock_guard lock(threadMutex);

         timeval timeout = {1, 0};

         log->info("libusb event handling thread running");

         std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

         // loop until shutdown is requested and all transfers are completed
         while (!shutdown || !transfers.empty())
         {
            if (shutdown)
               log->info("waiting for transfers to complete, remaining: {}", {static_cast<int>(transfers.size())});

            // handle libusb events
            if ((result = libusb_handle_events_timeout_completed(*ctx, &timeout, nullptr)) < 0)
            {
               if (result != LIBUSB_ERROR_INTERRUPTED)
                  shutdown = true;
            }
         }

         auto duration = std::chrono::steady_clock::now() - start;

         log->info("libusb event handling thread finished, running time {}", {duration});

      }).detach();

      result = 0;

      return true;
   }

   void close()
   {
      if (!hdl)
         return;

      log->info("stopping event handling thread");

      // signal shutdown to event thread
      shutdown = true;

      // wait for thread to finish
      std::lock_guard lock(threadMutex);

      // closing libusb device
      libusb_close(hdl);

      // invalidate handle
      hdl = nullptr;
   }

   bool claimInterface(int interface)
   {
      switch ((result = libusb_claim_interface(hdl, interface)))
      {
         case LIBUSB_SUCCESS:
            return true;

         case LIBUSB_ERROR_BUSY:
            log->error("unable to claim USB interface. Another program or driver has already claimed it.");
            return false;

         case LIBUSB_ERROR_NO_DEVICE:
            log->error("device has been disconnected.");
            return false;

         case LIBUSB_ERROR_NOT_FOUND:
            log->error("unable to claim interface, try again: LIBUSB_ERROR_NOT_FOUND.");
            return false;

         default:
            log->error("unable to claim interface, try again: {}", {lastError()});
            return false;
      }
   }

   bool releaseInterface(int interface)
   {
      if ((result = libusb_release_interface(hdl, interface)) != LIBUSB_SUCCESS)
      {
         log->error("unable to release interface: {}", {lastError()});
         return false;
      }

      return true;
   }

   bool ctrlTransfer(int outCmd, const void *txData, unsigned int txSize, int inCmd, void *rxData, unsigned int rxSize, int timeout, int wait)
   {
      if (log->isDebugEnabled())
         log->debug("USB CONTROL OUT, size {} bytes\n{}", {txSize, rt::ByteBuffer((unsigned char *)txData, txSize)});

      /* Send the control message. */
      if ((result = libusb_control_transfer(hdl, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, outCmd, 0x0000, 0x0000, (unsigned char *)txData, txSize, timeout)) < 0)
      {
         log->error("unable to send OUT request command {}: {}", {outCmd, lastError()});
         return false;
      }

      // check if except to receive data
      if (rxData)
      {
         usleep(wait * 1000);

         /* Send the control message. */
         if ((result = libusb_control_transfer(hdl, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, inCmd, 0x0000, 0x0000, (unsigned char *)rxData, rxSize, timeout)) < 0)
         {
            log->error("unable to send IN request command {}: {}", {inCmd, lastError()});
            return false;
         }

         if (log->isDebugEnabled())
            log->debug("USB CONTROL IN, size {} bytes\n{}", {rxSize, rt::ByteBuffer((unsigned char *)rxData, rxSize)});
      }

      return true;
   }

   int syncTransfer(int endpoint, void *data, unsigned int length, int timeout)
   {
      int transferred = 0;

      if (!(endpoint & LIBUSB_ENDPOINT_IN) && log->isDebugEnabled())
         log->debug("USB BULK OUT, size {} bytes\n{}", {length, rt::ByteBuffer((unsigned char *)data, length)});

      if ((result = libusb_bulk_transfer(hdl, endpoint, static_cast<unsigned char *>(data), length, &transferred, timeout)) < 0)
      {
         log->error("error in bulk transfer: {}", {lastError()});
         return -1;
      }

      if (endpoint & LIBUSB_ENDPOINT_IN && log->isDebugEnabled())
         log->debug("USB BULK IN, size {} bytes\n{}", {transferred, rt::ByteBuffer((unsigned char *)data, transferred)});

      return transferred;
   }

   int asyncTransfer(int endpoint, Transfer *transfer)
   {
      if (transfer == nullptr)
      {
         log->error("transfer is null");
         return -1;
      }

      libusb_transfer *usbTransfer = libusb_alloc_transfer(0);

      const auto transferInfo = new TransferInfo {this, transfer, usbTransfer};

      libusb_fill_bulk_transfer(usbTransfer, hdl, endpoint, transfer->data, static_cast<int>(transfer->available), transferHandler, transferInfo, transfer->timeout);

      if ((result = libusb_submit_transfer(usbTransfer)) != LIBUSB_SUCCESS)
      {
         libusb_free_transfer(usbTransfer);
         log->error("error in submit async transfer: {}", {lastError()});
         return -1;
      }

      transfers.push_back(transferInfo);

      return 0;
   }

   bool cancelTransfer(const Transfer *transfer)
   {
      if (transfer == nullptr)
      {
         log->error("transfer is null");
         return -1;
      }

      for (auto transferInfo: transfers)
      {
         if (transferInfo->transfer == transfer)
         {
            if ((result = libusb_cancel_transfer(transferInfo->usbTransfer)) != LIBUSB_SUCCESS)
            {
               log->error("error in cancel transfer: {}", {lastError()});
               return false;
            }

            return true;
         }
      }

      return false;
   }

   int setOption(libusb_option option, int value)
   {
      if ((result = libusb_set_option(*ctx, option, value)) < 0)
      {
         log->error("error in set option {}={}: {}", {option, value, lastError()});
         return -1;
      }

      return 0;
   }

   std::string lastError() const
   {
      return errorString(result);
   }

   static std::string errorString(int result)
   {
      return {libusb_error_name(result)};
   }

   void processTransfer(TransferInfo *transferInfo)
   {
      const auto transfer = transferInfo->transfer;
      const auto usbTransfer = transferInfo->usbTransfer;

      if (log->isDebugEnabled())
         log->debug("USB BULK IN, size {} bytes\n{}", {usbTransfer->actual_length, rt::ByteBuffer(usbTransfer->buffer, usbTransfer->actual_length)});

      transfer->actual = usbTransfer->actual_length;
      transfer->timeout = usbTransfer->timeout;

      switch (usbTransfer->status)
      {
         case LIBUSB_TRANSFER_COMPLETED:
            transfer->status = Completed;
            break;

         case LIBUSB_TRANSFER_ERROR:
            transfer->status = Error;
            break;

         case LIBUSB_TRANSFER_TIMED_OUT:
            transfer->status = TimeOut;
            break;

         case LIBUSB_TRANSFER_CANCELLED:
            transfer->status = Cancelled;
            break;

         case LIBUSB_TRANSFER_STALL:
            transfer->status = Stall;
            break;

         case LIBUSB_TRANSFER_NO_DEVICE:
            transfer->status = NoDevice;
            break;

         case LIBUSB_TRANSFER_OVERFLOW:
            transfer->status = Overflow;
            break;

         default:
            log->error("unknown transfer status");
            break;
      }

      // run user callback if present...
      if (transfer->callback)
      {
         if ((transferInfo->transfer = transfer->callback(transfer)))
         {
            libusb_fill_bulk_transfer(usbTransfer, hdl, usbTransfer->endpoint, transfer->data, (int)transfer->available, transferHandler, transferInfo, transfer->timeout);

            if ((result = libusb_submit_transfer(usbTransfer)) == LIBUSB_SUCCESS)
               return;

            log->error("error in submit async transfer: {}", {lastError()});
         }
      }

      // free transfer owner
      delete transferInfo;

      // remove from transfer list
      transfers.remove(transferInfo);

      // free underline transfer
      libusb_free_transfer(usbTransfer);
   }

   void list()
   {

   }
};

Usb::Usb(const Descriptor &desc) : impl(new Impl(desc))
{
}

bool Usb::open() const
{
   return impl->open();
}

void Usb::close()
{
   impl->close();
}

const Usb::Descriptor &Usb::descriptor() const
{
   return impl->descriptor;
}

int Usb::speed() const
{
   return impl->speed;
}

bool Usb::claimInterface(int interface)
{
   return impl->claimInterface(interface);
}

bool Usb::releaseInterface(int interface)
{
   return impl->releaseInterface(interface);
}

bool Usb::ctrlTransfer(int outCmd, const void *txData, unsigned int txSize, int inCmd, void *rxData, unsigned int rxSize, int timeout, int wait) const
{
   return impl->ctrlTransfer(outCmd, txData, txSize, inCmd, rxData, rxSize, timeout, wait);
}

int Usb::syncTransfer(Direction direction, int endpoint, void *data, unsigned int length, int timeout) const
{
   return impl->syncTransfer((direction ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN) | endpoint, data, length, timeout);
}

int Usb::asyncTransfer(Direction direction, int endpoint, Transfer *transfer) const
{
   return impl->asyncTransfer((direction ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN) | endpoint, transfer);
}

int Usb::cancelTransfer(Transfer *transfer) const
{
   return impl->cancelTransfer(transfer);
}

std::string Usb::lastError() const
{
   return impl->lastError();
}

bool Usb::isValid() const
{
   return impl->descriptor.vid != 0 && impl->descriptor.pid != 0;
}

bool Usb::isOpen() const
{
   return impl->hdl != nullptr;
}

bool Usb::isLowSpeed() const
{
   return impl->speed == LIBUSB_SPEED_LOW;
}

bool Usb::isHighSpeed() const
{
   return impl->speed == LIBUSB_SPEED_HIGH;
}

bool Usb::isSuperSpeed() const
{
   return impl->speed == LIBUSB_SPEED_SUPER;
}

std::list<Usb::Descriptor> Usb::list()
{
   std::list<Descriptor> devices;

   rt::Logger *log = rt::Logger::getLogger("hw.UsbDevice");

   libusb_device **devs;

   ssize_t count;

   // initialize usb library
   if (!ctx)
      ctx = std::make_shared<UsbContext>();

   // enumerate usb devices
   if ((count = libusb_get_device_list(*ctx, &devs)) < 0)
   {
      log->error("error getting USB device list: {}", {Impl::errorString((int)count)});
      return {};
   }

   // scan devices
   for (ssize_t i = 0; i < count; i++)
   {
      libusb_device *dev = devs[i];
      libusb_device_handle *hdl = nullptr;
      libusb_device_descriptor desc {};
      unsigned char manufacturer[64];
      unsigned char product[64];

      /* Assume the FW has not been loaded, unless proven wrong. */
      if (libusb_get_device_descriptor(dev, &desc) != LIBUSB_SUCCESS)
         continue;

      if (libusb_open(dev, &hdl) != LIBUSB_SUCCESS)
         continue;

      if (libusb_get_string_descriptor_ascii(hdl, desc.iManufacturer, manufacturer, sizeof(manufacturer)) < 0)
      {
         libusb_close(hdl);
         continue;
      }

      if (libusb_get_string_descriptor_ascii(hdl, desc.iProduct, product, sizeof(product)) < 0)
      {
         libusb_close(hdl);
         continue;
      }

      // close device
      libusb_close(hdl);

      // generate descriptor
      devices.push_back({
         .vid = desc.idVendor,
         .pid = desc.idProduct,
         .bus = libusb_get_bus_number(dev),
         .address = libusb_get_device_address(dev),
         .manufacturer = rt::Format::trim(reinterpret_cast<char *>(manufacturer)),
         .product = rt::Format::trim(reinterpret_cast<char *>(product)),
      });
   }

   return devices;
}

void transferHandler(struct libusb_transfer *t)
{
   if (auto transferInfo = static_cast<TransferInfo *>(t->user_data))
   {
      transferInfo->device->processTransfer(transferInfo);
   }
   else
   {
      libusb_free_transfer(t);
   }
}

}
