#!/usr/bin/env python3
#
# This file is part of NFC-LABORATORY.
#
# Copyright (C) 2026 Jose Vicente Campos Martinez, <josevcm@gmail.com>
#
# NFC-LABORATORY is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# NFC-LABORATORY is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

"""
Synthetic NFC-V (ISO/IEC 15693) capture generator for decoder regression tests.

Writes one <scenario>.wav (16 bit mono magnitude signal) and one <scenario>.json with the expected frames for
each scenario. The expected frames contain frame contents, type, phase and flags, not exact sample positions.

Signal model:
  - reader (VCD): 100% ASK pauses of 128/fc with pulse position coding, 1 out of 4 or 1 out of 256
  - card (VICC): single fc/32 subcarrier, high data rate, Manchester coding, SOF and EOF
  - deterministic gaussian noise

Usage: nfcv_synthetic.py <output-directory>
"""

import sys

from synthetic_common import *

NFC_V_TECH = 0x0104

PAUSE = 128  # reader pause length, 1/fc
SLOT = 256  # pulse position slot length, 1/fc
SUBCARRIER = 32  # card subcarrier period, 1/fc
SUBCARRIER_DEPTH = 0.04


def crc_v(data):
    crc = crc_ccitt(data, 0xFFFF, True)
    return list(data) + [crc & 0xff, crc >> 8]


def poll(c, data, code=4, phase=APPLICATION_PHASE, flags=0):
    """Reader frame: SOF, pulse position symbols, EOF"""
    pauses = []  # pause start times, 1/fc
    t = 0
    pauses += [0, 5 * PAUSE if code == 4 else 7 * PAUSE]  # SOF 1 out of 4 or 1 out of 256
    t = 8 * PAUSE
    bits = 2 if code == 4 else 8
    for b in data:
        for k in range(0, 8, bits):
            v = (b >> k) & ((1 << bits) - 1)
            pauses.append(t + v * SLOT + SLOT // 2)
            t += (1 << bits) * SLOT
    pauses.append(t + 2 * PAUSE)  # EOF
    t += 4 * PAUSE
    segments, last = [], 0
    for p in pauses:
        segments += [(1.0, p - last), (0.0, PAUSE)]
        last = p + PAUSE
    segments.append((1.0, t - last))
    c.levels(segments)
    c.expect(POLL_FRAME, data, phase, flags)


def listen(c, data, delay=4320, phase=APPLICATION_PHASE, flags=0):
    """Card frame delay (1/fc) after reader EOF: SOF, Manchester bits LSB first, EOF"""
    c.levels([(1.0, delay)])
    segments = [(0, 768), (1, 768), (0, 256), (1, 256)]  # SOF: unmodulated, 24 pulses, logic 1
    for b in data:
        for k in range(8):
            segments += [(0, 256), (1, 256)] if (b >> k) & 1 else [(1, 256), (0, 256)]
    segments += [(1, 256), (0, 256), (1, 768), (0, 768)]  # EOF: logic 0, 24 pulses, unmodulated
    c.subcarrier(segments, SUBCARRIER, SUBCARRIER_DEPTH)
    c.expect(LISTEN_FRAME, data, phase, flags)


UID = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x01, 0xE0]


def inventory(c, code=4):
    poll(c, crc_v([0x26, 0x01, 0x00]), code)  # Inventory, 1 slot, high data rate
    listen(c, crc_v([0x00, 0x00] + UID))
    c.idle(300)


def read_block(c, code=4):
    poll(c, crc_v([0x22, 0x20] + UID + [0x00]), code)  # Read Single Block, addressed
    listen(c, crc_v([0x00, 0x01, 0x02, 0x03, 0x04]))
    c.idle(300)


def scenario_basic(c):
    inventory(c)
    read_block(c)
    return "Inventory and Read Single Block exchanges with 1 out of 4 coding"


def scenario_code256(c):
    inventory(c, code=256)
    read_block(c, code=256)
    return "Inventory and Read Single Block exchanges with 1 out of 256 coding"


def scenario_large_response(c):
    inventory(c)
    poll(c, crc_v([0x22, 0x23] + UID + [0x00, 0x7F]))  # Read Multiple Blocks, 128 blocks
    listen(c, crc_v([0x00] + [(7 * i) & 0xff for i in range(128 * 4)]))  # 515 bytes response
    c.idle(300)
    return "Read Multiple Blocks with a 515 bytes response"


def scenario_rate_20m(c):
    inventory(c)
    read_block(c)
    return "Inventory and Read Single Block at 20 Msps"


SCENARIOS = {
    "nfcv_synthetic_basic": (scenario_basic, 10_000_000),
    "nfcv_synthetic_code256": (scenario_code256, 10_000_000),
    "nfcv_synthetic_large_response": (scenario_large_response, 10_000_000),
    "nfcv_synthetic_rate_20m": (scenario_rate_20m, 20_000_000),
}

if __name__ == '__main__':
    sys.exit(generate(SCENARIOS, NFC_V_TECH, __doc__))
