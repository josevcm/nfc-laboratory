"""
NFC Frame Data Models

Complete implementation of NFC frame data structures matching the TRZ file format.
Based on C++ RawFrame implementation in TraceStorageTask.cpp and RawFrame.h.
"""

from dataclasses import dataclass, field
from enum import IntEnum
from typing import Any, Dict, List, Optional


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
    Complete NFC frame representation matching TRZ format specification.

    All fields from TRZ frame.json are preserved.
    """

    # Sample-level timing (from TRZ)
    sample_start: int
    sample_end: int
    sample_rate: int

    # Time information
    time_start: float
    time_end: float
    date_time: float  # Absolute Unix timestamp

    # Frame classification
    tech_type: int
    frame_type: int
    frame_phase: int

    # Frame properties
    frame_rate: int  # Bitrate in bps
    frame_flags: int  # Bitmask

    # Frame data
    frame_data: bytes = field(default_factory=bytes)

    # Computed/derived fields
    _errors: Optional[List[str]] = field(default=None, repr=False)
    _tech_name: Optional[str] = field(default=None, repr=False)
    _type_name: Optional[str] = field(default=None, repr=False)
    _phase_name: Optional[str] = field(default=None, repr=False)

    def __post_init__(self):
        """Compute derived fields after initialization"""
        if self._errors is None:
            self._errors = self._decode_errors()
        if self._tech_name is None:
            self._tech_name = self._decode_tech()
        if self._type_name is None:
            self._type_name = self._decode_type()
        if self._phase_name is None:
            self._phase_name = self._decode_phase()

    # Properties for convenient access
    @property
    def timestamp(self) -> float:
        """Primary timestamp (time_start)"""
        return self.time_start

    @property
    def duration(self) -> float:
        """Frame duration in seconds"""
        return self.time_end - self.time_start

    @property
    def data(self) -> bytes:
        """Frame data payload"""
        return self.frame_data

    @property
    def length(self) -> int:
        """Frame data length in bytes"""
        return len(self.frame_data)

    @property
    def errors(self) -> List[str]:
        """Decoded error flags"""
        return self._errors if self._errors is not None else []

    @property
    def tech(self) -> str:
        """Technology name (e.g., 'NfcA')"""
        return self._tech_name if self._tech_name is not None else "Unknown"

    @property
    def type(self) -> str:
        """Frame type name (e.g., 'Poll')"""
        return self._type_name if self._type_name is not None else "Unknown"

    @property
    def phase(self) -> str:
        """Frame phase name (e.g., 'SelectionPhase')"""
        return self._phase_name if self._phase_name is not None else "Unknown"

    @property
    def rate(self) -> int:
        """Bitrate in bps"""
        return self.frame_rate

    def _decode_errors(self) -> List[str]:
        """Decode frame_flags bitmask to error list"""
        errors = []
        if self.frame_flags & FrameFlags.CrcError:
            errors.append("CRC")
        if self.frame_flags & FrameFlags.ParityError:
            errors.append("PARITY")
        if self.frame_flags & FrameFlags.SyncError:
            errors.append("SYNC")
        if self.frame_flags & FrameFlags.Truncated:
            errors.append("TRUNCATED")
        return errors

    def _decode_tech(self) -> str:
        """Decode tech_type to name"""
        try:
            return TechType(self.tech_type).name
        except ValueError:
            return f"Unknown(0x{self.tech_type:04x})"

    def _decode_type(self) -> str:
        """Decode frame_type to name"""
        try:
            enum_val = FrameType(self.frame_type)
            # Simplify common names
            type_map = {
                FrameType.NfcPollFrame: "Poll",
                FrameType.NfcListenFrame: "Listen",
                FrameType.NfcCarrierOn: "CarrierOn",
                FrameType.NfcCarrierOff: "CarrierOff",
            }
            return type_map.get(enum_val, enum_val.name)
        except ValueError:
            return f"Unknown(0x{self.frame_type:04x})"

    def _decode_phase(self) -> str:
        """Decode frame_phase to name"""
        try:
            return FramePhase(self.frame_phase).name
        except ValueError:
            return f"Unknown(0x{self.frame_phase:04x})"

    def has_flag(self, flag: FrameFlags) -> bool:
        """Check if specific flag is set"""
        return bool(self.frame_flags & flag)

    def is_poll(self) -> bool:
        """Check if this is a poll/request frame"""
        return self.frame_type in [FrameType.NfcPollFrame, FrameType.IsoRequestFrame]

    def is_listen(self) -> bool:
        """Check if this is a listen/response frame"""
        return self.frame_type in [FrameType.NfcListenFrame, FrameType.IsoResponseFrame]

    def is_carrier(self) -> bool:
        """Check if this is a carrier on/off event"""
        return self.frame_type in [FrameType.NfcCarrierOn, FrameType.NfcCarrierOff]

    def to_dict(self) -> Dict[str, Any]:
        """Export to TRZ-compatible dictionary"""
        result: Dict[str, Any] = {
            "sampleStart": self.sample_start,
            "sampleEnd": self.sample_end,
            "sampleRate": self.sample_rate,
            "timeStart": self.time_start,
            "timeEnd": self.time_end,
            "dateTime": self.date_time,
            "techType": self.tech_type,
            "frameType": self.frame_type,
            "framePhase": self.frame_phase,
            "frameRate": self.frame_rate,
            "frameFlags": self.frame_flags,
        }

        # Add frame data if present
        if self.frame_data:
            # Format as colon-separated hex
            result["frameData"] = ":".join(f"{b:02X}" for b in self.frame_data)

        return result

    @classmethod
    def from_trz_dict(cls, data: dict) -> "NFCFrame":
        """
        Create NFCFrame from TRZ frame.json dictionary.

        Expects all mandatory TRZ fields to be present.
        """
        # Parse frame data (format: "AA:BB:CC" or "52")
        frame_data_str = data.get("frameData", "")
        if frame_data_str:
            hex_data = frame_data_str.replace(":", "")
            frame_data = bytes.fromhex(hex_data)
        else:
            frame_data = bytes()

        return cls(
            sample_start=data["sampleStart"],
            sample_end=data["sampleEnd"],
            sample_rate=data["sampleRate"],
            time_start=data["timeStart"],
            time_end=data["timeEnd"],
            date_time=data["dateTime"],
            tech_type=data["techType"],
            frame_type=data["frameType"],
            frame_phase=data["framePhase"],
            frame_rate=data["frameRate"],
            frame_flags=data["frameFlags"],
            frame_data=frame_data,
        )

    def __repr__(self) -> str:
        return (
            f"NFCFrame(time={self.time_start:.6f}s, tech={self.tech}, "
            f"type={self.type}, phase={self.phase}, data={len(self.frame_data)}B)"
        )
