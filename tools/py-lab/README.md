# NFC Laboratory - Python Library (py_nfclab) - Steffen W

Python library for NFC trace analysis - read TRZ files, detect protocols, analyze frames.

## Features

- Read TRZ files (complete format support)
- Read live JSON streams (`./nfc-lab -j`)
- Protocol command detection (93 commands)
- Protocol-specific frame parsers (NFC-A/B/F/V)
- JSON export
- CLI tool with color output and structured frame display

## Usage

No installation required - just use it directly from the repository:

```bash
# From nfc-laboratory directory
python3 -m py_nfclab trace.trz

# Or with Python import
cd /path/to/nfc-laboratory
python3
>>> from py_nfclab import read_trz
>>> frames = read_trz("trace.trz")
```

## Quick Start

### Python API

```python
from py_nfclab import read_trz, detect_command, write_json

# Read TRZ file
frames = read_trz("trace.trz")

# Analyze frames
for frame in frames:
    if frame.is_poll():
        cmd = detect_command(frame)
        print(f"{frame.timestamp:.6f}s {frame.tech:10} {cmd or 'Unknown':15} {frame.data.hex()}")

# Export to JSON
write_json(frames, "output.json")
```

### CLI Tool

```bash
# Analyze TRZ file
python3 -m py_nfclab trace.trz

# Live monitoring
./nfc-lab -j | python3 -m py_nfclab

# Hide carrier events
python3 -m py_nfclab trace.trz --no-carrier
```

## API Reference

### Reading

```python
from py_nfclab import read_trz, TRZReader

# Simple read
frames = read_trz("trace.trz")

# Iterator (memory efficient)
reader = TRZReader("trace.trz")
for frame in reader.read_frames():
    print(frame)
```

### Frame Properties

```python
frame.timestamp    # Time in seconds (float)
frame.tech         # "NfcA", "NfcB", "NfcF", "NfcV"
frame.type         # "Poll", "Listen", "CarrierOn", "CarrierOff"
frame.phase        # "NfcSelectionPhase", "NfcApplicationPhase"
frame.data         # Frame payload (bytes)
frame.length       # Data length in bytes
frame.rate         # Bitrate in bps
frame.errors       # ["CRC", "PARITY", "SYNC", "TRUNCATED"]

# Methods
frame.is_poll()    # Poll/Request frame
frame.is_listen()  # Listen/Response frame
frame.is_carrier() # Carrier on/off event
```

### Protocol Detection

```python
from py_nfclab import detect_command

cmd = detect_command(frame)  # "WUPA", "REQB", "Inventory", etc.
```

Supports 93 commands across NFC-A/B/F/V, ISO-DEP, and ISO7816.

### Protocol Frame Parsing

```python
from py_nfclab import (
    parse_nfca_request, parse_nfca_response,
    parse_nfcb_request, parse_nfcb_response,
    parse_nfcf_request, parse_nfcf_response,
    parse_nfcv_request, parse_nfcv_response
)

# Parse NFC-A frames
if frame.tech == "NfcA" and frame.is_poll():
    req = parse_nfca_request(frame)
    print(f"Cmd:{req.cmd:02X} Params:{req.params.hex()} CRC:{req.crc.hex()}")

# Parse NFC-V with context
resp = parse_nfcv_response(frame, request_cmd=0x01)  # Inventory
print(f"UID: {resp.uid_be.hex()}")  # Automatic UID extraction
```

Parsers extract structured data (cmd, params, payload, crc) for all major NFC protocols.

### Export

```python
from py_nfclab import write_json, write_jsonl

# TRZ-compatible JSON
write_json(frames, "output.json")

# JSON Lines (one frame per line)
write_jsonl(frames, "output.jsonl")
```

## TRZ File Format

TRZ is a tar.gz archive containing `frame.json` with frame metadata.

### Frame Fields

Each frame in `frame.json` contains 11 fields:

| Field | Type | Description |
|-------|------|-------------|
| `sampleStart/End` | int | Sample indices |
| `sampleRate` | int | Sample rate in Hz (e.g., 3200000) |
| `timeStart/End` | float | Time in seconds (relative) |
| `dateTime` | float | Absolute Unix timestamp |
| `techType` | int | Tech: 0x0101=NfcA, 0x0102=NfcB, 0x0103=NfcF, 0x0104=NfcV |
| `frameType` | int | Type: 0x0100=CarrierOff, 0x0101=CarrierOn, 0x0102=Poll, 0x0103=Listen |
| `framePhase` | int | Phase: 0x0102=SelectionPhase, 0x0103=ApplicationPhase |
| `frameRate` | int | Bitrate in bps |
| `frameFlags` | int | Errors: 0x20=CRC, 0x10=Parity, 0x08=Truncated, 0x40=Sync |
| `frameData` | string | Hex payload: `"AA:BB:CC"` (optional for carrier events) |

## License

Same as nfc-laboratory main project.
