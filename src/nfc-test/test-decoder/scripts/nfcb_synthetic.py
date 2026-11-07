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
Synthetic NFC-B 106 kbps capture generator for decoder regression tests.

Writes one <scenario>.wav (16 bit mono, 10 Msps magnitude signal) and one <scenario>.json with the expected
frames for each scenario. The expected frames only contain frame contents, type, phase and flags, so the
decoder test does not depend on exact sample positions.

Signal model (ISO/IEC 14443-2/3 type B):
  - reader (PCD): 10% ASK with NRZ-L coding, SoS (10.5 ETU low + 2.5 ETU high), characters with start and stop
    bits, EoS (10.5 ETU low)
  - card (PICC): BPSK of the fc/16 subcarrier with NRZ-L coding, TR1 phase reference, SoS, characters, EoS
  - deterministic gaussian noise

Usage: nfcb_synthetic.py <output-directory>
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
SUBCARRIER = 16 / FC * SAMPLE_RATE  # subcarrier period
CARRIER_LEVEL = 0.5
ASK_LOW_LEVEL = 0.82  # reader logic 0 amplitude, modulation index ~10%
SUBCARRIER_DEPTH = 0.04
NOISE_LEVEL = 0.002

# frame constants, see lab/data/RawFrame.h
NFC_B_TECH = 0x0102
POLL_FRAME = 0x0102
LISTEN_FRAME = 0x0103
SELECTION_PHASE = 0x0102
APPLICATION_PHASE = 0x0103
TRUNCATED = 0x08


def crc_b(data):
    """Append ISO/IEC 14443-3 CRC_B (ISO/IEC 13239) to data"""
    crc = 0xffff
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0x8408 if crc & 1 else crc >> 1
    crc ^= 0xffff
    return list(data) + [crc & 0xff, crc >> 8]


def character_bits(data, egt=0):
    """Start bit, 8 data bits LSB first, stop bit and optional extra guard time (logic 1) for each byte"""
    bits = []
    for i, b in enumerate(data):
        bits += [0] + [(b >> k) & 1 for k in range(8)] + [1]
        if i < len(data) - 1:
            bits += [1] * egt
    return bits


class Capture:

    def __init__(self):
        self.signal = []
        self.frames = []

    def idle(self, us):
        self.signal += [1.0] * int(us * 1e-6 * SAMPLE_RATE)

    def _levels(self, levels):
        """Append a list of (logic value, length in ETU) as reader ASK signal"""
        position = len(self.signal)
        total = sum(length for _, length in levels)
        out = [1.0] * int(total * ETU + 1)
        t = 0.0
        for value, length in levels:
            for k in range(int(t), int(t + length * ETU)):
                out[k] = 1.0 if value else ASK_LOW_LEVEL
            t += length * ETU
        self.signal += out

    def poll(self, data, phase=APPLICATION_PHASE, flags=0, expected=None):
        """Reader frame: SoS, characters, EoS"""
        levels = [(0, 10.5), (1, 2.5)] + [(v, 1) for v in character_bits(data)] + [(0, 10.5), (1, 1)]
        self._levels(levels)
        self.expect(POLL_FRAME, data if expected is None else expected, phase, flags)

    def listen(self, data, delay_us=81, tr1=1400, egt=0, eos=True, phase=APPLICATION_PHASE, flags=0, expected=None):
        """Card frame starting delay_us after reader EoS: TR1 subcarrier, SoS, characters, EoS"""
        self.idle(delay_us - 1 * ETU / SAMPLE_RATE * 1e6)
        symbols = [(0, 10.5), (1, 2.5)] + [(v, 1) for v in character_bits(data, egt)] + ([(0, 10.5)] if eos else [])
        tr1_etu = tr1 / 128
        total = tr1_etu + sum(length for _, length in symbols)
        signs = [1] * int(total * ETU + 1)
        t = tr1_etu * ETU
        for value, length in symbols:
            if not value:
                for k in range(int(t), int(t + length * ETU)):
                    signs[k] = -1
            t += length * ETU
        half = SUBCARRIER / 2
        out = [1.0 + SUBCARRIER_DEPTH * (1 if (k / half) % 2 < 1 else -1) * signs[k] for k in range(len(signs) - 1)]
        out.append(1.0)
        self.signal += out
        if expected is not False:
            self.expect(LISTEN_FRAME, data if expected is None else expected, phase, flags)

    def expect(self, frame_type, data, phase, flags):
        self.frames.append({
            "techType": NFC_B_TECH,
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


def atqb(fsci=8, fwi=4):
    """ATQB: 0x50, PUPI, application data, protocol info (bit rate, FSCI + protocol type, FWI + ADC + FO)"""
    return crc_b([0x50, 0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, (fsci << 4) | 0x01, (fwi << 4) | 0x01])


def attrib(fsdi=8, tr1=0):
    """ATTRIB: 0x1D, PUPI, param 1 (TR0, TR1, EoS, SoS), param 2 (bit rates + FSDI), param 3, param 4 (CID)"""
    return crc_b([0x1D, 0x11, 0x22, 0x33, 0x44, tr1 << 4, fsdi & 0x0f, 0x01, 0x00])


def activation(c, fsci=8, fwi=4, fsdi=8, tr1=0, attrib_tr1=1400):
    c.poll(crc_b([0x05, 0x00, 0x00]), phase=SELECTION_PHASE)  # REQB
    c.listen(atqb(fsci, fwi), phase=SELECTION_PHASE)
    c.idle(300)
    c.poll(attrib(fsdi, tr1), phase=SELECTION_PHASE)
    c.listen(crc_b([0x00]), tr1=attrib_tr1, phase=SELECTION_PHASE)
    c.idle(300)


def apdu(c, tr1=1400):
    c.poll(crc_b([0x02, 0x00, 0xA4]))
    c.listen(crc_b([0x02, 0x90, 0x00]), tr1=tr1)
    c.idle(300)


def scenario_basic(c):
    activation(c)
    apdu(c)
    return "full activation: REQB, ATQB, ATTRIB, answer to ATTRIB and one I-block exchange"


def scenario_fsd_4096(c):
    activation(c, fsci=0x0C, fsdi=0x0C)
    c.poll(crc_b([0x02] + [i & 0xff for i in range(700)]))
    c.listen(crc_b([0x02] + [(3 * i) & 0xff for i in range(600)] + [0x90, 0x00]))
    c.idle(300)
    return "FSC and FSD of 4096 bytes followed by 703 and 605 bytes frames (frame capacity of 512 bytes)"


def scenario_fsdi_rfu(c):
    activation(c, fsdi=0x0F)
    apdu(c)
    return "ATTRIB with RFU FSDI=F must be handled as FSDI=C (decoder locked by zero frame size)"


def scenario_frame_size(c):
    activation(c, fsci=8, fsdi=0)
    c.poll(crc_b([0x02] + list(range(17))))  # 20 bytes reader frame, limited by FSC (256), not by FSD (16)
    c.listen(crc_b([0x02] + list(range(13))))  # 16 bytes card frame, exactly FSD
    c.idle(300)
    return "FSD=16 limits card frames only, card frame of exactly FSD bytes is not truncated"


def scenario_egt(c):
    activation(c)
    c.poll(crc_b([0x02, 0x00, 0xA4]))
    c.listen(crc_b([0x02, 0x90, 0x00]), egt=2)
    c.idle(300)
    return "card frame with 2 ETU extra guard time between characters"


def scenario_tr1_short(c):
    activation(c, tr1=1, attrib_tr1=1100)
    apdu(c, tr1=1100)
    return "ATTRIB with TR1 = 64/fs, card responses use a shorter phase reference"


def scenario_wtx(c):
    activation(c)
    c.poll(crc_b([0x02, 0x00, 0xA4]))
    c.listen(crc_b([0xF2, 0x04]))  # S(WTX) request, WTXM=4
    c.idle(300)
    c.poll(crc_b([0xF2, 0x04]))  # S(WTX) response
    c.listen(crc_b([0x02, 0x90, 0x00]), delay_us=12000)  # after 12 ms, over FWT (FWI=4) but within FWT * WTXM
    c.idle(300)
    return "S(WTX) waiting time extension, response arrives after FWT but before FWT * WTXM"


def scenario_hltb(c):
    c.poll(crc_b([0x05, 0x00, 0x00]), phase=SELECTION_PHASE)
    c.listen(atqb(), phase=SELECTION_PHASE)
    c.idle(300)
    c.poll(crc_b([0x50, 0x11, 0x22, 0x33, 0x44]), phase=SELECTION_PHASE)  # HLTB
    c.listen(crc_b([0x00]), phase=SELECTION_PHASE)
    c.idle(300)
    return "HLTB command and its answer are part of the selection phase"


def scenario_slot_marker(c):
    c.poll(crc_b([0x05, 0x00, 0x02]), phase=SELECTION_PHASE)  # REQB with 4 slots, no answer in first slot
    c.idle(1000)
    c.poll(crc_b([0x15]), phase=SELECTION_PHASE)  # Slot-MARKER for slot 2
    c.listen(atqb(), phase=SELECTION_PHASE)
    c.idle(300)
    return "Slot-MARKER is part of the selection phase and is answered with ATQB"


def scenario_no_eos(c):
    activation(c)
    c.poll(crc_b([0x02, 0x00, 0xA4]))
    c.listen(crc_b([0x02, 0x90, 0x00]), eos=False)  # subcarrier stops after last character, EoS suppressed
    c.idle(300)
    c.poll(crc_b([0x03, 0x00, 0xB0]))
    c.listen(crc_b([0x03, 0x90, 0x00]))
    c.idle(300)
    return "card frame without EoS (subcarrier stops after the last character) followed by a normal exchange"


SCENARIOS = {
    "nfcb_synthetic_basic": scenario_basic,
    "nfcb_synthetic_fsd_4096": scenario_fsd_4096,
    "nfcb_synthetic_fsdi_rfu": scenario_fsdi_rfu,
    "nfcb_synthetic_frame_size": scenario_frame_size,
    "nfcb_synthetic_egt": scenario_egt,
    "nfcb_synthetic_tr1_short": scenario_tr1_short,
    "nfcb_synthetic_wtx": scenario_wtx,
    "nfcb_synthetic_hltb": scenario_hltb,
    "nfcb_synthetic_slot_marker": scenario_slot_marker,
    "nfcb_synthetic_no_eos": scenario_no_eos,
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
