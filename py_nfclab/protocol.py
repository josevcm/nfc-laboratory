"""
Protocol Command Detection

Stateless protocol command detection for NFC frames.
Based on ISO/IEC standards and common NFC implementations.

Note: This is stateless detection. Some response types (SAK, UID, ATS, etc.)
cannot be reliably detected without context from previous frames.
"""

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
    0xA2: "WRITE",
    0xA5: "INCR_CNT",
    0x1A: "AUTH",  # Ultralight C
    0x1B: "PWD_AUTH",  # Ultralight EV1
    # MIFARE Classic
    0xA0: "WRITE",  # Classic WRITE (different from Ultralight 0xA2)
    0xB0: "TRANSFER",
    0xC0: "DECREMENT",
    0xC1: "INCREMENT",
    0x60: "AUTH_A",
    0x61: "AUTH_B",
}

# NFC-B Commands (ISO/IEC 14443-B)
NFC_B_COMMANDS = {
    0x00: "SLOT_MARKER",
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
ISO_DEP_R_BLOCK_MASK = 0xC0  # R-Block: bits 7-6 = 10
ISO_DEP_S_BLOCK_MASK = 0xC0  # S-Block: bits 7-6 = 11


def detect_command(frame: NFCFrame) -> Optional[str]:
    """
    Detect protocol command name from frame data.

    This is a best-effort, stateless detection. Response frames (ATQA, SAK, UID,
    ATS, etc.) cannot be reliably detected without context from previous Poll frames.

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

        # HLTA: 0x50 0x00 (+ 2 byte CRC = 4 bytes total)
        # Must check length to distinguish from HLTB
        if cmd == 0x50 and length == 4:
            return "HLTA"

        # PPS (Protocol Parameter Selection): 0xD0-0xDF
        if (cmd & 0xF0) == 0xD0 and length == 5:
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

    # NFC-B: Simple table lookup
    elif frame.tech == "NfcB":
        return NFC_B_COMMANDS.get(data[0])

    # NFC-F: Command in second byte (first byte is length)
    elif frame.tech == "NfcF":
        if length >= 2:
            return NFC_F_COMMANDS.get(data[1])

    # NFC-V: Command in second byte (first byte is flags)
    elif frame.tech == "NfcV":
        if length >= 2:
            return NFC_V_COMMANDS.get(data[1])

    # ISO7816 T=1: NAD + PCB structure
    elif frame.tech == "Iso7816":
        if length < 2:
            return None

        nad = data[0]
        pcb = data[1]

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

    # ISO-DEP (works on top of NFC-A/B): Check I/R/S-Blocks
    # Apply to Poll/Listen frames regardless of tech
    if frame.is_poll() or frame.is_listen():
        pcb = data[0]

        # S(DESELECT): PCB 0xC2 or 0xCA
        if (pcb & 0xF7) == 0xC2 and 3 <= length <= 4:
            return "S(DESELECT)"

        # S(WTX): PCB 0xF2 or 0xFA
        if (pcb & 0xF7) == 0xF2 and 3 <= length <= 4:
            return "S(WTX)"

        # R(ACK): Check R-Block with bit 4 = 0
        if (pcb & 0xE6) == 0xA2 and length == 3:
            return "R(ACK)"

        # R(NACK): Check R-Block with bit 4 = 1
        if (pcb & 0xE6) == 0xB2 and length == 3:
            return "R(NACK)"

        # I-Block: bit 7 = 0, length >= 4
        if (pcb & ISO_DEP_I_BLOCK_MASK) == 0x00 and length >= 4:
            return "I-Block"

    return None
