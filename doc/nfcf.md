# NFC-F decoder (JIS X 6319-4 / FeliCa)

This document describes how the NFC-F decoder in `lab-radio` works, and records the errors and optimizations found while reviewing it. It follows the structure of `doc/nfca.md`, where the common signal preprocessing and the per bit rate parameters are described (sections 2 and 3).

Files:

| File | Contents |
|---|---|
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcF.cpp` | Demodulator, frame decoder and protocol logic (`NfcF::Impl`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcF.h` | Public interface and command codes |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcTech.h` | Shared structures and correlation helper (`correlationPoints`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/include/lab/nfc/Nfc.h` | Timing constants (`NFCF_*`) |
| `src/nfc-test/test-decoder/scripts/nfcf_synthetic.py` | Synthetic NFC-F capture generator (section 7) |

---

## 1. Overview

NFC-F uses the same coding in both directions: Manchester coded ASK at 212 or 424 kbps, with no subcarrier. Every frame is:

```
preamble (48 bits of 0) | sync B2 4D | LEN | payload | CRC-16
```

`LEN` counts itself and the payload (up to 255). The CRC-16 is CCITT, MSB first, initial value 0, computed over `LEN` and the payload. The decoded `RawFrame` holds `LEN`, payload and CRC; the preamble and sync bytes are removed.

```
NfcDecoder::nextFrames()
   ├─ no active modulation → NfcF::detect() on every sample (212 and 424 kbps)
   └─ NFC-F modulation     → NfcF::decode()
                                 ├─ decodePollFrame()    (reader → card)
                                 └─ decodeListenFrame()  (card → reader, same coding)
```

Unlike NFC-A and NFC-B, both bit rates use `symbolDelayDetect = 0`. A 424 kbps preamble is not taken as a 212 kbps one because the preamble length check (48 symbols) fails at the wrong rate.

## 2. Reader frame detection (`detectModulation`)

Runs on every sample while there is no active modulation, for 212 and 424 kbps. It uses the same square template correlator as NFC-A (`doc/nfca.md`, section 4 and finding E7): `S0` is the second half of the last symbol minus the first half, `S1 ≈ −S0`, and `SD = |S0 − S1| / P2`.

1. **First pulse**: a peak of `SD` above `envelope · correlationThreshold` (0.5). The search window moves with the peak until ⅛ symbol after it.
2. **Preamble**: in a Manchester stream of zeros there is a transition at every half symbol, so a sync point is set every ½ symbol with a ±⅛ symbol window, and each peak must be above half the previous one. The first 94 half symbols (47 bits) must all be valid, otherwise the search restarts.
3. **End of preamble**: the preamble continues while transitions keep appearing at every half symbol. The first `1` of the sync byte `B2` breaks the pattern and ends it.
4. **Polarity and length**: the sign of `S0` at the last sync point gives the Manchester polarity (`SEARCH_MODE_OBSERVED` or `SEARCH_MODE_REVERSED`) and corrects the start by ½ symbol if needed. The preamble length must be 48 symbols ± ¼ symbol.
5. On success it sets `decoder->bitrate` and `decoder->modulation`, the frame type, and the first symbol window.

The search is also reset when the modulation depth exceeds `maximumModulationDeep` (90%), which discards NFC-A frames.

## 3. Symbols and frames

### 3.1 Symbols (`decodePollFrameSymbolAsk`, `decodeListenFrameSymbolAsk`)

- Each symbol has a window `[sync − ¼, sync + ¼]` around the expected mid-bit transition. The largest `SD` above the threshold resynchronizes the symbol end; the threshold is half the last peak.
- At `sync`, the comparison of `S0` and `S1`, with the detected polarity, gives **pattern L** (0) or **pattern H** (1).
- No peak in the window means there is no transition: **pattern E**, end of frame.

### 3.2 Frames (`decodePollFrame`, `decodeListenFrame`)

- Bits are MSB first. Bytes include the two sync bytes; a frame needs more than 2 bytes.
- Sync bytes other than `B2 4D` set `SyncError`.
- If the stream reaches the maximum frame size (2 + 255 + 2 bytes) the frame is flagged `Truncated`.

### 3.3 Card frame start (`decodeListenFrameStartAsk`)

Same preamble search as the reader detection, on the card load modulation. It starts one symbol before `guardEnd` to fill the correlator, uses `meanDeviation · 10` as the noise threshold, and returns `NoPattern` after `waitingEnd`. The threshold is calibrated after the guard time and again after every failed preamble candidate.

## 4. Protocol layer (`process`)

| Function | Recognizes (poll) | Effect |
|---|---|---|
| `processREQC` | Polling: `06 00 SC SC RC TSN` + CRC (8 bytes) | Resets the protocol parameters. Response window: FGT and `NFCF_FDT_ATQC + (TSN + 1) · NFCF_TSU_ATQC`. The next card frame is the polling response (selection phase) |
| `processOther` | anything else | Application phase, CRC check |

The Polling command code is `0x00`, the same value `lastCommand` uses for "no command", so a separate `pollingRequest` flag marks a pending polling response. It is cleared by any following frame and on response timeout.

Default timings (1/fc): FGT 1024, FWT `256·16·2⁴` (≈4.8 ms), polling response `512·64` plus `256·64` per time slot.

---

## 5. Errors found

Line numbers refer to `tech/NfcF.cpp` before the fixes. F1 to F3 were confirmed with synthetic captures before being fixed.

### Fix status

| Error | Status | Applied fix |
|---|---|---|
| F1 Every response classified as polling response | **Fixed** | Separate `pollingRequest` flag; the reference captures `test_NFC-F_212kbps_002` and `_003` were updated (39 responses to Read move to the application phase) |
| F2 Maximum length frames truncated | **Fixed** | Frame limit of 2 + 255 + 2 bytes instead of 256 |
| F3 Card response lost after a spurious burst | **Fixed** | The listen threshold is recalibrated when it is 0, after the guard time and after each failed preamble candidate |
| F4 Polling detection | **Fixed** | Requires `LEN = 6` and 8 bytes; `frame[5]` (TSN) is no longer read from shorter frames |
| F5 Pending command after timeout | **Fixed** | Cleared in `resetModulation()` |
| F6 Command name | **Fixed** | `NFCB_REQC` renamed to `NFCF_REQC` |
| F7 Minimum reader modulation depth | Pending | With `correlationThreshold = 0.5` only readers with about 25% depth or more are detected (question for the author) |
| F8 `minimumModulationDeep` unused | Pending | Its checks are commented out; the setting has no effect |
| F9 Reader frame during the response wait | Pending | Both directions use the same coding, so a reader frame sent before `waitingEnd` is decoded as a card response |
| F10 Command response times | Pending | The PMm response time parameters are not used; every non-polling command waits 4.8 ms |
| F11 `LEN` not used | Pending | The frame end is only given by the missing transition; `LEN` could validate it |
| F12 32-bit clock and unused fields | Pending | As in NFC-A |

### F1. Every card frame is classified as a polling response. **Medium**

`processREQC` stored the Polling command code in `frameStatus.lastCommand` and recognized the response with `lastCommand == NFCB_REQC`. The code is `0x00`, and `lastCommand` is also reset to 0 after every response, so any card frame matched. Responses to Read, Write and every other command were put in the selection phase. The reference captures had this error recorded: Read commands (`06`) were in the application phase but their responses (`07`) in the selection phase.

### F2. Maximum length frames are truncated. **Medium**

The frame limit was 256 bytes, but the stream also holds the two sync bytes and the CRC, so a frame with `LEN = 255` takes 259 bytes. Its last bytes were cut and the frame was flagged `Truncated` with a CRC error.

### F3. A spurious burst before the response loses it. **Medium**

Every failed preamble candidate in `decodeListenFrameStartAsk` reset `searchValueThreshold` to 0, but the threshold was only calibrated when `clock == guardEnd`. After a short burst (noise, or a reader modulation tail) the rest of the search ran with a zero threshold, every sample was a peak, and the real preamble was not found.

### F4 to F6. Other details. **Low**

- Polling was recognized by `frame[1] == 0` alone, and `frame[5]` was read without checking the frame length.
- After an unanswered Polling the pending state survived the timeout.
- `NfcF.h` named the Polling command `NFCB_REQC`.

### F7 to F12. Pending

- **Minimum modulation depth**: `SD ≈ 2 · depth · envelope`, so the detection threshold `envelope · 0.5` needs a depth of about 25%. The reference captures have 25–80% (reader) and 25–35% (card), but ISO/IEC 18092 allows readers from 8%. Lowering the default would increase false detections; it is a tuning decision.
- **`minimumModulationDeep`**: the depth checks are commented out, so the NFC-F minimum threshold setting has no effect.
- **Reader frame during the wait**: card and reader frames look the same to the decoder. A reader frame sent before `waitingEnd` (after an unanswered command, for example) is decoded as a card response. The modulation depth could tell them apart, but that needs real captures to tune.
- **Response times**: FeliCa cards announce per-command maximum response times in PMm. The decoder uses 4.8 ms for every command, so a slower response (some write commands) is lost.
- **`LEN`**: not used to delimit or validate the frame; the CRC catches most inconsistencies.

## 6. Optimizations

All of them keep the output byte-identical on the 19 reference captures and on the synthetic captures of the four technologies.

1. **Shared correlation helper**: `NfcTech::correlationPoints()` (moved from NFC-A) computes the three buffer points with one integer division instead of three, in detection and in both symbol decoders.
2. **Precomputed inverse** of `period2SymbolSamples`, and **local copies** of the signal clock and debug pointer in `detectModulation`.
3. **Duplicated code removed**: `resetSearch()` replaces the 4 preamble search reset blocks and `buildFrame()` the 2 frame building blocks, including the sync byte check.

**Measured performance**: no measurable change (about +1% with only NFC-F enabled, within the noise). Callgrind even counts slightly more instructions: on a current CPU the integer divisions are cheap. With only NFC-F enabled, almost half of the time (47%) is spent in the common `NfcDecoderStatus::nextSample()`, which is the real optimization target for all technologies (it divides by the envelope twice per sample).

## 7. Tests

The decoder tests are described in `doc/nfca.md`, section 10. NFC-F adds `scripts/nfcf_synthetic.py`, which uses the helpers in `scripts/synthetic_common.py`. It generates Manchester coded ASK frames (30% reader modulation, 28% card modulation) with preamble, sync, `LEN` and CRC. CTest runs it into the build directory (`decoder-nfcf-synthetic-generate`) and then one test per scenario.

| Scenario | Before the fixes | After the fixes |
|---|---|---|
| `basic`: Polling, response and Read at 212 kbps (F1) | **Fails**: Read response in the selection phase | Pass |
| `basic_424`: same at 424 kbps (F1) | **Fails**: same | Pass |
| `max_length`: card frame with `LEN = 255` (F2) | **Fails**: truncated, CRC error | Pass |
| `spurious`: burst before the card response (F3) | **Fails**: response lost | Pass |
| `unanswered`: Polling without response, then Read (F1, F5) | **Fails**: Read response in the selection phase | Pass |
