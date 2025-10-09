"""
Protocol parser for NFC frames.

This module provides protocol-specific parsing and event detection.
Based on StreamModel.cpp event detection logic.
"""

from typing import Optional

from .models import NFCFrame

# NFC-A Command mappings (ISO/IEC 14443-A standard + common extensions)
NFC_A_CMD = {
    # ISO/IEC 14443-A standard
    0x26: "REQA",
    0x52: "WUPA",
    0x50: "HLTA",
    0x93: "SEL1",
    0x95: "SEL2",
    0x97: "SEL3",
    0xE0: "RATS",
    # Common Ultralight/NTAG commands (de-facto standard)
    0x30: "READ",
    0x3A: "FAST_READ",
    0x39: "READ_CNT",
    0x3C: "READ_SIG",
    0xA0: "COMP_WRITE",
    0xA2: "WRITE",
    0xA5: "INCR_CNT",
    0x1A: "AUTH",  # Ultralight C
    0x1B: "PWD_AUTH",  # Ultralight EV1
    # MIFARE Classic (not ISO standard)
    0x60: "AUTH",
    0x61: "AUTH",
}

NFC_A_RESP = {0x26: "ATQA", 0x52: "ATQA"}

# NFC-B Command/Response mappings (ISO/IEC 14443-B standard)
NFC_B_CMD = {0x05: "REQB", 0x08: "WUPB", 0x1D: "ATTRIB", 0x50: "HLTB"}

NFC_B_RESP = {
    0x05: "ATQB",
    0x08: "ATQB",
}

# NFC-F Command/Response mappings
NFC_F_CMD = {
    0x00: "REQC",
}

NFC_F_RESP = {
    0x00: "ATQC",
}

# NFC-V Command mappings
NFC_V_CMD = {
    0x01: "Inventory",
    0x02: "StayQuiet",
    0x20: "ReadSingleBlock",
    0x21: "WriteSingleBlock",
    0x22: "LockBlock",
    0x23: "ReadMultipleBlocks",
    0x24: "WriteMultipleBlocks",
    0x25: "Select",
    0x26: "ResetToReady",
    0x27: "WriteAFI",
    0x28: "LockAFI",
    0x29: "WriteDSFID",
    0x2A: "LockDSFID",
    0x2B: "GetSystemInformation",
    0x2C: "GetMultipleBlockSecurityStatus",
    0x2D: "FastReadMultiple",
    0x30: "ExtReadSingleBlock",
    0x31: "ExtWriteSingleBlock",
    0x32: "ExtLockSingleBlock",
    0x33: "ExtReadMultipleBlocks",
    0x35: "Authenticate",
    0x39: "Challenge",
    0x3A: "ReadBuffer",
    0x3B: "ExtGetSysInfo",
    0x3C: "ExtGetMultiBlockSec",
    0x3D: "FastExtReadMultiple",
    0xA0: "InventoryRead",
    0xA1: "FastInventoryRead",
    0xA2: "SetEAS",
    0xA3: "ResetEAS",
    0xA4: "LockEAS",
    0xA5: "EASAlarm",
    0xA6: "ProtectEASAFI",
    0xA7: "WriteEASID",
    0xAB: "GetNXPSysInfo",
    0xB2: "GetRandomNumber",
    0xB3: "SetPassword",
    0xB4: "WritePassword",
    0xB5: "LockPassword",
    0xB6: "ProtectPage",
    0xB7: "LockPageProtection",
    0xB9: "Destroy",
    0xBA: "EnableNFCPrivacy",
    0xBB: "Password64Protection",
    0xBD: "ReadSignature",
    0xC0: "ReadConfiguration",
    0xC1: "WriteConfiguration",
    0xC2: "PickRandomUID",
    0xD2: "ReadSRAM",
    0xD3: "WriteSRAM",
    0xD4: "WriteI2C",
    0xD5: "ReadI2C",
}


class ProtocolParser:
    """
    Parse NFC frames and detect protocol events.

    This class provides protocol-specific parsing logic based on the
    C++ implementation in StreamModel.cpp.
    """

    def __init__(self) -> None:
        self.prev_poll_cmd: Optional[int] = None  # Only store first byte for context

    def parse(self, frame: NFCFrame) -> NFCFrame:
        """
        Parse a frame and add event and parsed_data information.

        Args:
            frame: The NFCFrame to parse

        Returns:
            The same frame with event and parsed_data fields filled
        """
        # Handle empty frames
        if not frame.data:
            frame.event = "ATR" if frame.tech == "ISO7816" else None
            self._update_context(frame)
            return frame

        # Check ISO-DEP first (works across NFC-A/B)
        iso_dep_event = self._detect_event_iso_dep(frame.data)
        if iso_dep_event:
            frame.event = iso_dep_event
        # Technology-specific detection
        elif frame.tech == "NfcA":
            frame.event = self._detect_event_nfc_a(frame)
        elif frame.tech == "NfcB":
            frame.event = self._detect_event_nfc_b(frame)
        elif frame.tech == "NfcF":
            frame.event = self._detect_event_nfc_f(frame)
        elif frame.tech == "NfcV":
            frame.event = self._detect_event_nfc_v(frame)
        elif frame.tech == "ISO7816":
            frame.event = self._detect_event_iso7816(frame)

        # Special handling for carrier events
        if frame.frame_type == "CarrierOn":
            frame.event = "RF-On"
        elif frame.frame_type == "CarrierOff":
            frame.event = "RF-Off"

        self._update_context(frame)
        return frame

    def _update_context(self, frame: NFCFrame) -> None:
        """Update parser context after processing a frame"""
        if frame.is_poll() and frame.data:
            self.prev_poll_cmd = frame.data[0]
        elif frame.is_listen():
            self.prev_poll_cmd = None

    def _detect_event_nfc_a(self, frame: NFCFrame) -> Optional[str]:
        """Detect NFC-A protocol events"""
        cmd = frame.data[0]
        length = len(frame.data)

        if frame.is_poll():
            # HALT command
            if cmd == 0x50 and length == 4:
                return "HALT"
            # Protocol Parameter Selection
            if (cmd & 0xF0) == 0xD0 and length == 5:
                return "PPS"
            # Check NFC-A command table
            if cmd in NFC_A_CMD:
                return NFC_A_CMD[cmd]

        elif frame.is_listen() and self.prev_poll_cmd is not None:
            # SAK or UID response
            if self.prev_poll_cmd in (0x93, 0x95, 0x97):
                if length == 3:
                    return "SAK"
                if length == 5:
                    return "UID"
            # ATS response
            if self.prev_poll_cmd == 0xE0 and frame.data[0] == (length - 2):
                return "ATS"
            # Check NFC-A response table
            if self.prev_poll_cmd in NFC_A_RESP:
                return NFC_A_RESP[self.prev_poll_cmd]

        return None

    def _detect_event_nfc_b(self, frame: NFCFrame) -> Optional[str]:
        """Detect NFC-B protocol events"""
        cmd = frame.data[0]

        if frame.is_poll():
            return NFC_B_CMD.get(cmd)
        else:
            return NFC_B_RESP.get(cmd)

    def _detect_event_nfc_f(self, frame: NFCFrame) -> Optional[str]:
        """Detect NFC-F protocol events"""
        if len(frame.data) < 2:
            return None

        cmd = frame.data[1]

        if frame.is_poll():
            return NFC_F_CMD.get(cmd, f"CMD {cmd:02X}")

        return NFC_F_RESP.get(cmd)

    def _detect_event_nfc_v(self, frame: NFCFrame) -> Optional[str]:
        """Detect NFC-V protocol events

        Note: Uses simple heuristic (Byte0=Flags, Byte1=Command).
        Full parsing would require flag evaluation for addressed mode.
        """
        if len(frame.data) < 2 or not frame.is_poll():
            return None

        cmd = frame.data[1]
        return NFC_V_CMD.get(cmd, f"CMD {cmd:02X}")

    def _detect_event_iso7816(self, frame: NFCFrame) -> Optional[str]:
        """Detect ISO7816 T=1 protocol events using bitmask logic"""
        if len(frame.data) < 2:
            return None

        nad = frame.data[0]
        pcb = frame.data[1]
        length = len(frame.data)

        # PPS
        if nad == 0xFF:
            return "PPS"

        # I-Block: bit 8 = 0
        if (pcb & 0x80) == 0x00 and length >= 4:
            return "I-Block"

        # R-Block: bits 8-7 = 10
        if (pcb & 0xC0) == 0x80 and length >= 4:
            return "R-Block"

        # S-Block: bits 8-7 = 11
        if (pcb & 0xC0) == 0xC0 and length >= 4:
            return "S-Block"

        return None

    def _detect_event_iso_dep(self, data: bytes) -> Optional[str]:
        """Detect ISO-DEP protocol events"""
        if not data:
            return None

        cmd = data[0]
        length = len(data)

        # S(Deselect)
        if (cmd & 0xF7) == 0xC2 and 3 <= length <= 4:
            return "S(Deselect)"

        # S(WTX)
        if (cmd & 0xF7) == 0xF2 and 3 <= length <= 4:
            return "S(WTX)"

        # R(ACK)
        if (cmd & 0xF6) == 0xA2 and length == 3:
            return "R(ACK)"

        # R(NACK)
        if (cmd & 0xF6) == 0xB2 and length == 3:
            return "R(NACK)"

        # I-Block
        if (cmd & 0xE6) == 0x02 and length >= 4:
            return "I-Block"

        # R-Block
        if (cmd & 0xE6) == 0xA2 and length == 3:
            return "R-Block"

        # S-Block
        if (cmd & 0xC7) == 0xC2 and 3 <= length <= 4:
            return "S-Block"

        return None
