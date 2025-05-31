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

#ifndef LAB_ISO_H
#define LAB_ISO_H

namespace lab {

// ISO7816 Default value for the frequency adjustment (Fi)
constexpr int ISO_7816_FI_DEF = 1; // 372

// ISO7816 Default value for the baud rate divisor adjustment (Di)
constexpr int ISO_7816_DI_DEF = 1; // 1

// ISO7816 Default maximum size of information field of blocks that can be received by the card (IFSC)
constexpr int ISO_7816_IFSC_DEF = 254;

// ISO7816 Default maximum size of information field of blocks that can be received by the interface device (IFSD)
constexpr int ISO_7816_IFSD_DEF = 254;

// ISO7816 Default character guard time (minimum delay between the leading edges of two consecutive characters) in ETUs
constexpr int ISO_7816_CGT_DEF = 12;

// ISO7816 Default character waiting Time (maximum delay between the leading edges of two consecutive characters) in ETUs
constexpr int ISO_7816_CWT_DEF = 9600;

// ISO7816 Default block guard time (minimum delay between the leading edge of last character and leading edge of next block) in ETUs
constexpr int ISO_7816_BGT_DEF = 22;

// ISO7816 Default block waiting Time (maximum delay between the leading edge of last character and next block) in ETUs
constexpr int ISO_7816_BWT_DEF = 9600;

// ISO7816 Default extra guard time in ETUs
constexpr int ISO_7816_EGT_DEF = 0;

// FN, frequency adjustment factor
constexpr int ISO_FI_TABLE[] = {0, 372, 558, 744, 1116, 1488, 1860, 0, 0, 512, 768, 1024, 1536, 2048, 0, 0};

// DN, baud rate adjustment factor
constexpr int ISO_DI_TABLE[] = {0, 1, 2, 4, 8, 16, 32, 64, 12, 20, 0, 0, 0, 0, 0, 0};

// Max frequency in MHz
constexpr int ISO_FM_TABLE[] = {0, 5000000, 6000000, 8000000, 12000000, 5000000, 5000000, 0, 0, 12000000, 12000000, 12000000, 12000000, 12000000, 0, 0};

// CWT, maximum delay between the leading edges of two consecutive characters in the block
constexpr int ISO_CWT_TABLE[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

// BWT, maximum delay between the leading edge of the last character of the block received by the card and the leading edge of the first character of the next block
constexpr int ISO_BWT_TABLE[] = {960, 1920, 3840, 7680, 15360, 30720, 61440, 122880, 245760, 491520, 0, 0, 0, 0, 0, 0};

// ISO7816 Default guard time (minimum delay between the leading edges of two consecutive characters is) units is ETU
constexpr const char *ISO_7816_CONVENTION_TABLE[] {"unknown", "direct", "inverse"};

}

#endif
