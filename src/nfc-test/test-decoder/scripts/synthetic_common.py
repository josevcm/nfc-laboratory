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
Common helpers for synthetic capture generators: signal buffer with carrier levels, expected frames and
WAV / JSON output in the format read by test-decoder.
"""

import json
import os
import random
import struct
import sys
import wave

FC = 13.56e6
CARRIER_LEVEL = 0.5
NOISE_LEVEL = 0.002

# frame constants, see lab/data/RawFrame.h
POLL_FRAME = 0x0102
LISTEN_FRAME = 0x0103
SELECTION_PHASE = 0x0102
APPLICATION_PHASE = 0x0103
SHORT_FRAME = 0x01
TRUNCATED = 0x08
CRC_ERROR = 0x20
SYNC_ERROR = 0x40


def crc_ccitt(data, init, reflected):
    """CRC-16 CCITT, reflected (ISO/IEC 13239, final complement) or not (MSB first, no complement)"""
    crc = init
    for b in data:
        if reflected:
            crc ^= b
            for _ in range(8):
                crc = (crc >> 1) ^ 0x8408 if crc & 1 else crc >> 1
        else:
            crc ^= b << 8
            for _ in range(8):
                crc = ((crc << 1) ^ 0x1021) & 0xffff if crc & 0x8000 else (crc << 1) & 0xffff
    return crc ^ 0xffff if reflected else crc


class Capture:

    def __init__(self, tech, sample_rate=10_000_000):
        self.tech = tech
        self.sample_rate = sample_rate
        self.signal = []
        self.frames = []

    def samples(self, fc_units):
        """Length in samples of a time given in carrier cycles (1/fc)"""
        return fc_units / FC * self.sample_rate

    def idle(self, us):
        self.signal += [1.0] * int(us * 1e-6 * self.sample_rate)

    def levels(self, segments):
        """Append a list of (level, length in 1/fc) segments"""
        t = 0.0
        start = len(self.signal)
        total = sum(length for _, length in segments)
        out = [1.0] * int(self.samples(total) + 1)
        for level, length in segments:
            a, b = int(self.samples(t)), int(self.samples(t + length))
            for k in range(a, b):
                out[k] = level
            t += length
        self.signal += out

    def subcarrier(self, segments, period, depth):
        """Append (on/off, length in 1/fc) segments of load modulation with a square subcarrier of given period (1/fc)"""
        t = 0.0
        total = sum(length for _, length in segments)
        out = [1.0] * int(self.samples(total) + 1)
        half = self.samples(period / 2)
        for on, length in segments:
            a, b = int(self.samples(t)), int(self.samples(t + length))
            if on:
                for k in range(a, b):
                    out[k] = 1.0 + (depth if int((k - a) / half) % 2 == 0 else -depth)
            t += length
        self.signal += out

    def expect(self, frame_type, data, phase=APPLICATION_PHASE, flags=0):
        self.frames.append({
            "techType": self.tech,
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
            output.setframerate(self.sample_rate)
            output.writeframes(bytes(samples))

        with open(path + '.json', 'w') as output:
            json.dump({"description": description, "frames": self.frames}, output, indent=2)


def generate(scenarios, tech, doc):
    """Command line entry: generate all scenarios {name: (builder, sample_rate)} into the given directory"""
    if len(sys.argv) != 2:
        print(doc)
        return 1

    output = sys.argv[1]
    os.makedirs(output, exist_ok=True)

    for name, (build, sample_rate) in scenarios.items():
        capture = Capture(tech, sample_rate)
        capture.idle(500)
        description = build(capture)
        capture.idle(500)
        capture.write(os.path.join(output, name), description)
        print("generated", os.path.join(output, name + ".wav"))

    return 0
