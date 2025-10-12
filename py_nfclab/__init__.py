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
from .models import NFCFrame

# Protocol detection
from .protocol import (
    NfcARequest,
    NfcAResponse,
    NfcBRequest,
    NfcBResponse,
    NfcFRequest,
    NfcFResponse,
    NfcVRequest,
    NfcVResponse,
    parse_nfca_request,
    parse_nfca_response,
    parse_nfcb_request,
    parse_nfcb_response,
    parse_nfcf_request,
    parse_nfcf_response,
    parse_nfcv_request,
    parse_nfcv_response,
)

# Readers
from .readers import read_trz

# Writers
from .writers import write_json, write_jsonl

__all__ = [
    # Core models
    "NFCFrame",
    # High-level I/O
    "read_trz",
    "write_json",
    "write_jsonl",
    # Protocol parsing
    "NfcARequest",
    "NfcAResponse",
    "parse_nfca_request",
    "parse_nfca_response",
    "NfcBRequest",
    "NfcBResponse",
    "parse_nfcb_request",
    "parse_nfcb_response",
    "NfcFRequest",
    "NfcFResponse",
    "parse_nfcf_request",
    "parse_nfcf_response",
    "NfcVRequest",
    "NfcVResponse",
    "parse_nfcv_request",
    "parse_nfcv_response",
]
