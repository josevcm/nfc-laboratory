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

#include "nfc/NfcA.h"

namespace nfc {

//bool NfcA::decodeFrameDevNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
//{
//   int pattern;
//
//   // read NFC-A request request
//   while ((pattern = decodeSymbolDevAskNfcA(buffer)) > PatternType::NoPattern)
//   {
//      streamStatus.pattern = pattern;
//
//      // detect end of request (Pattern-Y after Pattern-Z)
//      if ((streamStatus.pattern == PatternType::PatternY && (streamStatus.previous == PatternType::PatternY || streamStatus.previous == PatternType::PatternZ)) || streamStatus.bytes == protocolStatus.maxFrameSize)
//      {
//         // frames must contains at least one full byte or 7 bits for short frames
//         if (streamStatus.bytes > 0 || streamStatus.bits == 7)
//         {
//            // add remaining byte to request
//            if (streamStatus.bits >= 7)
//               streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
//
//            // set last symbol timing
//            if (streamStatus.previous == PatternType::PatternZ)
//               frameStatus.frameEnd = symbolStatus.start - bitrate->period2SymbolSamples;
//            else
//               frameStatus.frameEnd = symbolStatus.start - bitrate->period1SymbolSamples;
//
//            // build request frame
//            NfcFrame request = NfcFrame(TechType::NfcA, FrameType::PollFrame);
//
//            request.setFrameRate(frameStatus.symbolRate);
//            request.setSampleStart(frameStatus.frameStart);
//            request.setSampleEnd(frameStatus.frameEnd);
//            request.setTimeStart(double(frameStatus.frameStart) / double(sampleRate));
//            request.setTimeEnd(double(frameStatus.frameEnd) / double(sampleRate));
//
//            if (streamStatus.flags & ParityError)
//               request.setFrameFlags(FrameFlags::ParityError);
//
//            if (streamStatus.bytes == protocolStatus.maxFrameSize)
//               request.setFrameFlags(FrameFlags::Truncated);
//
//            if (streamStatus.bytes == 1 && streamStatus.bits == 7)
//               request.setFrameFlags(FrameFlags::ShortFrame);
//
//            // add bytes to frame and flip to prepare read
//            request.put(streamStatus.buffer, streamStatus.bytes).flip();
//
//            // clear modulation status for next frame search
//            modulation->symbolStartTime = 0;
//            modulation->symbolEndTime = 0;
//            modulation->filterIntegrate = 0;
//            modulation->phaseIntegrate = 0;
//
//            // clear stream status
//            streamStatus = {0,};
//
//            // process frame
//            process(request);
//
//            // add to frame list
//            frames.push_back(request);
//
//            // return request frame data
//            return true;
//         }
//
//         // reset modulation and restart frame detection
//         resetModulation();
//
//         // no valid frame found
//         return false;
//      }
//
//      if (streamStatus.previous)
//      {
//         int value = (streamStatus.previous == PatternType::PatternX);
//
//         // decode next bit
//         if (streamStatus.bits < 8)
//         {
//            streamStatus.data = streamStatus.data | (value << streamStatus.bits++);
//         }
//
//         // store full byte in stream buffer and check parity
//         else if (streamStatus.bytes < protocolStatus.maxFrameSize)
//         {
//            streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
//            streamStatus.flags |= !checkParity(streamStatus.data, value) ? ParityError : 0;
//            streamStatus.data = streamStatus.bits = 0;
//         }
//
//         // too many bytes in frame, abort decoder
//         else
//         {
//            // reset modulation status
//            resetModulation();
//
//            // no valid frame found
//            return false;
//         }
//      }
//
//      // update previous command state
//      streamStatus.previous = streamStatus.pattern;
//   }
//
//   // no frame detected
//   return false;
//}
//
//int NfcA::decodeSymbolDevAskNfcA(sdr::SignalBuffer &buffer)
//{
//   symbolStatus.pattern = PatternType::Invalid;
//
//   while (nextSample(buffer))
//   {
//      // compute pointers
//      modulation->signalIndex = (bitrate->offsetSignalIndex + signalClock);
//      modulation->filterIndex = (bitrate->offsetFilterIndex + signalClock);
//
//      // get signal samples
//      float currentData = signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];
//      float delayedData = signalStatus.signalData[modulation->filterIndex & (SignalBufferLength - 1)];
//
//      // integrate signal data over 1/2 symbol
//      modulation->filterIntegrate += currentData; // add new value
//      modulation->filterIntegrate -= delayedData; // remove delayed value
//
//      // correlation pointers
//      modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
//      modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
//      modulation->filterPoint3 = (modulation->signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;
//
//      // store integrated signal in correlation buffer
//      modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;
//
//      // compute correlation factors
//      modulation->correlatedS0 = modulation->correlationData[modulation->filterPoint1] - modulation->correlationData[modulation->filterPoint2];
//      modulation->correlatedS1 = modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint3];
//      modulation->correlatedSD = std::fabs(modulation->correlatedS0 - modulation->correlatedS1) / float(bitrate->period2SymbolSamples);
//
//#ifdef DEBUG_ASK_CORRELATION_CHANNEL
//      decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, modulation->correlatedSD);
//#endif
//
//#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
//      decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
//#endif
//      // compute symbol average
//      modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;
//
//      // set next search sync window from previous state
//      if (!modulation->searchStartTime)
//      {
//         // estimated symbol start and end
//         modulation->symbolStartTime = modulation->symbolEndTime;
//         modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;
//
//         // timig search window
//         modulation->searchStartTime = modulation->symbolEndTime - bitrate->period8SymbolSamples;
//         modulation->searchEndTime = modulation->symbolEndTime + bitrate->period8SymbolSamples;
//
//         // reset symbol parameters
//         modulation->symbolCorr0 = 0;
//         modulation->symbolCorr1 = 0;
//      }
//
//      // search max correlation peak
//      if (signalClock >= modulation->searchStartTime && signalClock <= modulation->searchEndTime)
//      {
//         if (modulation->correlatedSD > modulation->correlationPeek)
//         {
//            modulation->correlationPeek = modulation->correlatedSD;
//            modulation->symbolCorr0 = modulation->correlatedS0;
//            modulation->symbolCorr1 = modulation->correlatedS1;
//            modulation->symbolEndTime = signalClock;
//         }
//      }
//
//      // capture next symbol
//      if (signalClock == modulation->searchEndTime)
//      {
//#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
//         decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
//#endif
//         // detect Pattern-Y when no modulation occurs (below search detection threshold)
//         if (modulation->correlationPeek < modulation->searchThreshold)
//         {
//            // estimate symbol end from start (peak detection not valid due lack of modulation)
//            modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;
//
//            // setup symbol info
//            symbolStatus.value = 1;
//            symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
//            symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
//            symbolStatus.length = symbolStatus.end - symbolStatus.start;
//            symbolStatus.pattern = PatternType::PatternY;
//
//            break;
//         }
//
//         // detect Pattern-Z
//         if (modulation->symbolCorr0 > modulation->symbolCorr1)
//         {
//            // setup symbol info
//            symbolStatus.value = 0;
//            symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
//            symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
//            symbolStatus.length = symbolStatus.end - symbolStatus.start;
//            symbolStatus.pattern = PatternType::PatternZ;
//
//            break;
//         }
//
//         // detect Pattern-X, setup symbol info
//         symbolStatus.value = 1;
//         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
//         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
//         symbolStatus.length = symbolStatus.end - symbolStatus.start;
//         symbolStatus.pattern = PatternType::PatternX;
//
//         break;
//      }
//   }
//
//   // reset search status if symbol has detected
//   if (symbolStatus.pattern != PatternType::Invalid)
//   {
//      modulation->searchStartTime = 0;
//      modulation->searchEndTime = 0;
//      modulation->searchPulseWidth = 0;
//      modulation->correlationPeek = 0;
//      modulation->correlatedSD = 0;
//   }
//
//   return symbolStatus.pattern;
//}
//
//bool NfcA::nextSample(sdr::SignalBuffer &buffer)
//{
//   return false;
//}

}