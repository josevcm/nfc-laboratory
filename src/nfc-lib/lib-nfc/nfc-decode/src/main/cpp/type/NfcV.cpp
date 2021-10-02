/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include "NfcV.h"

namespace nfc {

struct NfcV::Impl
{
   rt::Logger log {"NfcV"};

   DecoderStatus *decoder;

   // bitrate parameters
   BitrateParams bitrateParams[4] {0,};

   // detected symbol status
   SymbolStatus symbolStatus {0,};

   // bit stream status
   StreamStatus streamStatus {0,};

   // frame processing status
   FrameStatus frameStatus {0,};

   // protocol processing status
   ProtocolStatus protocolStatus {0,};

   // modulation status for each bitrate
   ModulationStatus modulationStatus[4] {0,};

   // minimum modulation threshold to detect valid signal for NFC-V (default 85%)
   float minimumModulationThreshold = 0.850f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   Impl(DecoderStatus *decoder) : decoder(decoder)
   {
   }
};

NfcV::NfcV(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcV::~NfcV()
{
   delete self;
}

void NfcV::setModulationThreshold(float min)
{
   self->minimumModulationThreshold = min;
}

void NfcV::configure(long sampleRate)
{
   self->log.info("--------------------------------------------");
   self->log.info("initializing NFC-V decoder");
   self->log.info("--------------------------------------------");
   self->log.info("\tsignalSampleRate     {}", {self->decoder->sampleRate});
   self->log.info("\tpowerLevelThreshold  {}", {self->decoder->powerLevelThreshold});
   self->log.info("\tmodulationThreshold  {}", {self->minimumModulationThreshold});
}

bool NfcV::detectModulation()
{
   return false;
}

void NfcV::decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   if (self->frameStatus.frameType == PollFrame)
   {
      decodePollFrame(samples, frames);
   }

   if (self->frameStatus.frameType == ListenFrame)
   {
      decodeListenFrame(samples, frames);
   }
}

bool NfcV::decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   return false;
}

bool NfcV::decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   return false;
}

int NfcV::decodePollFrameSymbolAsk(sdr::SignalBuffer &buffer)
{
   return 0;
}

int NfcV::decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
{
   return 0;
}


}
