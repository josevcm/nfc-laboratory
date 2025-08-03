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

#ifndef DEV_SIGNALTYPE_H
#define DEV_SIGNALTYPE_H

namespace hw {

enum SignalType
{
   SIGNAL_TYPE_RADIO_IQ = 1, // 2 float components per sample (I / Q value)
   SIGNAL_TYPE_RADIO_SAMPLES = 2, // 1 float component per sample (magnitude value)
   SIGNAL_TYPE_RADIO_SIGNAL = 3, // 2 float components per sample (value / offset)
   SIGNAL_TYPE_LOGIC_SAMPLES = 12, // N float component per sample (1 / 0 value)
   SIGNAL_TYPE_LOGIC_SIGNAL = 13, // 2 float components per sample (value / offset)
   SIGNAL_TYPE_FFT_BIN = 20, // 2 float components per sample (magnitude / phase)
   SIGNAL_TYPE_CLK_SIGNAL = 21, // 2 float components per sample (clock / offset)
};

enum SampleType
{
   SAMPLE_TYPE_INTEGER = 1,
   SAMPLE_TYPE_FLOAT = 2
};

enum SampleSize
{
   SAMPLE_SIZE_8 = 8,
   SAMPLE_SIZE_16 = 16,
   SAMPLE_SIZE_32 = 32,
};

}

#endif
