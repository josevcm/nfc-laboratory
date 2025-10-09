"""
NFC Laboratory Python Library

A library for parsing and analyzing NFC frame data from nfc-lab.
Provides clean separation between data capture and interpretation.

Simplified to support only two sources:
- TRZ files (archived traces)
- Live JSON streams (from nfc-lab --json-frames)
"""

__version__ = "0.2.0"

from .models import FrameFlags, FrameType, NFCFrame, NFCTransaction, TechType
from .protocol import ProtocolParser
from .readers import LiveStreamReader, TRZReader, read_frames
from .transactions import TransactionBuilder

__all__ = [
    "NFCFrame",
    "NFCTransaction",
    "TechType",
    "FrameType",
    "FrameFlags",
    "ProtocolParser",
    "TransactionBuilder",
    "TRZReader",
    "LiveStreamReader",
    "read_frames",
]
