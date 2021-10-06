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

#ifndef NFC_NFC_H
#define NFC_NFC_H

// Frequency of operating field (carrier frequency)
#define NFC_FC 13.56E6

// Frequency of subcarrier modulation
#define NFC_FS (NFC_FC/16)

// Guard time between the end of a PCD transmission and the start of the PICC subcarrier generation
#define NFC_TR0_MIN 64

// Synchronization time between the start of the PICC subcarrier generation and the start of the PICC subcarrier modulation
#define NFC_TR1_MIN 80

// FSDI to FSD conversion (frame size)
constexpr static const int NFC_FDS_TABLE[] = {16, 24, 32, 40, 48, 64, 96, 128, 256, 512, 1024, 2048, 4096, 0, 0, 0};

// Start-up Frame Guard Time (SFGT = (256 x 16 / fc) * 2 ^ SFGI)
constexpr static const float NFC_SFGT_TABLE[] = {0.000302065, 0.00060413, 0.00120826, 0.002416519, 0.004833038, 0.009666077, 0.019332153, 0.038664307, 0.077328614, 0.154657227, 0.309314454, 0.618628909, 1.237257817, 2.474515634, 4.949031268, 9.898062537};

// Frame waiting time (FWT = (256 x 16 / fc) * 2 ^ FWI)
constexpr static const float NFC_FWT_TABLE[] = {0.000302065, 0.00060413, 0.00120826, 0.002416519, 0.004833038, 0.009666077, 0.019332153, 0.038664307, 0.077328614, 0.154657227, 0.309314454, 0.618628909, 1.237257817, 2.474515634, 4.949031268, 9.898062537};

// Number of Slots
constexpr static const float NFCB_SLOT_TABLE[] = {1, 2, 4, 8, 16, 0, 0, 0};

// TR0min
constexpr static const float NFCB_TR0_MIN_TABLE[] = {0, 48, 16, 0};

// TR1min
constexpr static const float NFCB_TR1_MIN_TABLE[] = {0, 64, 16, 0};

namespace nfc {

enum TechType
{
   None = 0,
   NfcA = 1,
   NfcB = 2,
   NfcF = 3,
   NfcV = 4
};

enum RateType
{
   r106k = 0,
   r212k = 1,
   r424k = 2,
   r848k = 3
};

enum FrameType
{
   NoCarrier = 0,
   EmptyFrame = 1,
   PollFrame = 2,
   ListenFrame = 3
};

enum FrameFlags
{
   ShortFrame = 0x01,
   Encrypted = 0x08,
   ParityError = 0x20,
   CrcError = 0x40,
   Truncated = 0x80
};

enum FramePhase
{
   CarrierFrame = 0,
   SelectionFrame = 1,
   ApplicationFrame = 2
};

}

#endif //NFC_LAB_NFC_H
