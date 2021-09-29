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

#include "NfcB.h"

#ifdef DEBUG_SIGNAL
//#define DEBUG_ASK_CORRELATION_CHANNEL 5
//#define DEBUG_ASK_INTEGRATION_CHANNEL 6
//#define DEBUG_ASK_SYNCHRONIZATION_CHANNEL 7

//#define DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL 5
//#define DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL 4
//#define DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL 7
#endif

namespace nfc {

struct NfcB::Impl
{
   rt::Logger log {"NfcB"};

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

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   Impl(DecoderStatus *decoder) : decoder(decoder)
   {
   }
};

NfcB::NfcB(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcB::~NfcB()
{
   delete self;
}

void NfcB::configure(long sampleRate)
{
   self->log.info("--------------------------------------------");
   self->log.info("initializing NFC-B decoder");
   self->log.info("--------------------------------------------");
   self->log.info("\tsignalSampleRate     {}", {self->decoder->sampleRate});
   self->log.info("\tpowerLevelThreshold  {}", {self->decoder->powerLevelThreshold});
   self->log.info("\tmodulationThreshold  {}", {self->decoder->modulationThreshold});

   // clear detected symbol status
   self->symbolStatus = {0,};

   // clear bit stream status
   self->streamStatus = {0,};

   // clear frame processing status
   self->frameStatus = {0,};

   // clear last detected frame end
   self->lastFrameEnd = 0;

   // clear chained flags
   self->chainedFlags = 0;
}

bool NfcB::detectModulation()
{
   // ignore low power signals
   if (self->decoder->signalStatus.powerAverage > self->decoder->powerLevelThreshold)
   {
   }

   return false;
}

void NfcB::decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
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

bool NfcB::decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   return false;
}

bool NfcB::decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   return false;
}

int NfcB::decodePollFrameSymbolAsk(sdr::SignalBuffer &buffer)
{
   return 0;
}

int NfcB::decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
{
   return 0;
}


}
