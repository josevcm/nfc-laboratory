# py_nfclab - NFC Laboratory Python Library

Python library for parsing and analyzing NFC frame data from nfc-lab.

## Architecture

```
py_nfclab/
├── models.py            # NFCFrame, NFCTransaction, Enums
├── protocol.py          # Protocol parser and event detection
├── transactions.py      # Transaction builder (Poll+Listen pairing)
└── readers.py           # TRZ archives and live JSON streams
```

## Core Classes

- **NFCFrame**: Single NFC frame with timing, data, tech type, and protocol event
- **NFCTransaction**: Poll/Listen pair with duration and command detection
- **ProtocolParser**: Detects protocol events (REQA, READ, etc.)
- **TransactionBuilder**: Pairs Poll frames with Listen responses
- **StreamProcessor**: Pipeline for frame parsing and transaction building

## Quick Start

### Read frames from TRZ or live stream

```python
from py_nfclab import read_frames

# TRZ file
for frame in read_frames("trace.trz"):
    print(f"{frame.timestamp:.6f} {frame.tech} {frame.frame_type}: {frame.data_hex}")

# Live stream
import sys
for frame in read_frames(sys.stdin):
    print(f"{frame.event}: {frame.data_hex}")
```

### Parse protocol events and build transactions

```python
from py_nfclab import read_frames, ProtocolParser, TransactionBuilder

parser = ProtocolParser()
builder = TransactionBuilder()

for frame in read_frames("trace.trz"):
    parser.parse(frame)
    transaction = builder.add_frame(frame)

    if transaction and transaction.is_complete():
        print(f"{transaction.poll.event} -> {transaction.listen.event}")
        print(f"  Duration: {transaction.duration*1000:.3f}ms")
```

### Stream processing with handlers

```python
from py_nfclab.transactions import StreamProcessor

processor = StreamProcessor()

processor.add_frame_handler(lambda f: print(f"Frame: {f.event}"))
processor.add_transaction_handler(lambda t: print(f"Transaction: {t.duration*1000:.2f}ms"))

import sys
for line in sys.stdin:
    processor.process_json_line(line)
processor.finalize()
```

## Supported Protocols

- **NFC-A** (ISO 14443-3A): REQA, WUPA, SEL, RATS, HLTA, MIFARE, Ultralight
- **NFC-B** (ISO 14443-3B): REQB, ATTRIB, HLTB
- **NFC-F** (FeliCa): REQC, ATQC
- **NFC-V** (ISO 15693): Inventory, Read/Write commands, NXP extensions
- **ISO-DEP**: I-Block, R-Block, S-Block across NFC-A/B
- **ISO7816**: T=1 protocol blocks, PPS

Event detection is context-aware (uses previous poll to interpret response) and based on the C++ StreamModel.cpp implementation.
