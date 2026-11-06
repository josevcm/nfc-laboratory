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
Synthetic NFC-F (JIS X 6319-4 / FeliCa) capture generator for decoder regression tests.

Writes one <scenario>.wav (16 bit mono magnitude signal) and one <scenario>.json with the expected frames for
each scenario. The expected frames contain frame contents (without preamble and sync bytes), type, phase and
flags, not exact sample positions.

Signal model: Manchester coded ASK in both directions at 212 or 424 kbps, 48 bits preamble, sync bytes B2 4D,
LEN byte, payload and CRC-16 (MSB first, initial value 0), deterministic gaussian noise.

Usage: nfcf_synthetic.py <output-directory>
"""

import sys

from synthetic_common import *

NFC_F_TECH = 0x0103

READER_LEVEL = 0.70  # reader modulated level (30% ASK)
CARD_LEVEL = 0.72  # card load modulation seen by the sniffer


def frame(payload):
    """LEN + payload + CRC"""
    data = [len(payload) + 1] + list(payload)
    crc = crc_ccitt(data, 0x0000, False)
    return data + [crc >> 8, crc & 0xff]


def manchester(c, data, rate, low):
    """Preamble, sync and data bytes, MSB first, Manchester coded (bit 1: low then high)"""
    etu = 128 >> rate
    bits = []
    for b in [0x00] * 6 + [0xB2, 0x4D] + list(data):
        bits += [(b >> (7 - k)) & 1 for k in range(8)]
    segments = []
    for v in bits:
        segments += [(low, etu / 2), (1.0, etu / 2)] if v else [(1.0, etu / 2), (low, etu / 2)]
    c.levels(segments + [(1.0, etu)])


def poll(c, data, rate=1, phase=APPLICATION_PHASE, flags=0):
    manchester(c, data, rate, READER_LEVEL)
    c.expect(POLL_FRAME, data, phase, flags)


def listen(c, data, rate=1, delay_us=300, phase=APPLICATION_PHASE, flags=0, expected=True):
    c.idle(delay_us)
    manchester(c, data, rate, CARD_LEVEL)
    if expected:
        c.expect(LISTEN_FRAME, data, phase, flags)


IDM = [0x01, 0x2E, 0x45, 0x76, 0x89, 0xAB, 0xCD, 0xEF]
PMM = [0x03, 0x01, 0x4B, 0x02, 0x4F, 0x49, 0x93, 0xFF]


def polling(c, rate=1):
    poll(c, frame([0x00, 0xFF, 0xFF, 0x00, 0x00]), rate, SELECTION_PHASE)  # Polling, SC=FFFF, RC=0, TSN=0
    listen(c, frame([0x01] + IDM + PMM), rate, delay_us=2500, phase=SELECTION_PHASE)
    c.idle(500)


def read(c, rate=1, blocks=1):
    poll(c, frame([0x06] + IDM + [0x01, 0x0B, 0x00, blocks] + sum([[0x80, i] for i in range(blocks)], [])), rate)
    listen(c, frame([0x07] + IDM + [0x00, 0x00, blocks] + [i & 0xff for i in range(16 * blocks)]), rate)
    c.idle(500)


def scenario_basic(c):
    polling(c)
    read(c)
    return "Polling, polling response and one Read Without Encryption exchange at 212 kbps"


def scenario_basic_424(c):
    polling(c, rate=2)
    read(c, rate=2)
    return "Polling, polling response and one Read Without Encryption exchange at 424 kbps"


def scenario_max_length(c):
    polling(c)
    poll(c, frame([0x06] + IDM + [0x01, 0x0B, 0x00, 0x01, 0x80, 0x00]))
    listen(c, frame([0x07] + IDM + [0x00, 0x00, 0x0F] + [i & 0xff for i in range(254 - 12)]))  # LEN = 255
    c.idle(500)
    return "card frame with the maximum LEN of 255 bytes (259 bytes with sync and CRC)"


def scenario_spurious(c):
    polling(c)
    poll(c, frame([0x06] + IDM + [0x01, 0x0B, 0x00, 0x01, 0x80, 0x00]))
    c.idle(150)
    c.levels([(CARD_LEVEL, 32), (1.0, 32)] * 20)  # short burst of 20 Manchester bits, not a valid preamble
    listen(c, frame([0x07] + IDM + [0x00, 0x00, 0x01] + list(range(16))), delay_us=100)
    c.idle(500)
    return "spurious burst before the card response, the response must still be decoded"


def scenario_unanswered(c):
    poll(c, frame([0x00, 0xFF, 0xFF, 0x00, 0x00]), phase=SELECTION_PHASE)  # Polling without response
    c.idle(4000)
    poll(c, frame([0x06] + IDM + [0x01, 0x0B, 0x00, 0x01, 0x80, 0x00]))
    listen(c, frame([0x07] + IDM + [0x00, 0x00, 0x01] + list(range(16))))
    c.idle(500)
    return "unanswered Polling followed by a Read exchange, the response must not be classified as polling response"


SCENARIOS = {
    "nfcf_synthetic_basic": (scenario_basic, 10_000_000),
    "nfcf_synthetic_basic_424": (scenario_basic_424, 10_000_000),
    "nfcf_synthetic_max_length": (scenario_max_length, 10_000_000),
    "nfcf_synthetic_spurious": (scenario_spurious, 10_000_000),
    "nfcf_synthetic_unanswered": (scenario_unanswered, 10_000_000),
}

if __name__ == '__main__':
    sys.exit(generate(SCENARIOS, NFC_F_TECH, __doc__))
