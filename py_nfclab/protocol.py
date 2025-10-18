"""
NFC Protocol Command Detection and Parsing

Stateless protocol command detection for NFC frames.
Based on ISO/IEC standards and common NFC implementations.

This module provides:
- Command detection for NFC-A, NFC-B, NFC-F, NFC-V, ISO-DEP, and ISO 7816
- Protocol-specific parsers for request/response frames
- Context-aware payload interpretation where possible

Note: Stateless detection has limitations. Some response types (SAK, UID, ATS)
cannot be reliably identified without context from previous Poll frames.
"""

from dataclasses import dataclass
from typing import Optional

from .models import NFCFrame

# =============================================================================
# Protocol Command Tables
# =============================================================================

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
    0x60: "GET_VERSION",  # NTAG/Ultralight EV1
    0xA0: "COMPAT_WRITE",  # Ultralight compatibility write
    0xA2: "WRITE",
    0xA5: "INCR_CNT",
    0x1A: "AUTH",  # Ultralight C
    0x1B: "PWD_AUTH",  # Ultralight EV1
    # MIFARE Classic
    0xB0: "TRANSFER",
    0xC0: "DECREMENT",
    0xC1: "INCREMENT",
    0xC2: "RESTORE",
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
    0x02: "RequestResponse",
    0x04: "RequestService",
    0x06: "Read",  # Read Without Encryption
    0x08: "Write",  # Write Without Encryption
    0x0A: "RequestSystemCode",
    0x0C: "Authentication1",
    0x0E: "Authentication2",
    0x10: "ReadSecure",  # Read with Encryption
    0x12: "WriteSecure",  # Write with Encryption
    0x1A: "RequestSpecVersion",
    0x1C: "ResetMode",
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
    0x34: "ExtWriteMultipleBlocks",
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

# NFC-V Error codes
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

# ISO-DEP (ISO/IEC 14443-4) - works on top of NFC-A/B
ISO_DEP_I_BLOCK_MASK = 0x80  # I-Block: bit 7 = 0


# =============================================================================
# Helper Functions
# =============================================================================


def _strip_crc_if_present(data: bytes) -> tuple[bytes, Optional[bytes]]:
    """Strip CRC (last 2 bytes) if frame is long enough"""
    CRC_LEN = 2
    if len(data) >= 4:
        return data[:-CRC_LEN], data[-CRC_LEN:]
    return data, None


# =============================================================================
# NFC-A (ISO/IEC 14443-A + ISO-DEP) Protocol Parsing
# =============================================================================

ATQA_LEN = 2  # Answer To Request A


@dataclass
class NfcARequest:
    """Parsed NFC-A request frame (ISO 14443-A)"""

    cmd: int
    params: Optional[bytes] = None
    crc: Optional[bytes] = None

    def __str__(self) -> str:
        parts = [f"Cmd:{self.cmd:02X}"]
        if self.params:
            parts.append(f"Params:{self.params.hex().upper()}")
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)


@dataclass
class NfcAResponse:
    """Parsed NFC-A response frame (ISO 14443-A)"""

    payload: Optional[bytes] = None
    crc: Optional[bytes] = None
    request_cmd: Optional[int] = None

    def __str__(self) -> str:
        if not self.payload:
            return f"CRC:{self.crc.hex().upper()}" if self.crc else "Empty"
        interp = self._interpret_payload()
        if interp:
            return interp
        parts = [f"Payload:{self.payload.hex().upper()}"]
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)

    def _interpret_payload(self) -> Optional[str]:
        """Interpret payload based on heuristics and request context"""
        p = self.payload
        if not p:
            return None

        # ATQA (2B)
        if len(p) == ATQA_LEN:
            return f"ATQA:{p.hex().upper()}"

        # SAK (1B) - stateless uncertain
        if len(p) == 1:
            return f"SAK:{p[0]:02X}"

        # ATS after RATS: TL | [T0][TA][TB][TC]... (min 1B)
        if self.request_cmd == 0xE0 and len(p) >= 1:
            tl = p[0]
            if tl == len(p):  # Simple plausibility check
                return f"ATS[{tl}B]:{p.hex().upper()}"

        # UID (4/7/10B) heuristic
        if len(p) in (4, 7, 10):
            return f"UID:{p.hex().upper()}"

        return None


def parse_nfca_request(frame: NFCFrame) -> Optional[NfcARequest]:
    """Parse NFC-A request frame (Poll frames)"""
    if frame.tech != "NfcA" or not frame.data or not frame.is_poll():
        return None
    data, crc = _strip_crc_if_present(frame.data)
    cmd = data[0]
    params = data[1:] if len(data) > 1 else None
    return NfcARequest(cmd=cmd, params=params if params else None, crc=crc)


def parse_nfca_response(
    frame: NFCFrame, request_cmd: Optional[int] = None
) -> Optional[NfcAResponse]:
    """Parse NFC-A response frame (Listen frames)"""
    if frame.tech != "NfcA" or not frame.data or frame.is_poll():
        return None
    data, crc = _strip_crc_if_present(frame.data)
    return NfcAResponse(
        payload=data if data else None, crc=crc, request_cmd=request_cmd
    )


# =============================================================================
# NFC-B (ISO/IEC 14443-B) Protocol Parsing
# =============================================================================

ATQB_MIN_LEN = 11  # PUPI(4) + AppData(4) + ProtInfo(3)


@dataclass
class NfcBRequest:
    """Parsed NFC-B request frame (ISO 14443-B)"""

    cmd: int
    params: Optional[bytes] = None
    crc: Optional[bytes] = None

    def __str__(self) -> str:
        parts = [f"Cmd:{self.cmd:02X}"]
        if self.params:
            parts.append(f"Params:{self.params.hex().upper()}")
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)


@dataclass
class NfcBResponse:
    """Parsed NFC-B response frame (ISO 14443-B)"""

    payload: Optional[bytes] = None
    crc: Optional[bytes] = None
    request_cmd: Optional[int] = None

    def __str__(self) -> str:
        if not self.payload:
            return f"CRC:{self.crc.hex().upper()}" if self.crc else "Empty"
        interp = self._interpret_payload()
        if interp:
            return interp
        parts = [f"Payload:{self.payload.hex().upper()}"]
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)

    def _interpret_payload(self) -> Optional[str]:
        """Interpret payload based on request context"""
        p = self.payload
        if not p:
            return None

        # ATQB after REQB/WUPB: 11B (without CRC)
        if self.request_cmd in (0x05, 0x08) and len(p) >= ATQB_MIN_LEN:
            pupi = p[0:4].hex().upper()
            app = p[4:8].hex().upper()
            proto = p[8:11].hex().upper()
            rest = p[11:].hex().upper()
            result = f"ATQB PUPI:{pupi} AppData:{app} ProtInfo:{proto}"
            if rest:
                result += f" Extra:{rest}"
            return result

        # ATTRIB response
        if self.request_cmd == 0x1D:
            return f"ATTRIB-Resp:{p.hex().upper()}"

        return None


def parse_nfcb_request(frame: NFCFrame) -> Optional[NfcBRequest]:
    """Parse NFC-B request frame (Poll frames)"""
    if frame.tech != "NfcB" or not frame.data or not frame.is_poll():
        return None
    data, crc = _strip_crc_if_present(frame.data)
    cmd = data[0]
    params = data[1:] if len(data) > 1 else None
    return NfcBRequest(cmd=cmd, params=params if params else None, crc=crc)


def parse_nfcb_response(
    frame: NFCFrame, request_cmd: Optional[int] = None
) -> Optional[NfcBResponse]:
    """Parse NFC-B response frame (Listen frames)"""
    if frame.tech != "NfcB" or not frame.data or frame.is_poll():
        return None
    data, crc = _strip_crc_if_present(frame.data)
    return NfcBResponse(
        payload=data if data else None, crc=crc, request_cmd=request_cmd
    )


# =============================================================================
# NFC-F (FeliCa / JIS X 6319-4) Protocol Parsing
# =============================================================================


@dataclass
class NfcFRequest:
    """Parsed NFC-F request frame (FeliCa)"""

    cmd: int
    body: Optional[bytes] = None  # after CMD (without L)

    def __str__(self) -> str:
        parts = [f"Cmd:{self.cmd:02X}"]
        if self.body:
            parts.append(f"Body:{self.body.hex().upper()}")
        return " | ".join(parts)


@dataclass
class NfcFResponse:
    """Parsed NFC-F response frame (FeliCa)"""

    cmd: int
    body: Optional[bytes] = None  # after CMD (without L)
    request_cmd: Optional[int] = None

    def __str__(self) -> str:
        interp = self._interpret_body()
        if interp:
            return interp
        parts = [f"Cmd:{self.cmd:02X}"]
        if self.body:
            parts.append(f"Body:{self.body.hex().upper()}")
        return " | ".join(parts)

    def _interpret_body(self) -> Optional[str]:
        """Interpret body based on request context"""
        b = self.body or b""

        # POLLING (0x00) response: IDm(8) PMm(8) [SysCodes...]
        if self.request_cmd == 0x00 and len(b) >= 16:
            idm = b[0:8].hex().upper()
            pmm = b[8:16].hex().upper()
            rest = b[16:]
            result = f"POLLING IDm:{idm} PMm:{pmm}"
            if rest:
                syscodes = " ".join(
                    [rest[i : i + 2].hex().upper() for i in range(0, len(rest), 2)]
                )
                result += f" SysCodes:{syscodes}"
            return result

        # Read/Write Without Encryption (0x06/0x08)
        # Format: IDm(8) + SF1(1) + SF2(1) + Data...
        if self.request_cmd in (0x06, 0x08) and len(b) >= 10:
            idm = b[0:8].hex().upper()
            sf1 = b[8]  # Status Flag 1
            sf2 = b[9]  # Status Flag 2
            data = b[10:].hex().upper() if len(b) > 10 else ""
            return f"IDm:{idm} SF1:{sf1:02X} SF2:{sf2:02X} Data:{data}"

        return None


def parse_nfcf_request(frame: NFCFrame) -> Optional[NfcFRequest]:
    """Parse NFC-F request frame (Poll frames)"""
    if frame.tech != "NfcF" or not frame.data or not frame.is_poll():
        return None
    if len(frame.data) < 2:
        return None
    L = frame.data[0]
    cmd = frame.data[1]
    # L-byte consistency check: L should equal total frame length
    # If mismatch, assume CRC (2B) is present and strip it
    if L == len(frame.data):
        # L matches total length - use L directly
        body = frame.data[2:L]
    elif len(frame.data) >= 4:
        # L mismatch - assume CRC (2B) at end, strip it
        body = frame.data[2:-2]
    else:
        # Too short for CRC
        body = frame.data[2:]
    return NfcFRequest(cmd=cmd, body=body if body else None)


def parse_nfcf_response(
    frame: NFCFrame, request_cmd: Optional[int] = None
) -> Optional[NfcFResponse]:
    """Parse NFC-F response frame (Listen frames)"""
    if frame.tech != "NfcF" or not frame.data or frame.is_poll():
        return None
    if len(frame.data) < 2:
        return None
    L = frame.data[0]
    cmd = frame.data[1]
    # L-byte consistency check: L should equal total frame length
    # If mismatch, assume CRC (2B) is present and strip it
    if L == len(frame.data):
        # L matches total length - use L directly
        body = frame.data[2:L]
    elif len(frame.data) >= 4:
        # L mismatch - assume CRC (2B) at end, strip it
        body = frame.data[2:-2]
    else:
        # Too short for CRC
        body = frame.data[2:]
    return NfcFResponse(cmd=cmd, body=body if body else None, request_cmd=request_cmd)


# =============================================================================
# NFC-V (ISO 15693) Protocol Parsing
# =============================================================================


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
                # Note: ⚠NONE warning removed here - use check() method instead
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
        sel = bool(self.flags & 0x10)
        addr = bool(self.flags & 0x20)
        if not (sel or addr):
            return (
                "No SEL/ADDR - Non-addressed mode; response depends on selection state"
            )
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
class NfcVRequest:
    """Parsed NFC-V request frame (ISO 15693)"""

    flags: int
    cmd: int
    uid: Optional[bytes] = None
    params: Optional[bytes] = None
    crc: Optional[bytes] = None

    def __str__(self) -> str:
        parts = [f"Flags:0x{self.flags:02X}"]
        cmd_name = NFC_V_COMMANDS.get(self.cmd, f"0x{self.cmd:02X}")
        parts.append(f"Cmd:{cmd_name}")
        if self.uid:
            parts.append(f"UID:{self.uid.hex().upper()}(LE)")
        if self.params:
            # Special formatting for Inventory params
            is_inv = bool(self.flags & 0x04)
            if is_inv and len(self.params) >= 1:
                # Inventory: [AFI] + MaskLen + Mask
                off = 0
                param_parts = []
                if self.flags & 0x10:  # AFI present
                    param_parts.append(f"AFI:{self.params[off]:02X}")
                    off += 1
                if len(self.params) > off:
                    mask_len = self.params[off]
                    param_parts.append(f"MaskLen:{mask_len}")
                    off += 1
                    if len(self.params) > off:
                        mask = self.params[off:]
                        param_parts.append(f"Mask:{mask.hex().upper()}")
                parts.append(" ".join(param_parts))
            else:
                parts.append(f"Params:{self.params.hex().upper()}")
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)

    @property
    def uid_be(self) -> Optional[bytes]:
        """UID in big-endian (human-readable) format"""
        return bytes(reversed(self.uid)) if self.uid else None


@dataclass
class NfcVResponse:
    """Parsed NFC-V response frame (ISO 15693)"""

    flags: int
    payload: Optional[bytes] = None
    crc: Optional[bytes] = None
    error_code: Optional[int] = None
    request_cmd: Optional[int] = None  # Optional: command from previous request

    def __str__(self) -> str:
        parts = [f"Flags:0x{self.flags:02X}"]
        if self.error_code is not None:
            err_msg = NFC_V_ERROR_CODES.get(self.error_code, f"0x{self.error_code:02X}")
            parts.append(f"ERR:{err_msg}")
        if self.payload is not None:  # Allow empty payload (b"")
            # Context-aware payload interpretation
            if self.request_cmd is not None and not self.is_error:
                interpreted = self._interpret_payload()
                if interpreted:
                    parts.append(interpreted)
                else:
                    parts.append(f"Payload:{self.payload.hex().upper()}")
            else:
                parts.append(f"Payload:{self.payload.hex().upper()}")
        if self.crc:
            parts.append(f"CRC:{self.crc.hex().upper()}")
        return " | ".join(parts)

    @property
    def is_error(self) -> bool:
        """Check if response is an error"""
        return bool(self.flags & 0x01)

    @property
    def uid(self) -> Optional[bytes]:
        """Extract UID from Inventory response (if applicable)"""
        if self.request_cmd == 0x01 and self.payload and len(self.payload) >= 8:
            # Inventory response payload: [DSFID?] + UID(8) (CRC already removed)
            if len(self.payload) == 9:  # With DSFID
                return self.payload[1:9]
            elif len(self.payload) == 8:  # Without DSFID
                return self.payload[0:8]
        return None

    @property
    def uid_be(self) -> Optional[bytes]:
        """UID in big-endian (human-readable) format"""
        uid = self.uid
        return bytes(reversed(uid)) if uid else None

    def _interpret_payload(self) -> Optional[str]:
        """Interpret payload based on request command"""
        if not self.payload or self.request_cmd is None:
            return None

        cmd = self.request_cmd
        data = self.payload

        # Inventory (0x01): DSFID? + UID(8)
        if cmd == 0x01:
            if len(data) == 9:  # With DSFID
                dsfid = data[0]
                uid = data[1:9]
                uid_be = bytes(reversed(uid))
                return f"DSFID:0x{dsfid:02X} UID:{uid_be.hex().upper()}"
            elif len(data) == 8:  # Without DSFID
                uid_be = bytes(reversed(data))
                return f"UID:{uid_be.hex().upper()}"

        # GetSystemInfo (0x2B): InfoFlags + fields
        if cmd == 0x2B and len(data) >= 1:
            info_flags = data[0]
            parts = [f"InfoFlags:0x{info_flags:02X}"]
            # Simple interpretation: just show raw for now
            if len(data) > 1:
                parts.append(f"Data:{data[1:].hex().upper()}")
            return " ".join(parts)

        # ReadSingleBlock (0x20), ExtReadSingleBlock (0x30)
        if cmd in (0x20, 0x30):
            return f"BlockData:{data.hex().upper()}"

        # ReadMultipleBlocks (0x23), ExtReadMultipleBlocks (0x33)
        if cmd in (0x23, 0x33):
            return f"BlockData[{len(data)}B]:{data.hex().upper()}"

        # GetMultipleBlockSecurityStatus (0x2C)
        if cmd == 0x2C:
            return f"SecurityStatus:{data.hex().upper()}"

        return None


def parse_nfcv_response(
    frame: NFCFrame, request_cmd: Optional[int] = None
) -> Optional[NfcVResponse]:
    """Parse NFC-V response frame (Listen frames)"""
    if frame.tech != "NfcV" or not frame.data or frame.is_poll():
        return None

    data = frame.data
    flags = data[0]

    # Error response: Flags(1) + ErrorCode(1) + CRC(2)
    if flags & 0x01:  # Error flag
        error_code = data[1] if len(data) >= 2 else None
        crc = data[-2:] if len(data) >= 4 else None
        return NfcVResponse(
            flags=flags, error_code=error_code, crc=crc, request_cmd=request_cmd
        )

    # Success response: Flags(1) + Payload + CRC(2)
    payload = data[1:-2] if len(data) >= 4 else (data[1:] if len(data) > 1 else None)
    crc = data[-2:] if len(data) >= 4 else None

    return NfcVResponse(flags=flags, payload=payload, crc=crc, request_cmd=request_cmd)


def parse_nfcv_request(frame: NFCFrame) -> Optional[NfcVRequest]:
    """Parse NFC-V request frame (Poll frames)"""
    if (
        frame.tech != "NfcV"
        or not frame.data
        or len(frame.data) < 2
        or not frame.is_poll()
    ):
        return None

    data = frame.data
    flags = data[0]
    cmd = data[1]
    off = 2

    is_inv = bool(flags & 0x04)
    addressed = bool(flags & 0x20)

    params: Optional[bytes]

    if is_inv:  # Inventory - special layout
        # F | CMD | [AFI] | MaskLen | Mask | CRC
        afi = None
        if flags & 0x10:  # AFI present
            if len(data) > off:
                afi = data[off]
                off += 1

        # Read MaskLen (in bits) - always present per ISO 15693
        mask_len_bits = data[off] if len(data) > off else 0
        off += 1

        # Calculate mask length in bytes
        mask_len_bytes = (mask_len_bits + 7) // 8
        mask = (
            data[off : off + mask_len_bytes]
            if len(data) >= off + mask_len_bytes
            else b""
        )

        # Build params: [AFI] + MaskLen + Mask (MaskLen always included)
        params_list = []
        if afi is not None:
            params_list.append(afi)
        params_list.append(mask_len_bits)  # Always present
        params_list.extend(mask)

        params = bytes(params_list)
        crc = data[-2:] if len(data) >= 4 else None

        return NfcVRequest(flags=flags, cmd=cmd, params=params, crc=crc)

    # Non-Inventory
    uid = None
    if addressed and len(data) >= off + 8:
        uid = data[off : off + 8]
        off += 8

    # Remaining bytes (excluding CRC, minimum 4 bytes total for valid frame)
    if len(data) >= off + 2:
        params_slice = data[off:-2]
        params = params_slice if params_slice else None
    else:
        params = None
    crc = data[-2:] if len(data) >= 4 else None

    return NfcVRequest(flags=flags, cmd=cmd, uid=uid, params=params, crc=crc)


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

    # Strip CRC for NFC-A/B/V to improve detection accuracy
    if frame.tech in ("NfcA", "NfcB", "NfcV"):
        data, _ = _strip_crc_if_present(frame.data)
    else:
        data = frame.data
    length = len(data)

    result = None

    # NFC-A: Special cases first, then table lookup
    if frame.tech == "NfcA":
        cmd = data[0]

        # HLTA (0x50 0x00) - must check second byte to distinguish from HLTB
        if cmd == 0x50 and length >= 2 and data[1] == 0x00:
            result = "HLTA"

        # AUTH_A/AUTH_B (MIFARE Classic) - conflicts with GET_VERSION (0x60)
        # Heuristic: length > 2 → AUTH, else GET_VERSION
        elif cmd == 0x60 and length >= 2:
            result = "AUTH_A" if length > 2 else NFC_A_COMMANDS.get(cmd)
        elif cmd == 0x61 and length >= 2:
            result = "AUTH_B"

        # PPS (0xD0-0xDF) - PPSS + PPS0 minimum, optional PPS1/PPS2/PPS3/CID
        elif (cmd & 0xF0) == 0xD0 and length >= 2:
            result = "PPS"

        # Cascade levels (0x93/0x95/0x97) - NVB determines mode
        elif cmd in (0x93, 0x95, 0x97) and length >= 2:
            level = {0x93: "1", 0x95: "2", 0x97: "3"}[cmd]
            nvb = data[1]
            if nvb == 0x70:
                result = f"SELECT{level}"
            elif (nvb & 0xF0) == 0x20:
                result = f"ANTICOLLISION{level}"
            else:
                result = f"SEL{level}"

        # Table lookup for remaining commands
        else:
            result = NFC_A_COMMANDS.get(cmd)

    # NFC-B: Table lookup, then SLOT_MARKER for Poll frames
    elif frame.tech == "NfcB":
        cmd = data[0]
        result = NFC_B_COMMANDS.get(cmd)
        # Slot markers (0x00-0x0F) only in Poll frames
        if result is None and frame.is_poll() and 0x00 <= cmd <= 0x0F:
            result = "SLOT_MARKER"

    # NFC-F: Command in second byte (first byte is length)
    # Responses use CMD+1 (e.g., POLLING 0x00 → response 0x01)
    elif frame.tech == "NfcF":
        if length >= 2:
            cmd = data[1]
            if frame.is_poll():
                result = NFC_F_COMMANDS.get(cmd)
            else:
                # Response: map back to request (CMD-1)
                request_cmd = (cmd - 1) & 0xFF
                cmd_name = NFC_F_COMMANDS.get(request_cmd)
                result = f"{cmd_name}-Resp" if cmd_name else None

    # NFC-V: Command in second byte (first byte is flags)
    elif frame.tech == "NfcV":
        if length >= 2 and frame.is_poll():
            result = NFC_V_COMMANDS.get(data[1])

    # ISO 7816 T=1: PCB-based block structure
    elif frame.tech == "Iso7816":
        if length >= 1:
            pcb = data[0]
            # I-Block: bit 7 = 0
            if (pcb & 0x80) == 0x00:
                result = "I-Block"
            # R-Block: bits 7-6 = 10
            elif (pcb & 0xC0) == 0x80:
                result = "R-Block"
            # S-Block: bits 7-6 = 11
            elif (pcb & 0xC0) == 0xC0:
                result = "S-Block"

    # ISO-DEP (ISO/IEC 14443-4) on top of NFC-A/B
    # Only if nothing recognized yet and frame is Poll/Listen
    if (
        result is None
        and frame.tech in ("NfcA", "NfcB")
        and (frame.is_poll() or frame.is_listen())
    ):
        pcb = data[0]

        # S(DESELECT): PCB 0xC2/0xCA
        if (pcb & 0xF7) == 0xC2:
            return "S(DESELECT)"

        # S(WTX): PCB 0xF2/0xFA
        if (pcb & 0xF7) == 0xF2:
            return "S(WTX)"

        # R(ACK): bit 4 = 0
        if (pcb & 0xE6) == 0xA2:
            return "R(ACK)"

        # R(NACK): bit 4 = 1
        if (pcb & 0xE6) == 0xB2:
            return "R(NACK)"

        # I-Block: bit 7 = 0
        if (pcb & ISO_DEP_I_BLOCK_MASK) == 0x00:
            return "I-Block"

    return result
