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
Synthetic NFC-A 106 kbps capture generator for decoder regression tests.

Writes one <scenario>.wav (16 bit mono, 10 Msps magnitude signal) and one <scenario>.json with the expected
frames for each scenario. The expected frames only contain frame contents, type, phase and flags, so the
decoder test does not depend on exact sample positions.

Signal model:
  - reader (PCD): 100% ASK pauses of 2.5 us with modified Miller coding
  - card (PICC): load modulation with fc/16 subcarrier (OOK) and Manchester coding
  - deterministic gaussian noise

Usage: nfca_synthetic.py <output-directory>
"""

import json
import os
import random
import struct
import sys
import wave

SAMPLE_RATE = 10_000_000
FC = 13.56e6
ETU = 128 / FC * SAMPLE_RATE  # samples per bit
PAUSE = 2.5e-6 * SAMPLE_RATE  # reader pause length
SUBCARRIER = 16 / FC * SAMPLE_RATE  # subcarrier period
CARRIER_LEVEL = 0.5
SUBCARRIER_DEPTH = 0.04
NOISE_LEVEL = 0.002

# frame constants, see lab/data/RawFrame.h
NFC_A_TECH = 0x0101
POLL_FRAME = 0x0102
LISTEN_FRAME = 0x0103
SELECTION_PHASE = 0x0102
APPLICATION_PHASE = 0x0103
SHORT_FRAME = 0x01


def crc_a(data):
    """Append ISO/IEC 14443-3 CRC_A to data"""
    crc = 0x6363
    for b in data:
        b ^= crc & 0xff
        b = (b ^ (b << 4)) & 0xff
        crc = ((crc >> 8) ^ (b << 8) ^ (b << 3) ^ (b >> 4)) & 0xffff
    return list(data) + [crc & 0xff, crc >> 8]


def frame_bits(data, bits=None):
    """Bit sequence LSB first with odd parity per byte, or raw bits for short frames"""
    if bits is not None:
        return [(data[0] >> i) & 1 for i in range(bits)]
    sequence = []
    for b in data:
        value = [(b >> i) & 1 for i in range(8)]
        sequence += value + [1 - sum(value) % 2]
    return sequence


class Capture:

    def __init__(self):
        self.signal = []
        self.frames = []

    def idle(self, us):
        self.signal += [1.0] * int(us * 1e-6 * SAMPLE_RATE)

    def poll(self, data, bits=None, phase=APPLICATION_PHASE, flags=0):
        """Reader frame: SoF (Z), data bits, EoF (logic 0 followed by Y)"""
        patterns = ['Z']
        previous = 'Z'
        for value in frame_bits(data, bits) + [0]:
            pattern = 'X' if value else ('Y' if previous == 'X' else 'Z')
            patterns.append(pattern)
            previous = pattern
        patterns.append('Y')

        out = [1.0] * (int(len(patterns) * ETU) + 1)
        for i, pattern in enumerate(patterns):
            if pattern in ('X', 'Z'):
                start = i * ETU + (ETU / 2 if pattern == 'X' else 0)
                for k in range(int(start), int(start + PAUSE)):
                    out[k] = 0.0
        self.signal += out
        self.expect(POLL_FRAME, data, phase, flags)

    def listen(self, data, bits=None, delay_us=91.15, phase=APPLICATION_PHASE, flags=0, expected=True):
        """Card frame after delay_us from reader EoF: SoF (D), Manchester data bits, EoF (no modulation)"""
        self.idle(delay_us - 2 * ETU / SAMPLE_RATE * 1e6)
        sequence = [1] + frame_bits(data, bits)

        out = [1.0] * (int((len(sequence) + 1) * ETU) + 1)
        for i, value in enumerate(sequence):
            start = i * ETU + (0 if value else ETU / 2)
            for k in range(int(start), int(start + ETU / 2)):
                high = int((k - start) / (SUBCARRIER / 2)) % 2 == 0
                out[k] = 1.0 + (SUBCARRIER_DEPTH if high else -SUBCARRIER_DEPTH)
        self.signal += out
        if expected:
            self.expect(LISTEN_FRAME, data, phase, flags)

    def expect(self, frame_type, data, phase, flags):
        self.frames.append({
            "techType": NFC_A_TECH,
            "frameType": frame_type,
            "framePhase": phase,
            "frameFlags": flags,
            "frameData": ":".join("%02X" % b for b in data)
        })

    def write(self, path, description):
        rng = random.Random(1)
        samples = bytearray()
        for value in self.signal:
            level = max(-1.0, min(1.0, CARRIER_LEVEL * value + rng.gauss(0, NOISE_LEVEL)))
            samples += struct.pack('<h', int(level * 32767))

        with wave.open(path + '.wav', 'wb') as output:
            output.setnchannels(1)
            output.setsampwidth(2)
            output.setframerate(SAMPLE_RATE)
            output.writeframes(bytes(samples))

        with open(path + '.json', 'w') as output:
            json.dump({"description": description, "frames": self.frames}, output, indent=2)


def activation(capture):
    """REQA / ATQA exchange"""
    capture.poll([0x26], bits=7, phase=SELECTION_PHASE, flags=SHORT_FRAME)
    capture.listen([0x04, 0x00], phase=SELECTION_PHASE)
    capture.idle(300)


def rats(capture, param, ats):
    capture.poll(crc_a([0xE0, param]), phase=SELECTION_PHASE)
    if ats is not None:
        capture.listen(ats, phase=SELECTION_PHASE)
    capture.idle(300)


def apdu(capture):
    capture.poll(crc_a([0x02, 0x00, 0xA4]))
    capture.listen(crc_a([0x02, 0x90, 0x00]))
    capture.idle(300)


ATS = crc_a([0x05, 0x78, 0x80, 0x70, 0x02])  # TL=5, T0 with TA, TB, TC and FSCI=8


def scenario_basic(c):
    activation(c)
    rats(c, 0x80, ATS)
    apdu(c)
    return "full activation: REQA, ATQA, RATS, ATS and one I-block exchange"


def scenario_fsdi_4096(c):
    activation(c)
    rats(c, 0xC0, None)
    c.idle(6000)
    c.poll(crc_a([0x02] + [i & 0xff for i in range(700)]))
    c.idle(300)
    activation(c)
    return "RATS with FSDI=C (4096 bytes) followed by a 703 bytes reader frame (stream buffer overflow)"


def scenario_fsdi_rfu(c):
    activation(c)
    rats(c, 0xF0, ATS)
    apdu(c)
    return "RATS with RFU FSDI=F must be handled as FSDI=C (decoder locked by zero frame size)"


def scenario_ats_short(c):
    activation(c)
    rats(c, 0x80, crc_a([0x05, 0x78]))
    apdu(c)
    return "ATS shorter than announced TL, interface bytes must not be read from CRC"


def scenario_sblock(c):
    activation(c)
    c.poll(crc_a([0xC2]))
    c.listen(crc_a([0xC2]))
    c.idle(300)
    return "S(DESELECT) request and response of 3 bytes"


def scenario_wtx(c):
    activation(c)
    rats(c, 0x80, crc_a([0x02, 0x78, 0x80, 0x40]))  # ATS with FWI=4 (FWT ~4.8 ms)
    c.poll(crc_a([0x02, 0x00, 0xA4]))
    c.listen(crc_a([0xF2, 0x04]))  # S(WTX) request, WTXM=4
    c.idle(300)
    c.poll(crc_a([0xF2, 0x04]))  # S(WTX) response
    c.listen(crc_a([0x02, 0x90, 0x00]), delay_us=12000)  # response after 12 ms, over FWT but within FWT * WTXM
    c.idle(300)
    return "S(WTX) waiting time extension, response arrives after FWT but before FWT * WTXM"


def scenario_spurious(c):
    c.poll([0x26], bits=7, phase=SELECTION_PHASE, flags=SHORT_FRAME)
    c.listen([0x03], bits=2, expected=False)  # 2 bits burst, not a valid frame
    c.listen([0x04, 0x00], delay_us=45, phase=SELECTION_PHASE)
    c.idle(300)
    return "spurious 2 bits burst before ATQA, partial bits must be discarded"


SCENARIOS = {
    "nfca_synthetic_basic": scenario_basic,
    "nfca_synthetic_fsdi_4096": scenario_fsdi_4096,
    "nfca_synthetic_fsdi_rfu": scenario_fsdi_rfu,
    "nfca_synthetic_ats_short": scenario_ats_short,
    "nfca_synthetic_sblock": scenario_sblock,
    "nfca_synthetic_wtx": scenario_wtx,
    "nfca_synthetic_spurious": scenario_spurious,
}


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 1

    output = sys.argv[1]
    os.makedirs(output, exist_ok=True)

    for name, build in SCENARIOS.items():
        capture = Capture()
        capture.idle(500)
        description = build(capture)
        capture.idle(500)
        capture.write(os.path.join(output, name), description)
        print("generated", os.path.join(output, name + ".wav"))

    return 0


if __name__ == '__main__':
    sys.exit(main())
