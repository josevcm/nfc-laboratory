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

#include <rt/Logger.h>

#include <lab/iso/IsoDecoder.h>

#include <tech/Iso7816.h>

namespace lab {

struct IsoDecoder::Impl
{
   rt::Logger *log = rt::Logger::getLogger("decoder.IsoDecoder");

   static constexpr int ENABLED_ISO7816 = 1 << 0;

   // debug disabled by default
   int debugEnabled = false;

   // all tech enabled by default
   int enabledTech = ENABLED_ISO7816;

   // NFC-A Decoder
   Iso7816 iso7816;

   // global decoder status
   IsoDecoderStatus decoder;

   Impl();

   inline void cleanup();

   inline void initialize();

   inline std::list<RawFrame> nextFrames(hw::SignalBuffer &samples);
};

IsoDecoder::IsoDecoder() : impl(std::make_shared<Impl>())
{
}

void IsoDecoder::initialize()
{
   impl->initialize();
}

void IsoDecoder::cleanup()
{
   impl->cleanup();
}

bool IsoDecoder::isDebugEnabled() const
{
   return impl->debugEnabled;
}

void IsoDecoder::setEnableDebug(bool enabled)
{
   impl->debugEnabled = enabled;
}

bool IsoDecoder::isISO7816Enabled() const
{
   return impl->enabledTech & Impl::ENABLED_ISO7816;
}

void IsoDecoder::setEnableISO7816(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_ISO7816;
   else
      impl->enabledTech &= ~Impl::ENABLED_ISO7816;
}

std::list<RawFrame> IsoDecoder::nextFrames(hw::SignalBuffer samples)
{
   return impl->nextFrames(samples);
}

long IsoDecoder::sampleRate() const
{
   return impl->decoder.sampleRate;
}

void IsoDecoder::setSampleRate(long sampleRate)
{
   impl->decoder.sampleRate = sampleRate;
}

long IsoDecoder::streamTime() const
{
   return impl->decoder.streamTime;
}

void IsoDecoder::setStreamTime(long referenceTime)
{
   impl->decoder.streamTime = referenceTime;
}

IsoDecoder::Impl::Impl() : iso7816(&decoder)
{
}

void IsoDecoder::Impl::initialize()
{
   log->warn("initialize ISO decoder");

   // clear signal master clock
   decoder.signalClock = 0;

   // clear signal cache
   decoder.signalCache.reset();

   // configure only if samplerate > 0
   if (decoder.sampleRate > 0)
   {
      // calculate sample time unit
      decoder.sampleTime = 1.0 / static_cast<double>(decoder.sampleRate);

      // configure ISO7816 decoder
      iso7816.initialize(decoder.sampleRate);

      if (debugEnabled)
      {
         log->warn("---------------------------------------------------");
         log->warn("SIGNAL DEBUG ENABLED!, highly affected performance!");
         log->warn("---------------------------------------------------");

         decoder.debug = std::make_shared<IsoSignalDebug>(DEBUG_CHANNELS, decoder.sampleRate);

         log->warn("write signal debug data to file: {}", {std::get<std::string>(decoder.debug->recorder->get(hw::SignalDevice::PARAM_DEVICE_NAME))});
      }
   }

   // starts without bitrate
   decoder.bitrate = nullptr;

   // starts without modulation
   decoder.modulation = nullptr;
}

void IsoDecoder::Impl::cleanup()
{
   if (decoder.debug)
      decoder.debug.reset();
}

std::list<RawFrame> IsoDecoder::Impl::nextFrames(hw::SignalBuffer &samples)
{
   // detected frames
   std::list<RawFrame> frames;

   // only process valid sample buffer
   if (samples)
   {
      // re-configure decoder parameters on sample rate changes
      if (decoder.sampleRate != samples.sampleRate())
      {
         decoder.sampleRate = samples.sampleRate();

         initialize();
      }

      if (decoder.debug)
         decoder.debug->begin(samples.elements());
   }

   do
   {
      if (!decoder.bitrate)
      {
         while (decoder.nextSample(samples))
         {
            if ((enabledTech & ENABLED_ISO7816) && iso7816.detect(frames))
               break;
         }
      }

      if (decoder.bitrate)
      {
         switch (decoder.bitrate->techType)
         {
            case Iso7816Tech:
               iso7816.decode(samples, frames);
               break;
            default:
               log->warn("unsupported tech type: {}", {decoder.bitrate->techType});
               break;
         }
      }

   }
   while (decoder.hasSamples(samples));

   if (decoder.debug)
      decoder.debug->write();

   return frames;
}

}
