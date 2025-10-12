"""
NFC Laboratory Python Library

Complete Python library for reading and analyzing NFC trace files from nfc-lab.
Fully compatible with TRZ file format specification.

Example usage:
    from py_nfclab import read_trz, write_json

    # Read TRZ file
    frames = read_trz("trace.trz")

    # Analyze frames
    for frame in frames:
        if frame.is_poll() and frame.tech == "NfcA":
            print(f"{frame.datetime_str}: {frame.data.hex()}")

    # Export to JSON
    write_json(frames, "output.json")
"""

__version__ = "1.0.0"

# Core data models
from .models import FrameFlags, FramePhase, FrameType, NFCFrame, TechType

# Protocol detection
from .protocol import (
    NFC_V_COMMANDS,
    NFC_V_ERROR_CODES,
    NfcVFlags,
    NfcVFrame,
    detect_command,
    parse_nfcv_flags,
    parse_nfcv_frame,
)

# Readers
from .readers import TRZReader, read_trz

# Writers
from .writers import frames_to_json, write_json, write_jsonl

__all__ = [
    # Models
    "NFCFrame",
    "TechType",
    "FrameType",
    "FramePhase",
    "FrameFlags",
    # Readers
    "TRZReader",
    "read_trz",
    # Writers
    "write_json",
    "write_jsonl",
    "frames_to_json",
    # Protocol
    "detect_command",
    "parse_nfcv_flags",
    "parse_nfcv_frame",
    "NfcVFlags",
    "NfcVFrame",
    "NFC_V_COMMANDS",
    "NFC_V_ERROR_CODES",
]
