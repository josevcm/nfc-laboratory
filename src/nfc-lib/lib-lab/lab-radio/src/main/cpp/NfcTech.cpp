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

#include <lab/data/RawFrame.h>

#include <NfcTech.h>

namespace lab {

bool NfcDecoderStatus::nextSample(hw::SignalBuffer &buffer)
{
   if (buffer.remaining() == 0 || buffer.type() != hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES)
      return false;

   // update signal clock and pulse filter
   ++signalClock;
   ++pulseFilter;

   signalValue = buffer.get();

   float signalDiff = std::abs(signalValue - signalEnvelope) / signalEnvelope;

   // signal average envelope detector
   if (signalDiff < 0.05f || pulseFilter > signalParams.elementaryTimeUnit * 10)
   {
      // reset silence counter
      pulseFilter = 0;

      // compute signal average
      signalEnvelope = signalEnvelope * signalParams.signalEnveW0 + signalValue * signalParams.signalEnveW1;
   }
   else if (signalClock < signalParams.elementaryTimeUnit)
   {
      signalEnvelope = signalValue;
   }

   // process new IIR filter value
   signalFilterN0 = signalValue + signalFilterN1 * signalParams.signalIIRdcA;

   // update signal value for IIR removal filter
   signalFiltered = signalFilterN0 - signalFilterN1;

   // update IIR filter component
   signalFilterN1 = signalFilterN0;

   // compute signal variance
   signalDeviation = signalDeviation * signalParams.signalMdevW0 + std::abs(signalFiltered) * signalParams.signalMdevW1;

   // process new signal envelope value
   signalAverage = signalAverage * signalParams.signalMeanW0 + signalValue * signalParams.signalMeanW1;

   // store signal components in process buffer
   sample[signalClock & (BUFFER_SIZE - 1)].samplingValue = signalValue;
   sample[signalClock & (BUFFER_SIZE - 1)].filteredValue = signalFiltered;
   sample[signalClock & (BUFFER_SIZE - 1)].meanDeviation = signalDeviation;
   sample[signalClock & (BUFFER_SIZE - 1)].modulateDepth = (signalEnvelope - std::clamp(signalValue, 0.0f, signalEnvelope)) / signalEnvelope;

   // get absolute DC-removed signal for edge detector
   float filteredRectified = std::fabs(signalFiltered);

   // detect last carrier edge on/off
   if (filteredRectified > signalHighThreshold)
   {
      // search maximum pulse value
      if (filteredRectified > carrierEdgePeak)
      {
         carrierEdgePeak = filteredRectified;
         carrierEdgeTime = signalClock;
      }
   }
   else if (filteredRectified < signalLowThreshold)
   {
      carrierEdgePeak = 0;
   }

   if (debug)
   {
      debug->block(signalClock);

      debug->set(DEBUG_SIGNAL_VALUE_CHANNEL, sample[signalClock & (BUFFER_SIZE - 1)].samplingValue);
      debug->set(DEBUG_SIGNAL_FILTERED_CHANNEL, sample[signalClock & (BUFFER_SIZE - 1)].filteredValue);
      debug->set(DEBUG_SIGNAL_VARIANCE_CHANNEL, sample[signalClock & (BUFFER_SIZE - 1)].meanDeviation);
      debug->set(DEBUG_SIGNAL_AVERAGE_CHANNEL, signalAverage);
   }

   return true;
}

}
