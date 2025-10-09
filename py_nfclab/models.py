"""
Data models for NFC frames and transactions.

This module defines the core data structures used throughout the library.
Based on the C++ implementation in RawFrame.h and StreamModel.cpp.
"""

import json
from dataclasses import dataclass, field
from enum import IntEnum
from typing import List, Optional


class TechType(IntEnum):
    """NFC Technology Types (from RawFrame.h)"""

    NoneTech = 0x0000
    # NFC tech types
    NfcAnyTech = 0x0100
    NfcA = 0x0101
    NfcB = 0x0102
    NfcF = 0x0103
    NfcV = 0x0104
    # ISO tech types
    IsoAnyTech = 0x0200
    Iso7816 = 0x0201


class FrameType(IntEnum):
    """Frame Types (from RawFrame.h)"""

    # NFC Frame types
    NfcCarrierOff = 0x0100
    NfcCarrierOn = 0x0101
    NfcPollFrame = 0x0102
    NfcListenFrame = 0x0103
    # ISO Frame types
    IsoVccLow = 0x0200
    IsoVccHigh = 0x0201
    IsoRstLow = 0x0202
    IsoRstHigh = 0x0203
    IsoATRFrame = 0x0210
    IsoRequestFrame = 0x0211
    IsoResponseFrame = 0x0212
    IsoExchangeFrame = 0x0213


class FramePhase(IntEnum):
    """Frame Phases (from RawFrame.h)"""

    # NFC Frame phases
    NfcAnyPhase = 0x0100
    NfcCarrierPhase = 0x0101
    NfcSelectionPhase = 0x0102
    NfcApplicationPhase = 0x0103
    # ISO Frame phases
    IsoAnyPhase = 0x0200


class FrameFlags(IntEnum):
    """Frame Flags (from RawFrame.h)"""

    ShortFrame = 0x01
    Encrypted = 0x02
    Truncated = 0x08
    ParityError = 0x10
    CrcError = 0x20
    SyncError = 0x40


@dataclass
class NFCFrame:
    """
    Represents a single NFC frame.

    This is the core data structure that holds all information about a frame,
    separated from its interpretation. Based on lab::RawFrame from the C++ code.
    """

    # Timing information
    timestamp: float
    time_start: Optional[float] = None
    time_end: Optional[float] = None
    delta: Optional[float] = None

    # Frame classification
    tech: str = "UNKNOWN"  # String representation (e.g., "NfcA", "NfcV")
    tech_type: Optional[TechType] = None  # Enum value
    frame_type: str = "UNKNOWN"  # String representation (e.g., "Poll", "Listen")
    frame_type_enum: Optional[FrameType] = None  # Enum value
    phase: Optional[FramePhase] = None

    # Frame data
    data: bytes = field(default_factory=bytes)
    data_hex: str = ""
    length: int = 0

    # Protocol information
    rate: Optional[int] = None
    flags: List[str] = field(default_factory=list)
    flags_enum: int = 0
    errors: List[str] = field(default_factory=list)

    # Interpretation (filled by protocol parser)
    event: Optional[str] = None
    parsed_data: Optional[dict] = None

    # Original JSON for reference
    raw_json: Optional[dict] = None

    @classmethod
    def from_json(cls, json_str: str) -> "NFCFrame":
        """Create NFCFrame from JSON string (as output by nfc-lab --json-frames)"""
        data = json.loads(json_str)
        return cls.from_dict(data)

    @classmethod
    def from_dict(cls, data: dict) -> "NFCFrame":
        """Create NFCFrame from dictionary (works for both live and TRZ format)"""
        hex_data = data.get("data", "")
        errors_list = list(data.get("errors", []))

        try:
            data_bytes = bytes.fromhex(hex_data) if hex_data else bytes()
        except ValueError:
            data_bytes = bytes()
            errors_list.append("invalid-hex")

        tech_str = data.get("tech", "UNKNOWN")
        tech_enum = (
            getattr(TechType, tech_str, None) if hasattr(TechType, tech_str) else None
        )

        # Map frame type string to enum
        frame_type_str = data.get("type", "UNKNOWN")
        frame_type_map = {
            "Poll": FrameType.NfcPollFrame,
            "Listen": FrameType.NfcListenFrame,
            "CarrierOn": FrameType.NfcCarrierOn,
            "CarrierOff": FrameType.NfcCarrierOff,
        }
        frame_type_enum = frame_type_map.get(frame_type_str)

        # Parse flags array to bitfield
        flags_list = data.get("flags", [])
        flag_map = {
            "crc-error": FrameFlags.CrcError,
            "parity-error": FrameFlags.ParityError,
            "sync-error": FrameFlags.SyncError,
            "truncated": FrameFlags.Truncated,
            "encrypted": FrameFlags.Encrypted,
            "short-frame": FrameFlags.ShortFrame,
        }
        flags_bits = sum(flag_map.get(f, 0) for f in flags_list)

        return cls(
            timestamp=data.get("timestamp", 0),
            time_start=data.get("time_start"),
            time_end=data.get("time_end"),
            tech=tech_str,
            tech_type=tech_enum,
            frame_type=frame_type_str,
            frame_type_enum=frame_type_enum,
            data=data_bytes,
            data_hex=hex_data,
            length=data.get("length", len(data_bytes)),
            rate=data.get("rate"),
            flags=flags_list,
            flags_enum=flags_bits,
            errors=errors_list,
            raw_json=data,
        )

    def has_flag(self, flag: FrameFlags) -> bool:
        """Check if frame has a specific flag set"""
        return bool(self.flags_enum & flag)

    def is_poll(self) -> bool:
        """Check if this is a poll/request frame"""
        return self.frame_type in ["Poll", "Request"] or self.frame_type_enum in [
            FrameType.NfcPollFrame,
            FrameType.IsoRequestFrame,
        ]

    def is_listen(self) -> bool:
        """Check if this is a listen/response frame"""
        return self.frame_type in ["Listen", "Response"] or self.frame_type_enum in [
            FrameType.NfcListenFrame,
            FrameType.IsoResponseFrame,
        ]

    def __repr__(self) -> str:
        return (
            f"NFCFrame(timestamp={self.timestamp:.6f}, tech={self.tech}, "
            f"type={self.frame_type}, event={self.event}, length={self.length})"
        )


@dataclass
class NFCTransaction:
    """
    Represents a transaction: a Poll frame paired with its Listen response.

    This enables higher-level protocol analysis by grouping related frames together.
    """

    poll: NFCFrame
    listen: Optional[NFCFrame] = None

    # Transaction timing
    start_time: float = 0
    end_time: Optional[float] = None
    duration: Optional[float] = None

    # Transaction interpretation
    command: Optional[str] = None
    response_status: Optional[str] = None
    parsed_transaction: Optional[dict] = None

    def __post_init__(self):
        self.start_time = self.poll.timestamp
        self.command = self.poll.event
        if self.listen:
            self.end_time = self.listen.timestamp
            self.duration = self.end_time - self.start_time

    def is_complete(self) -> bool:
        """Check if transaction has both poll and listen frames"""
        return self.listen is not None

    def has_error(self) -> bool:
        """Check if either frame in the transaction has errors"""
        error_flags = (
            FrameFlags.CrcError,
            FrameFlags.ParityError,
            FrameFlags.SyncError,
        )
        if self.poll.errors or any(self.poll.has_flag(f) for f in error_flags):
            return True
        if self.listen and (
            self.listen.errors or any(self.listen.has_flag(f) for f in error_flags)
        ):
            return True
        return False

    def __repr__(self) -> str:
        status = "complete" if self.is_complete() else "incomplete"
        return (
            f"NFCTransaction(time={self.start_time:.6f}, command={self.command}, "
            f"status={status}, has_response={self.listen is not None})"
        )
