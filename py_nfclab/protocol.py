"""
Protocol Command Detection

Stateless protocol command detection for NFC frames.
Based on ISO/IEC standards and common NFC implementations.

Note: This is stateless detection. Some response types (SAK, UID, ATS, etc.)
cannot be reliably detected without context from previous frames.
"""

from dataclasses import dataclass
from typing import Optional

from .models import NFCFrame

# NFC-A Commands (ISO/IEC 14443-A + common extensions)
NFC_A_COMMANDS = {
    # ISO/IEC 14443-A standard
    0x26: "REQA",
    0x52: "WUPA",
    0xE0: "RATS",
    # Ultralight/NTAG commands
    0x30: "READ",
    0x3A: "FAST_READ",
    0x39: "READ_CNT",
    0x3C: "READ_SIG",
    0xA2: "WRITE(UL)",
    0xA5: "INCR_CNT",
    0x1A: "AUTH",  # Ultralight C
    0x1B: "PWD_AUTH",  # Ultralight EV1
    # MIFARE Classic
    0xA0: "WRITE(MC)",
    0xB0: "TRANSFER",
    0xC0: "DECREMENT",
    0xC1: "INCREMENT",
    0x60: "AUTH_A",
    0x61: "AUTH_B",
}

# NFC-B Commands (ISO/IEC 14443-B)
NFC_B_COMMANDS = {
    0x05: "REQB",
    0x08: "WUPB",
    0x1D: "ATTRIB",
    0x50: "HLTB",
}

# NFC-F Commands (FeliCa / JIS X 6319-4)
NFC_F_COMMANDS = {
    0x00: "POLLING",  # aka REQC
    0x04: "RequestService",
    0x06: "Read",  # Read Without Encryption
    0x08: "Write",  # Write Without Encryption
    0x0A: "RequestSystemCode",
    0x0C: "Authentication1",
    0x0E: "Authentication2",
    0x10: "ReadSecure",  # Read with Encryption
    0x12: "WriteSecure",  # Write with Encryption
}

# NFC-V Commands (ISO/IEC 15693)
NFC_V_COMMANDS = {
    # Standard commands (ISO/IEC 15693-3)
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
    0x2B: "GetSystemInfo",
    0x2C: "GetMultipleBlockSecStatus",
    0x2D: "FastReadMultiple",
    # Extended commands (large block addresses)
    0x30: "ExtReadSingleBlock",
    0x31: "ExtWriteSingleBlock",
    0x32: "ExtLockSingleBlock",
    0x33: "ExtReadMultipleBlocks",
    0x35: "Authenticate",
    0x39: "Challenge",
    0x3A: "ReadBuffer",
    0x3B: "ExtGetSystemInfo",
    0x3C: "ExtGetMultiBlockSec",
    0x3D: "FastExtReadMultiple",
    # Fast commands (tag-specific)
    0xC0: "FastReadSingleBlock",
    0xC1: "WriteConfiguration",
    0xC2: "PickRandomUID",
    0xC3: "FastReadMultipleBlocks",
}

# ISO-DEP Commands (ISO/IEC 14443-4, works on top of NFC-A/B)
# PCB byte structure for block detection
ISO_DEP_I_BLOCK_MASK = 0x80  # I-Block: bit 7 = 0


# =============================================================================
# NFC-V (ISO 15693) Protocol Parsing
# =============================================================================

# NFC-V Error codes (response byte after error flag)
NFC_V_ERROR_CODES = {
    0x01: "Not supported",
    0x02: "Not recognized",
    0x0F: "Unknown",
    0x10: "Block N/A",
    0x11: "Already locked",
    0x12: "Locked",
    0x13: "Not programmed",
    0x14: "Not locked",
}


class NfcVFlags:
    """Compact NFC-V flag parser for ISO 15693 request/response flags"""

    def __init__(self, flags: int, is_request: bool = True):
        self.flags = flags
        self.is_request = is_request
        self.is_inv = bool(flags & 0x04)  # Inventory flag

    def __str__(self) -> str:
        """Human-readable flag description"""
        parts = []
        f = self.flags

        if self.is_request:
            # Subcarrier/HDR only meaningful in requests
            parts.append("Dual" if f & 0x01 else "Single")
            parts.append("HDR" if f & 0x02 else "LDR")

            if self.is_inv:  # Inventory
                parts.append("INV")
                if f & 0x10:
                    parts.append("AFI")
                parts.append("1-slot" if f & 0x20 else "16-slot")
            else:  # Non-Inventory
                if f & 0x10:
                    parts.append("SEL")
                if f & 0x20:
                    parts.append("ADDR")
                if not (f & 0x30):
                    parts.append("⚠NONE")
            if f & 0x08:
                parts.append("EXT")
            if f & 0x40:
                parts.append("OPT")
        else:  # Response
            # In responses, bit0=error, bit1 not used for subcarrier
            if f & 0x01:
                parts.append("⚠ERR")
            if f & 0x08:
                parts.append("EXT")

        return f"0x{f:02X}[{','.join(parts)}]"

    def check(self) -> Optional[str]:
        """Return warning if flags are problematic, None if OK"""
        if not self.is_request or self.is_inv:
            return None
        if not (self.flags & 0x30):  # Neither Selected nor Addressed
            return "Missing Selected/Addressed flag - tag likely won't respond"
        return None


def parse_nfcv_flags(frame: NFCFrame) -> Optional[str]:
    """Parse NFC-V flags from frame - returns compact string or None"""
    if frame.tech != "NfcV" or not frame.data:
        return None

    flags = NfcVFlags(frame.data[0], frame.is_poll())
    result = str(flags)

    # Add error code if response with error
    if not flags.is_request and (flags.flags & 0x01) and len(frame.data) >= 2:
        err_code = frame.data[1]
        err_msg = NFC_V_ERROR_CODES.get(err_code, f"0x{err_code:02X}")
        result += f" ERR:{err_msg}"

    # Add warning if problematic
    warning = flags.check()
    if warning:
        result += f" ⚠{warning}"

    return result


@dataclass
class NfcVFrame:
    """Parsed NFC-V frame structure (ISO 15693)"""

    flags: int
    cmd: Optional[int] = None
    uid: Optional[bytes] = None
    params: Optional[bytes] = None
    crc: Optional[bytes] = None
    error_code: Optional[int] = None

    def __str__(self) -> str:
        parts = [f"Flags:0x{self.flags:02X}"]
        if self.cmd is not None:
            cmd_name = NFC_V_COMMANDS.get(self.cmd, f"0x{self.cmd:02X}")
            parts.append(f"Cmd:{cmd_name}")
        if self.uid:
            parts.append(f"UID:{self.uid.hex().upper()}(LE)")
        if self.error_code is not None:
            err_msg = NFC_V_ERROR_CODES.get(self.error_code, f"0x{self.error_code:02X}")
            parts.append(f"ERR:{err_msg}")
        if self.params:
            parts.append(f"Params:{self.params.hex().upper()}")
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)

    @property
    def uid_be(self) -> Optional[bytes]:
        """UID in big-endian (human-readable) format"""
        return bytes(reversed(self.uid)) if self.uid else None


def parse_nfcv_frame(frame: NFCFrame) -> Optional[NfcVFrame]:
    """
    Parse NFC-V frame into components: flags, cmd, uid, params, crc.

    Args:
        frame: NFCFrame with tech="NfcV"

    Returns:
        NfcVFrame with parsed components, or None if invalid

    Example:
        >>> parsed = parse_nfcv_frame(frame)
        >>> print(f"Command: {parsed.cmd}, UID: {parsed.uid_be.hex()}")
    """
    if frame.tech != "NfcV" or not frame.data or len(frame.data) < 2:
        return None

    data = frame.data
    flags = data[0]

    # Check if request or response
    if frame.is_poll():  # Request
        cmd = data[1]
        off = 2

        is_inv = bool(flags & 0x04)
        addressed = bool(flags & 0x20)

        if is_inv:  # Inventory - special layout
            # F | CMD | [AFI] | MaskLen | Mask | CRC
            params_start = off
            if flags & 0x10:  # AFI present
                off += 1
            # Rest is mask data + CRC (minimum: Flags + CMD + CRC = 4 bytes)
            return NfcVFrame(
                flags=flags,
                cmd=cmd,
                params=data[params_start:-2] if len(data) >= params_start + 2 else None,
                crc=data[-2:] if len(data) >= 4 else None,
            )

        # Non-Inventory
        uid = None
        if addressed and len(data) >= off + 8:
            uid = data[off : off + 8]
            off += 8

        # Remaining bytes (excluding CRC, minimum 4 bytes total for valid frame)
        params = data[off:-2] if len(data) >= off + 2 else None
        crc = data[-2:] if len(data) >= 4 else None

        return NfcVFrame(flags=flags, cmd=cmd, uid=uid, params=params, crc=crc)

    else:  # Response
        # Check for error
        if flags & 0x01:  # Error flag
            error_code = data[1] if len(data) >= 2 else None
            crc = data[-2:] if len(data) >= 4 else None
            return NfcVFrame(flags=flags, error_code=error_code, crc=crc)

        # Success response - check for Inventory response (has UID)
        # Inventory response: Flags(1) + [DSFID(1)] + UID(8) + CRC(2)
        # Length: 10 bytes (no DSFID) or 12 bytes (with DSFID)
        if len(frame.data) in (10, 12):
            if len(frame.data) == 12:
                # With DSFID
                uid = data[2:10]
                params = data[1:2]  # DSFID
                crc = data[-2:]
            else:
                # Without DSFID (10 bytes)
                uid = data[1:9]
                params = None
                crc = data[-2:]
            return NfcVFrame(flags=flags, uid=uid, params=params, crc=crc)

        # Other responses
        params = data[1:-2] if len(data) >= 4 else None
        crc = data[-2:] if len(data) >= 4 else None
        return NfcVFrame(flags=flags, params=params, crc=crc)


def detect_command(frame: NFCFrame) -> Optional[str]:
    """
    Detect protocol command name from frame data.

    This is a best-effort, stateless detection. Response frames
    (ATQA, SAK, UID, ATS, etc.) cannot be reliably detected without
    context from previous Poll frames.

    Args:
        frame: NFCFrame to analyze

    Returns:
        Command name string if recognized, None otherwise

    Example:
        >>> frame = NFCFrame(...)  # NFC-A WUPA
        >>> detect_command(frame)
        'WUPA'
    """
    if not frame.data or len(frame.data) == 0:
        return None

    data = frame.data
    length = len(data)

    # NFC-A: Special cases first, then table lookup
    if frame.tech == "NfcA":
        cmd = data[0]

        # HLTA: 0x50 0x00 (+ optional 2 byte CRC)
        # Must check length to distinguish from HLTB
        if cmd == 0x50 and length >= 2 and data[1] == 0x00:
            return "HLTA"

        # PPS (Protocol Parameter Selection): 0xD0-0xDF, variable length
        if (cmd & 0xF0) == 0xD0 and 4 <= length <= 7:
            return "PPS"

        # Cascade levels: 0x93/0x95/0x97
        # Second byte determines ANTICOLLISION (0x20) or SELECT (0x70)
        if cmd in (0x93, 0x95, 0x97) and length >= 2:
            level = {0x93: "1", 0x95: "2", 0x97: "3"}[cmd]
            if data[1] == 0x20:
                return f"ANTICOLLISION{level}"
            elif data[1] == 0x70:
                return f"SELECT{level}"
            else:
                return f"SEL{level}"

        # Table lookup for other commands
        return NFC_A_COMMANDS.get(cmd)

    # NFC-B: Check specific commands first, then SLOT_MARKER range
    elif frame.tech == "NfcB":
        cmd = data[0]
        # Check table first (REQB=0x05, WUPB=0x08, etc.)
        result = NFC_B_COMMANDS.get(cmd)
        if result:
            return result
        # Slot markers: 0x00-0x0F (S0-S15) for remaining codes
        if 0x00 <= cmd <= 0x0F:
            return "SLOT_MARKER"
        return None

    # NFC-F: Command in second byte (first byte is length)
    elif frame.tech == "NfcF":
        if length >= 2:
            return NFC_F_COMMANDS.get(data[1])

    # NFC-V: Command in second byte (first byte is flags), only for requests
    elif frame.tech == "NfcV":
        if length >= 2 and frame.is_poll():
            return NFC_V_COMMANDS.get(data[1])

    # ISO7816 T=1: NAD + PCB structure
    elif frame.tech == "Iso7816":
        if length < 2:
            return None

        pcb = data[0] if data[0] != 0xFF else data[1] if length > 1 else None
        if pcb is None:
            return None

        # I-Block: bit 8 = 0
        if (pcb & 0x80) == 0x00 and length >= 3:
            return "I-Block"

        # R-Block: bits 8-7 = 10
        if (pcb & 0xC0) == 0x80 and length >= 3:
            return "R-Block"

        # S-Block: bits 8-7 = 11
        if (pcb & 0xC0) == 0xC0 and length >= 3:
            return "S-Block"

    # ISO-DEP (works on top of NFC-A/B): Check I/R/S-Blocks
    # Only apply to NFC-A/NFC-B Poll/Listen frames to avoid false positives
    if frame.tech in ("NfcA", "NfcB") and (frame.is_poll() or frame.is_listen()):
        pcb = data[0]

        # S(DESELECT): PCB 0xC2 or 0xCA
        if (pcb & 0xF7) == 0xC2 and 3 <= length <= 5:
            return "S(DESELECT)"

        # S(WTX): PCB 0xF2 or 0xFA
        if (pcb & 0xF7) == 0xF2 and 3 <= length <= 5:
            return "S(WTX)"

        # R(ACK): Check R-Block with bit 4 = 0 (can have CID byte)
        if (pcb & 0xE6) == 0xA2 and 3 <= length <= 5:
            return "R(ACK)"

        # R(NACK): Check R-Block with bit 4 = 1 (can have CID byte)
        if (pcb & 0xE6) == 0xB2 and 3 <= length <= 5:
            return "R(NACK)"

        # I-Block: bit 7 = 0, length >= 4
        if (pcb & ISO_DEP_I_BLOCK_MASK) == 0x00 and length >= 4:
            return "I-Block"

    return None
