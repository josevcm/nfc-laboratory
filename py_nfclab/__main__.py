#!/usr/bin/env python3
"""
NFC Frame Monitor - py_nfclab CLI

Displays NFC frames in human-readable format.
Supports both live streams (stdin) and TRZ files.

Usage:
    # Live monitoring
    ./nfc-lab -j | python3 -m py_nfclab

    # Analyze TRZ file
    python3 -m py_nfclab trace.trz

    # Show only Poll/Listen frames (hide carrier events)
    ./nfc-lab -j | python3 -m py_nfclab --no-carrier
"""

import argparse
import json
import sys
from pathlib import Path
from typing import Iterator

from . import (
    NFCFrame,
    parse_nfca_request,
    parse_nfca_response,
    parse_nfcb_request,
    parse_nfcb_response,
    parse_nfcf_request,
    parse_nfcf_response,
    parse_nfcv_request,
    parse_nfcv_response,
)
from .protocol import detect_command
from .readers import TRZReader


# ANSI Color codes
class Colors:
    RESET = "\033[0m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    BLUE = "\033[94m"
    MAGENTA = "\033[95m"
    CYAN = "\033[96m"
    BOLD = "\033[1m"
    GRAY = "\033[90m"


TECH_COLORS = {
    "NfcA": Colors.GREEN,
    "NfcB": Colors.BLUE,
    "NfcF": Colors.MAGENTA,
    "NfcV": Colors.CYAN,
    "Iso7816": Colors.YELLOW,
}

TYPE_COLORS = {
    "Poll": Colors.BOLD + Colors.YELLOW,
    "Listen": Colors.GRAY,
    "CarrierOn": Colors.GREEN,
    "CarrierOff": Colors.RED,
}


def format_frame(frame: NFCFrame) -> str:
    """Format a single frame for display"""
    tech_color = TECH_COLORS.get(frame.tech, Colors.RESET)
    type_color = TYPE_COLORS.get(frame.type, Colors.RESET)

    # Detect protocol command
    command = detect_command(frame)

    # Build output with consistent column widths
    output = f"[{Colors.BOLD}{frame.timestamp:>12.6f}{Colors.RESET}] "
    output += f"{tech_color}{frame.tech:>12}{Colors.RESET} "

    # Rate column: 6 characters wide, always present
    rate_str = f"{frame.rate}" if frame.rate else ""
    output += f"{rate_str:>6} "

    output += f"{type_color}{frame.type:>10}{Colors.RESET} | "

    # Command column: 16 characters wide, always present
    command_str = f"{Colors.CYAN}{command}{Colors.RESET}" if command else ""
    # Calculate padding for colored string
    visible_len = len(command) if command else 0
    padding = 16 - visible_len
    output += command_str + " " * padding + " | "

    # Data formatting - unified for all protocols
    hex_data = " ".join(f"{b:02X}" for b in frame.data) if frame.data else "(no data)"

    # Special formatting for NFC-A frames
    if frame.tech == "NfcA" and frame.data:
        if frame.is_poll():
            parsed_req = parse_nfca_request(frame)
            if parsed_req:
                # Request: Cmd | [Params] | [CRC]
                output += f"{frame.length:3}B | "
                output += f"Cmd:{parsed_req.cmd:02X} "
                if parsed_req.params:
                    output += f"Params:{parsed_req.params.hex().upper()} "
                if parsed_req.crc:
                    output += f"CRC:{parsed_req.crc.hex().upper()}"
            else:
                output += f"{frame.length:3}B | {hex_data}"
        else:
            parsed_resp = parse_nfca_response(frame)
            if parsed_resp:
                # Response: [Payload] | [CRC]
                output += f"{frame.length:3}B | "
                if parsed_resp.payload is not None:
                    payload_hex = parsed_resp.payload.hex().upper()
                    if len(payload_hex) > 32:
                        output += f"Payload[{len(parsed_resp.payload)}B]:{payload_hex[:32]}... "
                    else:
                        output += f"Payload:{payload_hex} "
                if parsed_resp.crc:
                    output += f"CRC:{parsed_resp.crc.hex().upper()}"
            else:
                output += f"{frame.length:3}B | {hex_data}"

    # Special formatting for NFC-B frames
    elif frame.tech == "NfcB" and frame.data:
        if frame.is_poll():
            parsed_req = parse_nfcb_request(frame)
            if parsed_req:
                # Request: Cmd | [Params] | [CRC]
                output += f"{frame.length:3}B | "
                output += f"Cmd:{parsed_req.cmd:02X} "
                if parsed_req.params:
                    output += f"Params:{parsed_req.params.hex().upper()} "
                if parsed_req.crc:
                    output += f"CRC:{parsed_req.crc.hex().upper()}"
            else:
                output += f"{frame.length:3}B | {hex_data}"
        else:
            parsed_resp = parse_nfcb_response(frame)
            if parsed_resp:
                # Response: [Payload] | [CRC]
                output += f"{frame.length:3}B | "
                if parsed_resp.payload is not None:
                    payload_hex = parsed_resp.payload.hex().upper()
                    if len(payload_hex) > 32:
                        output += f"Payload[{len(parsed_resp.payload)}B]:{payload_hex[:32]}... "
                    else:
                        output += f"Payload:{payload_hex} "
                if parsed_resp.crc:
                    output += f"CRC:{parsed_resp.crc.hex().upper()}"
            else:
                output += f"{frame.length:3}B | {hex_data}"

    # Special formatting for NFC-F frames
    elif frame.tech == "NfcF" and frame.data:
        if frame.is_poll():
            parsed_req = parse_nfcf_request(frame)
            if parsed_req:
                # Request: L | Cmd | [Body]
                output += f"{frame.length:3}B | "
                if len(frame.data) > 0:
                    output += f"L:{frame.data[0]:02X} "
                output += f"Cmd:{parsed_req.cmd:02X} "
                if parsed_req.body:
                    body_hex = parsed_req.body.hex().upper()
                    if len(body_hex) > 32:
                        output += f"Body[{len(parsed_req.body)}B]:{body_hex[:32]}... "
                    else:
                        output += f"Body:{body_hex}"
            else:
                output += f"{frame.length:3}B | {hex_data}"
        else:
            parsed_resp = parse_nfcf_response(frame)
            if parsed_resp:
                # Response: L | Cmd | [Body]
                output += f"{frame.length:3}B | "
                if len(frame.data) > 0:
                    output += f"L:{frame.data[0]:02X} "
                output += f"Cmd:{parsed_resp.cmd:02X} "
                if parsed_resp.body:
                    body_hex = parsed_resp.body.hex().upper()
                    if len(body_hex) > 32:
                        output += f"Body[{len(parsed_resp.body)}B]:{body_hex[:32]}... "
                    else:
                        output += f"Body:{body_hex}"
            else:
                output += f"{frame.length:3}B | {hex_data}"

    # Special formatting for NFC-V frames
    elif frame.tech == "NfcV" and frame.data:
        if frame.is_poll():
            parsed_req = parse_nfcv_request(frame)
            if parsed_req:
                # Request: Flags | Cmd | [UID] | [Params] | CRC
                output += f"{frame.length:3}B | "
                output += f"Flag:{parsed_req.flags:02X} "
                output += f"Cmd:{parsed_req.cmd:02X} "
                if parsed_req.uid:
                    uid_be = parsed_req.uid_be
                    if uid_be:
                        output += f"UID:{uid_be.hex().upper()} "
                if parsed_req.params:
                    output += f"Params:{parsed_req.params.hex().upper()} "
                if parsed_req.crc:
                    output += f"CRC:{parsed_req.crc.hex().upper()}"
            else:
                output += f"{frame.length:3}B | {hex_data}"
        else:
            parsed_resp = parse_nfcv_response(frame)
            if parsed_resp:
                # Response: Flags | [ERR/Payload] | CRC
                output += f"{frame.length:3}B | "
                output += f"Flag:{parsed_resp.flags:02X} "
                if parsed_resp.error_code is not None:
                    output += (
                        f"{Colors.RED}ERR:{parsed_resp.error_code:02X}{Colors.RESET} "
                    )
                elif parsed_resp.payload is not None:
                    # Show length for long payloads
                    payload_hex = parsed_resp.payload.hex().upper()
                    if len(payload_hex) > 32:
                        output += f"Payload[{len(parsed_resp.payload)}B]:{payload_hex[:32]}... "
                    else:
                        output += f"Payload:{payload_hex} "
                if parsed_resp.crc:
                    output += f"CRC:{parsed_resp.crc.hex().upper()}"
            else:
                output += f"{frame.length:3}B | {hex_data}"
    else:
        # Standard formatting for other protocols
        output += f"{frame.length:3}B | {hex_data}"

    if frame.errors:
        output += f" {Colors.RED}[{', '.join(frame.errors)}]{Colors.RESET}"

    return output


def read_live_stream(stream) -> Iterator[NFCFrame]:
    """
    Read frames from live JSON stream (one frame per line).

    This handles the extended JSON format from ./nfc-lab -j
    """
    for line in stream:
        line = line.strip()

        # Skip comments and empty lines
        if not line or line.startswith("#"):
            continue

        try:
            data = json.loads(line)

            # Convert extended live format to TRZ format
            # Live format has both camelCase and snake_case, we normalize to TRZ camelCase
            trz_data = {
                "sampleStart": data.get("sample_start", 0),
                "sampleEnd": data.get("sample_end", 0),
                "sampleRate": data.get("sample_rate", 3200000),
                "timeStart": data.get("time_start", data.get("timestamp", 0.0)),
                "timeEnd": data.get("time_end", data.get("timestamp", 0.0)),
                "dateTime": data.get("date_time", 0.0),
                "techType": data.get("tech_type", 0),
                "frameType": data.get("frame_type", 0),
                "framePhase": data.get(
                    "frame_phase", 0x0101
                ),  # Default to NfcCarrierPhase
                "frameRate": data.get("rate", data.get("frame_rate", 0)),
                "frameFlags": data.get("frame_flags", 0),
            }

            # Add frame data if present
            if "data" in data and data["data"]:
                # Live format has hex without separators
                hex_str = data["data"]
                # Add colons every 2 chars for TRZ format
                trz_data["frameData"] = ":".join(
                    hex_str[i : i + 2] for i in range(0, len(hex_str), 2)
                )

            yield NFCFrame.from_trz_dict(trz_data)

        except (json.JSONDecodeError, KeyError):
            # Skip invalid lines silently (nfc-lab may output non-JSON status messages)
            continue


def process_frames(frames: Iterator[NFCFrame], show_carrier: bool = True) -> int:
    """Process and display frames"""
    count = 0

    for frame in frames:
        # Filter carrier events if requested
        if not show_carrier and frame.is_carrier():
            continue

        print(format_frame(frame))
        sys.stdout.flush()
        count += 1

    return count


def main():
    parser = argparse.ArgumentParser(
        description="NFC Frame Monitor",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "file", nargs="?", help="TRZ file to analyze (omit for live stdin mode)"
    )
    parser.add_argument(
        "--no-carrier",
        action="store_true",
        help="Hide carrier on/off events",
    )
    args = parser.parse_args()

    show_carrier = not args.no_carrier

    # Print header
    print(f"{Colors.CYAN}{Colors.BOLD}NFC Frame Monitor{Colors.RESET}")
    if args.file:
        print(f"{Colors.CYAN}File: {args.file}{Colors.RESET}")
    else:
        print(f"{Colors.CYAN}Live mode - reading from stdin{Colors.RESET}")
        print(f"{Colors.GRAY}(Press Ctrl+C to stop){Colors.RESET}")
    print("=" * 80)

    try:
        if args.file:
            # File mode - read TRZ
            file_path = Path(args.file)
            if not file_path.exists():
                print(
                    f"{Colors.RED}Error: File not found: {args.file}{Colors.RESET}",
                    file=sys.stderr,
                )
                sys.exit(1)

            reader = TRZReader(file_path)
            frame_count = process_frames(reader.read_frames(), show_carrier)
        else:
            # Live mode - read from stdin
            frame_count = process_frames(read_live_stream(sys.stdin), show_carrier)

        # Print statistics
        print(f"\n{'=' * 80}")
        print(f"{Colors.YELLOW}Total frames displayed: {frame_count}{Colors.RESET}")

    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Stopped by user{Colors.RESET}")
        sys.exit(0)
    except Exception as e:
        print(f"{Colors.RED}Error: {e}{Colors.RESET}", file=sys.stderr)
        import traceback

        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
