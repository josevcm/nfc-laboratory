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

#ifndef LAB_NFC_H
#define LAB_NFC_H

namespace lab {

enum NfcRateType
{
   r106k = 0,
   r212k = 1,
   r424k = 2,
   r848k = 3
};

// Frequency of operating field (carrier frequency) in Hz
constexpr float NFC_FC = 13.56E6;

// Frequency of subcarrier modulation in Hz
constexpr float NFC_FS = NFC_FC / 16;

// Elementary time unit
constexpr float NFC_ETU = 128 / NFC_FC;

// Activation frame waiting time, in 1/FS units
constexpr int NFC_FWT_ACTIVATION = 71680;

// FSDI to FSD conversion (frame size)
constexpr int NFC_FDS_TABLE[] = {16, 24, 32, 40, 48, 64, 96, 128, 256, 512, 1024, 2048, 4096, 0, 0, 0};

// Start-up Frame Guard Time, SFGT = 256 x 16 * (2 ^ SFGI) in 1/fc units
constexpr int NFC_SFGT_TABLE[] = {4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728};

// Frame waiting time FWT = 256 x 16 * (2 ^ FWI) in 1/fc units
constexpr int NFC_FWT_TABLE[] = {4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728};

/*
 * NFC-A parameters
 */

// NFC-A Default guard time between the end of a PCD transmission and the start of the PICC subcarrier generation in 1/FC units
constexpr int NFCA_FGT_DEF = 1024;

// NFC-A Default Frame Waiting Time
constexpr int NFCA_FWT_DEF = 256 * 16 * (1 << 4);

// NFC-A Default Start-up Frame Guard Time
constexpr int NFCA_SFGT_DEF = 256 * 16 * (1 << 0);

// NFC-A Default Request Guard Time
constexpr int NFCA_RGT_DEF = 7000;

// NFC-A Maximum frame Waiting Time for ATQA response
constexpr int NFCA_FWT_ATQA = 128 * 18;

/*
 * NFC-B parameters
 */

// NFC-B Guard time between the end of a PCD transmission and the start of the PICC subcarrier generation in 1/FC units
constexpr int NFCB_TR0_MIN = 1024;

// NFC-B Synchronization time between the start of the PICC subcarrier generation and the start of the PICC subcarrier modulation in 1/FC units
constexpr int NFCB_TR1_MIN = 1024;

// NFC-B Synchronization time between the start of the PICC subcarrier generation and the start of the PICC subcarrier modulation in 1/FC units
constexpr int NFCB_TR1_MAX = 3200;

// NFC-B Start of Sequence first modulation
constexpr int NFCB_TLISTEN_S1_MIN = 1272;

// NFC-B Start of Sequence first modulation
constexpr int NFCB_TLISTEN_S1_MAX = 1416;

// NFC-B Start of Sequence first modulation
constexpr int NFCB_TLISTEN_S2_MIN = 248;

// NFC-B Start of Sequence first modulation
constexpr int NFCB_TLISTEN_S2_MAX = 392;

// NFC-B Default Request Guard Time
constexpr int NFCB_FGT_DEF = NFCB_TR0_MIN;

// NFC-B Default Frame Waiting Time
constexpr int NFCB_FWT_DEF = 256 * 16 * (1 << 4);

// NFC-B Default Start-up Frame Guard Time
constexpr int NFCB_SFGT_DEF = 256 * 16 * (1 << 0);

// NFC-B Default Request Guard Time, defined as the minimum time between the start bits of two consecutive REQA commands.
constexpr int NFCB_RGT_DEF = 7000;

// NFC-B Frame Waiting Time for ATQB response
constexpr int NFCB_FWT_ATQB = 7680;

// NFC-B Number of Slots
constexpr int NFCB_SLOT_TABLE[] = {1, 2, 4, 8, 16, 0, 0, 0};

// NFC-B TR0min, in 1/FC units
constexpr int NFCB_TR0_MIN_TABLE[] = {0, 48 * 16, 16 * 16, 0};

// NFC-B TR1min, in 1/FC units
constexpr int NFCB_TR1_MIN_TABLE[] = {0, 64 * 16, 16 * 16, 0};

/*
 * NFC-F parameters
 */

// NFC-F Default Request Guard Time
constexpr int NFCF_FGT_DEF = 1024;

// NFC-F Default Frame Waiting Time
constexpr int NFCF_FWT_DEF = 256 * 16 * (1 << 4);

// NFC-F Default Start-up Frame Guard Time
constexpr int NFCF_SFGT_DEF = 4096;

// NFC-F Default Request Guard Time, defined as the minimum time between the start bits of two consecutive REQC commands.
constexpr int NFCF_RGT_DEF = 7000;

// NFC-F Frame Delay Time for ATQC response, between the end of the Request Frame and the first Time Slot
constexpr int NFCF_FDT_ATQC = 512 * 64;

// NFC-F Time Slot Unit for ATQC
constexpr int NFCF_TSU_ATQC = 256 * 64;

/*
 * NFC-V parameters
 */

// NFC-V Guard time between the end of a PCD transmission and the start of the PICC subcarrier generation in 1/FC units
constexpr int NFCV_TR0_MIN = 1024;

// NFC-V Default Request Guard Time
constexpr int NFCV_FGT_DEF = NFCV_TR0_MIN;

// NFC-V Default Request Guard Time
constexpr int NFCV_TLISTEN_S1 = 768;

// NFC-V Default Request Guard Time
constexpr int NFCV_TLISTEN_S2 = 256;

// NFC-V Default Frame Waiting Time
constexpr int NFCV_FWT_DEF = 256 * 16 * (1 << 4);

// NFC-V Default Start-up Frame Guard Time
constexpr int NFCV_SFGT_DEF = 4096;

// NFC-V Default Request Guard Time, defined as the minimum time between the start bits of two consecutive REQV commands.
constexpr int NFCV_RGT_DEF = 7000;

}

#endif
