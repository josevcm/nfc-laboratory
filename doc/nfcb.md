# NFC-B decoder (ISO/IEC 14443 type B)

This document describes how the NFC-B decoder in `lab-radio` works, and records the errors and optimizations found while reviewing it. It follows the same structure as `doc/nfca.md`; the common signal preprocessing and the per bit rate parameters are described there (sections 2 and 3) and are shared by both decoders.

Files:

| File | Contents |
|---|---|
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcB.cpp` | Demodulator, frame decoder and protocol logic (`NfcB::Impl`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcB.h` | Public interface and command codes |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcTech.h` | Structures shared by all technologies, frame size conversion (`frameSizeFromIndex`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/include/lab/nfc/Nfc.h` | Timing constants (`NFCB_*`, FSD/FWT tables) |
| `src/nfc-test/test-decoder/scripts/nfcb_synthetic.py` | Synthetic NFC-B capture generator (section 10) |

---

## 1. Overview

```
NfcDecoder::nextFrames()
   ├─ no active modulation → NfcB::detect() on every sample (106 and 212 kbps)
   └─ NFC-B modulation     → NfcB::decode()
                                 ├─ decodePollFrame()    (reader → card, 10% ASK + NRZ-L)
                                 └─ decodeListenFrame()  (card → reader, subcarrier BPSK + NRZ-L)
        │
        ▼
process(RawFrame) → protocol phase, CRC, timings (TR0/FWT/TR1) for the next frame
```

Type B frames are made of characters: start bit (0), 8 data bits LSB first, stop bit (1). Characters may be separated by an extra guard time (EGT) of logic 1. A frame is delimited by a Start of Sequence (SoS) and an End of Sequence (EoS):

| | Reader (PCD) | Card (PICC) |
|---|---|---|
| Modulation | 10% ASK of the carrier | BPSK of the fc/16 subcarrier |
| Before SoS | — | TR1: unmodulated subcarrier, phase reference (logic 1) |
| SoS | 10–11 ETU of 0, then 2–3 ETU of 1 | 10–11 ETU of 0, then 2–3 ETU of 1 |
| EGT | 0–6 ETU | 0–2 ETU |
| EoS | 10–11 ETU of 0 | 10–11 ETU of 0, then subcarrier off |

The decoder state is split as in NFC-A (`bitrateParams`, `modulationStatus`, `symbolStatus`, `streamStatus`, `frameStatus`, `protocolStatus`). `protocolStatus` holds FSD (`maxFrameSize`, card frames limit), FSC (`maxPollFrameSize`, reader frames limit), FWT, TR0 and the TR1/SoS timing windows.

## 2. Reader frame detection (`detectModulation`)

Runs on every sample while there is no active modulation, for 106 and 212 kbps. It works on `filteredValue`, the DC-removed signal, which behaves as an edge detector: a falling edge of the carrier gives a negative peak and a rising edge a positive one. For each bit rate it is a three-stage search for the SoS:

1. **Falling edge**: `filteredValue < −envelope·minimumModulationDeep` (10%). The peak is tracked until ¼ symbol after the last maximum. The SoS starts ⅛ symbol before the peak.
2. **Rising edge after 10–11 ETU**: any rising edge above half the first peak before `start + 10 ETU − ½` resets the search. Inside the window `[10 ETU − ½, 11 ETU + ½]` it searches for the positive peak; if none is found the search restarts.
3. **Falling edge after 2–3 ETU**: any falling edge before `2 ETU − ½` resets the search. Inside `[2 ETU − ½, 3 ETU + ½]` it searches for the negative peak, which is the start bit of the first character.

When the third edge is found:

- `decoder->bitrate` and `decoder->modulation` are set, and `frameStatus.frameType = NfcPollFrame`;
- the first synchronization point is set to the middle of the start bit (`edge + ½ symbol`);
- the edge threshold for resynchronization is half the last peak.

The search is also reset when the modulation depth exceeds `maximumModulationDeep` (90%), which discards NFC-A frames, or when a peak is older than one symbol.

## 3. Reader frame: 10% ASK + NRZ-L

### 3.1 Symbols (`decodePollFrameSymbolAsk`)

- After each symbol a resynchronization window `[sync + ¼, sync + ¾]` symbol is opened around the next expected boundary. The largest `|filteredValue|` above the threshold moves the next sync point to `edge + ½ symbol`.
- At each sync point, the modulation depth decides the symbol: depth above `minimumModulationDeep` is **pattern L** (0), otherwise **pattern H** (1).

### 3.2 Characters and frame (`decodePollFrame`)

- `bits == 0`: an L is the start bit; an H is EGT and is skipped, up to 6 ETU. A seventh H is a stream error.
- `bits == 1..8`: data bits, LSB first.
- `bits == 9`: an H is the stop bit and completes the byte. An L with all data bits at 0 completes 10 ETU of 0, which is the **EoS**. An L with data is a stream error.
- If a complete character is received when the frame already has `maxPollFrameSize` bytes, the frame is flagged `Truncated`.
- A valid frame has at least 3 bytes (command + CRC_B). Frames ending on a stream error are flagged `Truncated`.

## 4. Card frame: subcarrier BPSK + NRZ-L

`process()` sets `guardEnd = frameEnd + TR0` and `waitingEnd = frameEnd + FWT` after each reader frame.

### 4.1 SoS (`decodeListenFrameStartBpsk`)

- Differential phase demodulation: `filtered(t) · filtered(t − 1 symbol) · 10`, integrated over ¼ symbol (`phaseIntegrate`). It is positive when the subcarrier keeps its phase over one symbol and negative right after a phase inversion, so with NRZ-L it marks the bit transitions.
- From `guardEnd` the threshold is calibrated from the noise (`meanDeviation`). After `waitingEnd`, or when a modulation deeper than `maximumModulationDeep` appears, it returns `NoPattern` (timeout).
- Three positive phase periods are measured, each one ended by a phase inversion:
  1. **TR1**: phase reference before the SoS, in `[tr1MinimumTime, tr1MaximumTime]`. The measured length is about one symbol shorter than the real TR1, because the detector needs one symbol of history.
  2. **S1**: the 10–11 ETU of logic 0 (`NFCB_TLISTEN_S1_MIN..MAX`).
  3. **S2**: the 2–3 ETU of logic 1 (`NFCB_TLISTEN_S2_MIN..MAX`).
- The phase peak seen during the SoS sets the phase threshold for symbol decoding (25% of the peak).

### 4.2 Symbols (`decodeListenFrameSymbolBpsk`)

Same as NFC-A BPSK (see `doc/nfca.md`, section 6.2): a zero-crossing detector resynchronizes once per symbol, a phase inversion toggles the bit value (patterns M = 0, N = 1), and when the phase stays below the threshold the subcarrier is gone (**pattern O**).

### 4.3 Characters and frame (`decodeListenFrame`)

- Same character structure as the reader frame, with EGT of logic 1 skipped up to 2 ETU.
- EoS: 10 ETU of 0 (`bits == 9`, data 0, pattern M).
- Pattern O (subcarrier lost) ends the frame. Between characters it is a normal end (the EoS may be suppressed by ATTRIB); inside a character it is a stream error.
- A complete character received when the frame already has `maxFrameSize` bytes flags the frame `Truncated`.
- The frame end is extended by 352/fc to cover the EoS, which is not detected completely.

## 5. Protocol layer (`process`)

| Function | Recognizes (poll) | Effect |
|---|---|---|
| `processREQB` | `05 AFI PARAM` + CRC (5 bytes), or Slot-MARKER `n5` + CRC (3 bytes) | REQB/WUPB resets the protocol parameters. Response window: TR0 min and `NFCB_FWT_ATQB`. The ATQB answer (`0x50`, 14 or 15 bytes) gives FSC (FSCI) and FWT (FWI, 15 → 4) |
| `processATTRIB` | `1D` + PUPI + 4 params, 11 bytes or more | FSD from FSDI, TR0 from param 1 (RFU → default), TR1 minimum from param 1, FWT = activation. The answer to ATTRIB is CRC checked |
| `processHLTB` | `50` + PUPI + CRC (7 bytes) | Selection phase, the `00` answer is CRC checked |
| `processOther` | anything else | Application phase, CRC check, forgets the last command. An S(WTX) response (`F2`/`FA`) extends the FWT of the next response to `FWT × WTXM` |

Frame size indexes 'D' to 'F' (RFU) are interpreted as 'C' (4096 bytes), and frame sizes are limited to the stream buffer capacity. `lastCommand` is cleared after each response and on response timeout.

---

## 6. Errors found

Line numbers refer to `tech/NfcB.cpp` before the fixes. B1 to B5 and B7 to B9 were confirmed with synthetic captures (section 10) before being fixed.

### Fix status

| Error | Status | Applied fix |
|---|---|---|
| B1 Frames over 512 bytes | **Fixed** | `buildFrame` allocates the `RawFrame` with the actual size. The stream buffer overflow had already been fixed with the NFC-A changes (`NfcStreamStatus::buffer` of 4096 + 2 bytes) |
| B2 RFU frame size index | **Fixed** | Shared `frameSizeFromIndex()` interprets 'D'–'F' as 'C' |
| B3 FSD and FSC mixed | **Fixed** | FSC (ATQB) limits reader frames and FSD (ATTRIB) limits card frames |
| B4 Exact size frames flagged truncated | **Fixed** | Truncation is decided when a complete character exceeds the limit, so the EoS of a frame of exactly the maximum size is decoded normally |
| B5 Card EGT | **Fixed** | Up to 2 ETU of logic 1 between card characters are skipped |
| B6 SoS phase threshold | **Fixed** | The phase peak is tracked during the SoS, so the threshold is 25% of the real peak instead of 0 |
| B7 TR1 from ATTRIB | **Fixed** | The minimum TR1 follows ATTRIB param 1, compensating the one symbol measurement delay |
| B8 Missing EoS (pattern O) | **Fixed** | Pattern O ends the frame instead of being decoded as a data bit |
| B9 S(WTX) | **Fixed** | An S(WTX) response from the reader extends the FWT of the next response |
| B10 ATQB parsing | **Fixed** | Requires `0x50` and at least 14 bytes; FWI = 15 is treated as 4 |
| B11 `lastCommand` after timeout | **Fixed** | Cleared in `resetModulation()` and when an unknown command is received |
| B12 HLTB and Slot-MARKER | **Fixed** | Recognized as selection phase; the ATQB answer to a Slot-MARKER is parsed |
| B13 ATTRIB answer CRC | **Fixed** | CRC checked |
| B14 Detection `break` | **Fixed** | A failed third stage at 106 kbps no longer skips the 212 kbps detector for that sample |
| B15 Threshold at exact `guardEnd` | **Fixed** | Calibrated on the first sample after the guard time |
| B16 TR0 RFU | **Fixed** | TR0 value 3 uses the default guard time instead of 0 |
| B17 424 kbps detection | Pending | The detection loop stops at 212 kbps although the comment says 424 kbps (question for the author) |
| B18 `correlationThreshold` unused | Pending | It can be configured from the API and UI but NFC-B never reads it |
| B19 Card bit rate and SoS suppression | Pending | ATTRIB can select a different card bit rate and suppress the SoS; the decoder assumes the reader bit rate and requires the SoS |
| B20 Reader frame during the wait | Pending | Only modulation deeper than 90% stops the response search; a 10% NFC-B reader frame before `waitingEnd` is not detected |
| B21 32-bit clock | Pending | Same as NFC-A E8, affects the whole decoder |
| B22 Unused fields | Pending | `startUpGuardTime`, `requestGuardTime`, `lastFrameEnd`, `tr1MaximumTime` is never updated from ATTRIB |

### B1. Frames over 512 bytes abort the decoder. **Severe**

`RawFrame(tech, type)` allocates 512 bytes, and `put()` of a larger frame fails an `assert`. The project Release flags do not define `NDEBUG`, so the application aborts. Before the NFC-A changes the stream buffer was also 512 bytes, and frames over that size corrupted memory (the decoder hung). It happens after an ATQB or ATTRIB announcing 1024 bytes or more.

### B2. RFU frame size index locks the decoder. **Severe**

`NFC_FDS_TABLE[13..15] = 0`, so an ATQB FSCI or ATTRIB FSDI of 'D'–'F' set `maxFrameSize = 0`. From then on every frame was truncated on its first symbol and discarded, until the next REQB.

### B3. FSD and FSC share one limit. **Medium**

A single `maxFrameSize` was set from the ATQB FSCI (what the card can receive) and then from the ATTRIB FSDI (what the reader can receive), and applied to both directions. With a small FSD, valid reader frames were truncated.

### B4. Frames of exactly the maximum size are flagged truncated. **Medium**

The truncation check (`bytes == maxFrameSize`) ran on the symbol after the last stop bit, which is the first symbol of the EoS. A frame of exactly FSD or FSC bytes, common in chained ISO-DEP transfers, was flagged `Truncated` and the rest of its EoS left undecoded.

### B5. Card EGT is a stream error. **Medium**

A logic 1 in the start bit position of a card frame was a stream error (l. 608), but ISO/IEC 14443-3 allows 0 to 2 ETU of EGT between card characters. Responses from cards using EGT were cut after the first byte.

### B6. SoS phase threshold is always zero. **Low**

At the end of the SoS the phase threshold was set from `detectorPeakValue`, which the listen SoS search never wrote, so it was 0. The first symbols were decided against a zero threshold and the EoS could not be detected until a symbol without phase change updated it.

### B7. TR1 from ATTRIB is ignored. **Medium**

ATTRIB param 1 can lower the minimum card TR1 to 64/fs or 16/fs (1024 or 256 /fc), but the decoder kept a fixed minimum of 1024/fc. Since the measured preamble is one symbol shorter than TR1, a card using the reduced TR1 was never detected.

### B8. Missing EoS hangs the response decoding. **Medium**

Pattern O (subcarrier lost) was not handled in `decodeListenFrame`: it was decoded as a data bit with a stale value. The symbol decoder returns pattern O without advancing the sync point, so the decoder stayed in that frame until a phase change resynchronized it. A card frame without EoS (allowed when ATTRIB suppresses it, or a truncated response) was lost together with all the following frames.

### B9. S(WTX) is not handled. **Medium**

As in NFC-A, a response sent after an S(WTX) extension exceeded the negotiated FWT and was lost as a timeout.

### B10. ATQB is parsed without checks. **Medium**

`frame[10]` and `frame[11]` were read from any answer to REQB, without checking its length or the `0x50` header, so short or corrupted answers set FSC and FWT from arbitrary bytes. FWI = 15 was not mapped to 4.

### B11. `lastCommand` survives timeouts. **Low**

As NFC-A E10: after an unanswered command, the next unknown frame's response was interpreted according to the old command. After an unanswered REQB, for example, it could be parsed as an ATQB and change FSC and FWT.

### B12. HLTB and Slot-MARKER are not recognized. **Low**

Both are part of the selection phase but were classified as application frames, and the ATQB answer to a Slot-MARKER was not parsed. The reference capture `test_NFC-B_106kbps_001` contains a Slot-MARKER (`15 54 B7`); its expected phase was updated accordingly.

### B13 to B16. Other details. **Low**

- The answer to ATTRIB was not CRC checked.
- When the third detection stage failed at 106 kbps the loop used `break` (l. 401), skipping the 212 kbps detector for that sample. If that sample was the end of a 212 kbps search window, its exact `clock == searchEndTime` check was missed and that detector could stay stuck until the next frame.
- The listen threshold was calibrated only when `clock == guardEnd` exactly.
- TR0 value 3 (RFU) set a guard time of 0.

### B17 to B22. Pending

- **424 kbps detection**: `detectModulation` loops `rate <= r212k`, while the comment mentions 424 kbps. It may be a deliberate limit (for example to avoid false detections) or an oversight.
- **`correlationThreshold`**: configurable but unused.
- **Card bit rate and SoS/EoS suppression**: ATTRIB param 2 can select a different bit rate for the card, and param 1 can suppress SoS and EoS. The decoder assumes the reader bit rate and requires the card SoS; a missing EoS is now tolerated (B8).
- **Reader frame during the wait**: `decodeListenFrameStartBpsk` only stops on modulation deeper than `maximumModulationDeep` (90%). A reader NFC-B frame (10%) sent before `waitingEnd`, for example after a very long FWT, is not detected until the wait expires.
- **32-bit clock** and **unused fields**: as in NFC-A.

## 7. Optimizations

### 7.1 Applied

They keep the decoder output byte-identical on the 19 captures in `wav/` and on the synthetic captures.

1. **Local copies in `detectModulation`.** `signalClock` and the debug pointer are read once per call.
2. **Edge threshold computed once.** `envelope · minimumModulationDeep` is the same for both bit rates and is computed once per sample.
3. **Duplicated code removed.** `resetSearch()` replaces the 5 SoS search reset blocks, `resetListenSearch()` the 3 card SoS reset blocks, and `buildFrame()` the 2 blocks that built a `RawFrame`. The search reset blocks cleared slightly different sets of fields; the extra fields are always zero at those points, so the result is the same.

**Measured performance**: about +2–4% with all four technologies enabled, at the limit of the measurement noise. The NFC-B detector was already light (about 12% of the total instructions, against 30% for NFC-A before its optimization). After the bug fixes the speed is the same as the original within the noise.

### 7.2 Discarded

- **Restructuring the edge detector**: the idle path is a handful of comparisons per bit rate; there is no division or memory-heavy work to remove.

## 8. Tests

The decoder tests are described in `doc/nfca.md`, section 10. They run with:

```
ctest --test-dir build -R decoder
```

NFC-B adds `scripts/nfcb_synthetic.py`, which generates 106 kbps NFC-B captures: the reader uses 10% ASK with NRZ-L coding, the card uses BPSK of the fc/16 subcarrier with TR1, SoS and EoS, and deterministic gaussian noise is added. CTest runs it into the build directory (`decoder-nfcb-synthetic-generate`) and then one test per scenario (`decoder-nfcb-synthetic-*`).

| Scenario | Before the fixes | After the fixes |
|---|---|---|
| `basic`: REQB, ATQB, ATTRIB, answer, I-block | Pass | Pass |
| `fsd_4096`: FSC and FSD of 4096 bytes, frames of 703 and 605 bytes (B1) | **Fails**: aborts (`RawFrame` capacity), hung before the NFC-A buffer fix | Both frames complete |
| `fsdi_rfu`: ATTRIB with FSDI = F (B2) | **Fails**: all following frames lost | All frames decoded |
| `frame_size`: FSD = 16, reader frame of 20 bytes and card frame of 16 (B3, B4) | **Fails**: reader frame cut at 16 bytes, card frame flagged truncated | Both frames correct |
| `egt`: 2 ETU of EGT between card characters (B5) | **Fails**: response cut after the first byte | Response complete |
| `tr1_short`: ATTRIB with TR1 = 64/fs (B7) | **Fails**: all responses lost | All responses decoded |
| `wtx`: S(WTX) and response at 12 ms, over the 4.8 ms FWT (B9) | **Fails**: response lost | Response decoded |
| `no_eos`: card frame without EoS, then a normal exchange (B8) | **Fails**: that frame and all following ones lost | All frames decoded |
| `hltb`: HLTB and its answer (B12) | Fails: classified as application phase | Selection phase |
| `slot_marker`: REQB with 4 slots, Slot-MARKER and ATQB (B12) | Fails: Slot-MARKER classified as application phase | Selection phase |

The reference capture `test_NFC-B_106kbps_001` changes in one frame, the Slot-MARKER `15 54 B7`, which is now in the selection phase (B12). Its `.json` was updated; every other frame of the 19 reference captures is unchanged.
