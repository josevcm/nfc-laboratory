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

#ifndef LOGIC_ISODECODER_H
#define LOGIC_ISODECODER_H

#include <list>

#include <hw/SignalBuffer.h>

#include <lab/data/RawFrame.h>

namespace lab {

class IsoDecoder
{
   struct Impl;

   public:

      IsoDecoder();

      void initialize();

      void cleanup();

      std::list<RawFrame> nextFrames(hw::SignalBuffer samples);

      bool isDebugEnabled() const;

      void setEnableDebug(bool enabled);

      bool isISO7816Enabled() const;

      void setEnableISO7816(bool enabled);

      long sampleRate() const;

      void setSampleRate(long sampleRate);

      long streamTime() const;

      void setStreamTime(long referenceTime);

   private:

      std::shared_ptr<Impl> impl;

};

}

#endif
