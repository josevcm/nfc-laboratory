#!/usr/bin/env python3
"""
NFC Frame Monitor - py_nfclab CLI

Displays NFC frames in human-readable format with transaction pairing.
Supports both live streams (stdin) and TRZ files.

Usage:
    # Live monitoring
    ./nfc-lab -j | python3 -m py_nfclab

    # Analyze TRZ file
    python3 -m py_nfclab trace.trz

    # Without transaction pairing
    ./nfc-lab -j | python3 -m py_nfclab --no-pairing
    python3 -m py_nfclab trace.trz --no-pairing
"""

import argparse
import sys
from pathlib import Path

from . import NFCFrame, NFCTransaction, read_frames
from .transactions import StreamProcessor


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
    DARK_GRAY = "\033[2m"


TECH_COLORS = {
    "NfcA": Colors.GREEN,
    "NfcB": Colors.BLUE,
    "NfcF": Colors.MAGENTA,
    "NfcV": Colors.CYAN,
    "ISO7816": Colors.YELLOW,
}

TYPE_COLORS = {
    "Poll": Colors.BOLD + Colors.YELLOW,
    "Listen": Colors.GRAY,
    "CarrierOn": Colors.GREEN,
    "CarrierOff": Colors.RED,
}


def format_frame(frame: NFCFrame, indent: str = "") -> str:
    """Format a single frame for display"""
    hex_data = " ".join(
        frame.data_hex[i : i + 2] for i in range(0, len(frame.data_hex), 2)
    )
    tech_color = TECH_COLORS.get(frame.tech, Colors.RESET)
    type_color = TYPE_COLORS.get(frame.frame_type, Colors.RESET)

    output = f"{indent}[{Colors.BOLD}{frame.timestamp:>12.6f}{Colors.RESET}] "
    output += f"{tech_color}{frame.tech:>8}{Colors.RESET} "
    output += f"{frame.rate or 0:>6} " if frame.rate else "       "
    output += f"{type_color}{frame.frame_type:>10}{Colors.RESET} | "
    output += (
        f"{Colors.CYAN}{frame.event or '':>14}{Colors.RESET} | "
        if frame.event
        else " " * 17 + "| "
    )
    output += f"{frame.length:3} bytes | {hex_data}"

    if frame.flags:
        output += f" {Colors.MAGENTA}[{', '.join(frame.flags)}]{Colors.RESET}"
    if frame.errors:
        output += f" {Colors.RED}[{', '.join(frame.errors)}]{Colors.RESET}"

    return output


def format_transaction(transaction: NFCTransaction) -> str:
    """Format a transaction (Poll + Listen pair) for display"""
    lines = [format_frame(transaction.poll)]

    if transaction.listen:
        lines.append(format_frame(transaction.listen, "└─ "))
        duration_ms = transaction.duration * 1000 if transaction.duration else 0
        summary = f"  {Colors.DARK_GRAY}└─ {transaction.command or 'Unknown'}, {duration_ms:.3f}ms"
        if transaction.has_error():
            summary += f" {Colors.RED}[ERROR]{Colors.RESET}"
        lines.append(summary + Colors.RESET)
    else:
        lines.append(f"  {Colors.DARK_GRAY}└─ [No response]{Colors.RESET}")

    return "\n".join(lines)


def process_stream(stream, pair_transactions: bool = True):
    """Process frames from stdin (live mode)"""
    processor = StreamProcessor()
    frame_count = [0]
    transaction_count = [0]

    def on_frame(frame):
        frame_count[0] += 1
        if not pair_transactions:
            print(format_frame(frame))
            sys.stdout.flush()

    def on_transaction(transaction):
        transaction_count[0] += 1
        print(format_transaction(transaction))
        print()
        sys.stdout.flush()

    processor.add_frame_handler(on_frame)
    if pair_transactions:
        processor.add_transaction_handler(on_transaction)

    try:
        for line in stream:
            processor.process_json_line(line)
    except KeyboardInterrupt:
        pass
    finally:
        processor.finalize()

    return frame_count[0], transaction_count[0]


def process_file(file_path: Path, pair_transactions: bool = True):
    """Process frames from TRZ file (file mode)"""
    processor = StreamProcessor()
    frame_count = [0]
    transaction_count = [0]

    def on_frame(frame):
        frame_count[0] += 1
        if not pair_transactions:
            print(format_frame(frame))

    def on_transaction(transaction):
        transaction_count[0] += 1
        print(format_transaction(transaction))
        print()

    processor.add_frame_handler(on_frame)
    if pair_transactions:
        processor.add_transaction_handler(on_transaction)

    for frame in read_frames(file_path):
        processor.process_frame(frame)

    processor.finalize()

    return frame_count[0], transaction_count[0]


def main():
    parser = argparse.ArgumentParser(description="NFC Frame Monitor")
    parser.add_argument(
        "file", nargs="?", help="TRZ file to analyze (omit for live mode)"
    )
    parser.add_argument(
        "--no-pairing", action="store_true", help="Disable transaction pairing"
    )
    args = parser.parse_args()

    pair_transactions = not args.no_pairing

    # Print header
    print(f"{Colors.CYAN}{Colors.BOLD}NFC Frame Monitor{Colors.RESET}")
    if args.file:
        print(f"{Colors.CYAN}Analyzing: {args.file}{Colors.RESET}")
    else:
        print(f"{Colors.CYAN}Live mode (Press Ctrl+C to stop){Colors.RESET}")
    if pair_transactions:
        print(f"{Colors.CYAN}Transaction pairing enabled{Colors.RESET}")
    print("=" * 100)

    try:
        if args.file:
            # File mode
            file_path = Path(args.file)
            if not file_path.exists():
                print(
                    f"{Colors.RED}Error: File not found: {args.file}{Colors.RESET}",
                    file=sys.stderr,
                )
                sys.exit(1)
            frame_count, transaction_count = process_file(file_path, pair_transactions)
        else:
            # Live mode
            frame_count, transaction_count = process_stream(
                sys.stdin, pair_transactions
            )

        # Print statistics
        print(f"\n{'=' * 100}")
        print(
            f"{Colors.YELLOW}{'Analysis complete' if args.file else 'Monitoring stopped'}{Colors.RESET}"
        )
        print(f"Total frames: {frame_count}")
        if pair_transactions:
            print(f"Total transactions: {transaction_count}")

    except Exception as e:
        print(f"{Colors.RED}Error: {e}{Colors.RESET}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
