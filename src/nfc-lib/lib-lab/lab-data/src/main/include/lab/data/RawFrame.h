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

#ifndef DATA_RAWFRAME_H
#define DATA_RAWFRAME_H

#include <rt/ByteBuffer.h>

namespace lab {

enum FrameTech
{
   NoneTech = 0x0000,

   // NFC tech types
   NfcNoneTech = 0x0100,
   NfcATech = 0x0101,
   NfcBTech = 0x0102,
   NfcFTech = 0x0103,
   NfcVTech = 0x0104,

   // ISO tech types
   IsoNoneTech = 0x0200,
   Iso7816Tech = 0x0201,
};

enum FrameType
{
   // NFC Frame types
   NfcCarrierOff = 0x0100,
   NfcCarrierOn = 0x0101,
   NfcPollFrame = 0x0102,
   NfcListenFrame = 0x0103,

   // ISO Frame types
   IsoATRFrame = 0x0201,
   IsoRequestFrame = 0x0211,
   IsoResponseFrame = 0x0212,
   IsoExchangeFrame = 0x0213,
};

enum FramePhase
{
   // NFC Frame phases
   NfcCarrierPhase = 0x0100,
   NfcSelectionPhase = 0x0101,
   NfcApplicationPhase = 0x0102
};

enum FrameFlags
{
   ShortFrame = 0x01,
   Encrypted = 0x02,
   Truncated = 0x08,
   ParityError = 0x10,
   CrcError = 0x20,
   SyncError = 0x40
};

class RawFrame : public rt::ByteBuffer
{
   struct Impl;

   public:

      static const RawFrame Nil;

   public:

      RawFrame();

      explicit RawFrame(unsigned int size);

      RawFrame(unsigned int techType, unsigned int frameType);

      RawFrame(unsigned int techType, unsigned int frameType, double timeStart, double timeEnd);

      RawFrame(const RawFrame &other);

      RawFrame &operator=(const RawFrame &other);

      bool operator==(const RawFrame &other) const;

      bool operator!=(const RawFrame &other) const;

      bool operator<(const RawFrame &other) const;

      bool operator>(const RawFrame &other) const;

      operator bool() const;

      unsigned int techType() const;

      void setTechType(unsigned int techType);

      unsigned int frameType() const;

      void setFrameType(unsigned int frameType);

      unsigned int framePhase() const;

      void setFramePhase(unsigned int framePhase);

      unsigned int frameFlags() const;

      void setFrameFlags(unsigned int frameFlags);

      void clearFrameFlags(unsigned int frameFlags);

      bool hasFrameFlags(unsigned int frameFlags) const;

      unsigned int frameRate() const;

      void setFrameRate(unsigned int frameRate);

      double timeStart() const;

      void setTimeStart(double timeStart);

      double timeEnd() const;

      void setTimeEnd(double timeEnd);

      double dateTime() const;

      void setDateTime(double dateTime);

      unsigned long sampleStart() const;

      void setSampleStart(unsigned long sampleStart);

      unsigned long sampleEnd() const;

      void setSampleEnd(unsigned long sampleEnd);

      unsigned long sampleRate() const;

      void setSampleRate(unsigned long sampleRate);

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif
