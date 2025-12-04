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

#ifndef DEV_USB_H
#define DEV_USB_H

#include <list>
#include <memory>

namespace hw {

class Usb
{
   public:

      struct Impl;

      enum Direction
      {
         In = 0,
         Out = 1
      };

      typedef struct
      {
         int vid;
         int pid;
         int bus;
         int address;
         std::string manufacturer;
         std::string product;
      } Descriptor;

      enum TransferStatus
      {
         Issued,
         Completed,
         Error,
         TimeOut,
         Cancelled,
         Stall,
         NoDevice,
         Overflow
      };

      struct Transfer
      {
         TransferStatus status = Issued;
         void *user = nullptr;
         unsigned char *data = nullptr;
         unsigned int available = 0;
         unsigned int actual = 0;
         unsigned int timeout = 0;
         std::function<Transfer *(Transfer *transfer)> callback;
      };

   public:

      explicit Usb(const Descriptor &desc = {});

      ~Usb() = default;

      static std::list<Descriptor> list();

      bool open() const;

      void close();

      const Descriptor &descriptor() const;

      int speed() const;

      bool claimInterface(int interface);

      bool releaseInterface(int interface);

      bool ctrlTransfer(int outCmd, const void *txData, unsigned int txSize, int inCmd = 0, void *rxData = nullptr, unsigned int rxSize = 0, int timeout = 3000, int wait = 10) const;

      int syncTransfer(Direction direction, int endpoint, void *data, unsigned int length, int timeout = 30000) const;

      bool asyncTransfer(Direction direction, int endpoint, Transfer *transfer) const;

      bool cancelTransfer(Transfer *transfer) const;

      std::string lastError() const;

      bool isValid() const;

      bool isOpen() const;

      bool isLowSpeed() const;

      bool isHighSpeed() const;

      bool isSuperSpeed() const;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif
