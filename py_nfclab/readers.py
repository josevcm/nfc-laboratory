"""
Simplified file readers for NFC-lab output formats.

Only supports two sources:
- TRZ files (tar.gz archives with frame.json)
- Live JSON streams (from stdin or nfc-lab --json-frames)

Both now use the same unified JSON format.
"""

import json
import tarfile
from pathlib import Path
from typing import Iterator, Optional, TextIO, Union

from .models import FrameFlags, FrameType, NFCFrame, TechType


class TRZReader:
    """
    Reader for TRZ (trace archive) files.

    TRZ files are tar.gz archives containing a frame.json file.
    """

    def __init__(self, file_path: Union[str, Path]):
        self.file_path = Path(file_path)
        if not self.file_path.exists():
            raise FileNotFoundError(f"TRZ file not found: {file_path}")

    def read_frames(self) -> Iterator[NFCFrame]:
        """Read frames from TRZ file."""
        with tarfile.open(self.file_path, "r:gz") as tar:
            json_member = tar.getmember("frame.json")
            json_file = tar.extractfile(json_member)
            if not json_file:
                return

            metadata = json.load(json_file)
            frames_data = metadata.get("frames", [])

            for frame_data in frames_data:
                # Convert old TRZ format to unified format
                yield self._normalize_trz_frame(frame_data)

    def _normalize_trz_frame(self, frame_data: dict) -> NFCFrame:
        """
        Normalize old TRZ format to match new live format.

        Old TRZ uses: frameData, techType, frameType, frameFlags
        New format uses: data, tech, type, flags
        """
        # Convert frameData "26:01:00" -> "260100"
        frame_data_str = frame_data.get("frameData", "")
        hex_data = frame_data_str.replace(":", "")

        # Convert techType int -> tech string
        tech_type_int = frame_data.get("techType", 0)
        try:
            tech_str = TechType(tech_type_int).name
        except ValueError:
            tech_str = "UNKNOWN"

        # Convert frameType int -> type string
        frame_type_int = frame_data.get("frameType", 0)
        try:
            frame_enum = FrameType(frame_type_int)
            type_map = {
                FrameType.NfcPollFrame: "Poll",
                FrameType.NfcListenFrame: "Listen",
                FrameType.NfcCarrierOn: "CarrierOn",
                FrameType.NfcCarrierOff: "CarrierOff",
            }
            frame_type_str = type_map.get(frame_enum, frame_enum.name)
        except ValueError:
            frame_type_str = "UNKNOWN"

        # Convert frameFlags int -> flags array
        frame_flags_int = frame_data.get("frameFlags", 0)
        flags_list = []

        flag_map = {
            FrameFlags.CrcError: "crc-error",
            FrameFlags.ParityError: "parity-error",
            FrameFlags.SyncError: "sync-error",
            FrameFlags.Truncated: "truncated",
            FrameFlags.Encrypted: "encrypted",
            FrameFlags.ShortFrame: "short-frame",
        }

        for flag_bit, flag_name in flag_map.items():
            if frame_flags_int & flag_bit:
                flags_list.append(flag_name)

        # Add request/response flags
        if frame_type_int == FrameType.NfcPollFrame:
            flags_list.append("request")
        elif frame_type_int == FrameType.NfcListenFrame:
            flags_list.append("response")

        # Build normalized dict matching new format
        normalized = {
            "timestamp": frame_data.get("timeStart", 0),
            "tech": tech_str,
            "type": frame_type_str,
            "length": len(bytes.fromhex(hex_data)) if hex_data else 0,
            "data": hex_data,
            "time_start": frame_data.get("timeStart"),
            "time_end": frame_data.get("timeEnd"),
            "flags": flags_list,
        }

        if frame_data.get("frameRate"):
            normalized["rate"] = frame_data["frameRate"]

        return NFCFrame.from_dict(normalized)


class LiveStreamReader:
    """
    Reader for live JSON streams (e.g., from stdin or nfc-lab --json-frames).

    Reads JSON objects line-by-line with the unified format.
    """

    def __init__(self, stream: Optional[TextIO] = None):
        import sys

        self.stream = stream or sys.stdin

    def read_frames(self) -> Iterator[NFCFrame]:
        """Read frames from stream line-by-line."""
        for line in self.stream:
            line = line.strip()

            # Skip empty lines and comments
            if not line or line.startswith("#"):
                continue

            try:
                yield NFCFrame.from_json(line)
            except json.JSONDecodeError:
                continue


def read_frames(source: Union[str, Path, TextIO]) -> Iterator[NFCFrame]:
    """
    Read frames from any source.

    Args:
        source:
            - Path to .trz file (string or Path)
            - TextIO stream (stdin, file handle, etc.)

    Yields:
        NFCFrame objects

    Examples:
        # From TRZ file
        for frame in read_frames("trace.trz"):
            print(frame)

        # From stdin (live)
        import sys
        for frame in read_frames(sys.stdin):
            print(frame)
    """
    reader: Union[TRZReader, LiveStreamReader]
    if isinstance(source, (str, Path)):
        source_path = Path(source)
        if not source_path.suffix == ".trz":
            raise ValueError(f"Only .trz files supported, got: {source_path.suffix}")

        reader = TRZReader(source_path)
    else:
        reader = LiveStreamReader(source)

    yield from reader.read_frames()
