# NFC-A decoder (ISO/IEC 14443 type A)

This document describes how the NFC-A decoder in `lab-radio` works, and records the errors and optimizations found while reviewing it.

Files:

| File | Contents |
|---|---|
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcA.cpp` | Demodulator, frame decoder and protocol logic (`NfcA::Impl`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcA.h` | Public interface and command codes |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcTech.h` / `NfcTech.cpp` | Structures shared by all technologies and per-sample preprocessing (`nextSample`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcDecoder.cpp` | Orchestrator: carrier detection and dispatch to NFC-A/B/F/V |
| `src/nfc-lib/lib-lab/lab-radio/src/main/include/lab/nfc/Nfc.h` | Timing constants (FGT, FWT, SFGT, FSD/FWT tables) |
| `src/nfc-test/test-decoder` | Decoder regression test and synthetic NFC-A capture generator (section 10) |

---

## 1. Overview

```
SignalBuffer (signal magnitude, 1 float channel)
        │
        ▼
NfcDecoderStatus::nextSample()        ← common per-sample preprocessing
        │   envelope, DC filter (IIR), mean deviation, modulation depth
        │   → circular buffer sample[1024]
        ▼
NfcDecoder::nextFrames()
   ├─ no active modulation → NfcA::detect() (and B, F, V) on every sample
   └─ NFC-A modulation     → NfcA::decode()
                                 ├─ decodePollFrame()    (reader → card, 100% ASK + modified Miller)
                                 └─ decodeListenFrame()  (card → reader)
                                        ├─ 106 kbps: subcarrier OOK + Manchester
                                        └─ 212/424 kbps: subcarrier BPSK + NRZ-L
        │
        ▼
process(RawFrame) → protocol phase, CRC, timings (FGT/FWT) for the next frame
        │
        ▼
std::list<RawFrame>
```

The decoder is a state machine that processes one sample at a time. Its state is split into:

- `bitrateParams[rate]`: symbol lengths in samples for 106, 212 and 424 kbps (indexes `r106k`..`r424k`).
- `modulationStatus[rate]`: detector or demodulator state for each bit rate (integrators, search windows, peaks, buffers).
- `symbolStatus`: last decoded symbol (pattern, value, start, end, edge).
- `streamStatus`: bits and bytes accumulated for the current frame.
- `frameStatus`: expected frame type (poll or listen), start and end, and the `guardEnd` and `waitingEnd` windows.
- `protocolStatus`: negotiated parameters (FSD, FSC, FWT, SFGT), updated by RATS/ATS.
- `chainedFlags`: flags propagated to the following frames (e.g. `Encrypted` after MIFARE AUTH).

`decoder->bitrate` and `decoder->modulation` point to the detected bit rate. When `decoder->modulation == nullptr`, the orchestrator goes back to detection mode.

## 2. Common preprocessing (`NfcDecoderStatus::nextSample`)

For each input sample:

1. Increments `signalClock`, the master clock in samples (`unsigned int`).
2. Updates the envelope (`signalEnvelope`) with an exponential average (constant 5·10⁵/fs), but only when the sample differs less than 5% from the envelope, or after 10 ETU without an update. This keeps modulation pauses from dragging the envelope down.
3. Applies an IIR DC-removal filter: `y = x + 0.9·y₁; filtered = y − y₁`.
4. Computes the mean deviation of the filtered signal (`meanDeviation`) and the slow average (`signalAverage`, used for carrier detection).
5. Stores four values in `sample[clock & 1023]`:
   - `samplingValue`: raw magnitude.
   - `filteredValue`: magnitude without DC.
   - `meanDeviation`: noise level.
   - `modulateDepth = (env − clamp(x, 0, env)) / env`: modulation depth, from 0 to 1.

All demodulators read this buffer through fixed offsets (`offsetSignalIndex`, `offsetDelayNIndex`, `offsetFutureIndex`) computed in `initialize()`.

## 3. Per bit rate parameters (`initialize`)

With `u = fs / fc` (samples per carrier cycle, `sampleTimeUnit`) and `rate` = 0, 1, 2 (106, 212, 424 kbps):

| Field | Value | Meaning |
|---|---|---|
| `period1SymbolSamples` | `u·(128 >> rate)` | 1 symbol (1 ETU) |
| `period2SymbolSamples` | `u·(64 >> rate)` | ½ symbol |
| `period4SymbolSamples` | `u·(32 >> rate)` | ¼ symbol |
| `period8SymbolSamples` | `u·(16 >> rate)` | ⅛ symbol |
| `period0SymbolSamples` | `u·(256 >> rate)` | 2 symbols |
| `symbolDelayDetect` | sum of `period1` of the lower bit rates | extra delay for each bit rate |

The `symbolDelayDetect` delay makes 212 and 424 kbps work on older samples. On each sample 106 kbps is evaluated first, and a higher bit rate only wins if the lower one has not already validated the pulse. All published times are corrected by subtracting this delay.

The whole delay line (`symbolDelayDetect + period1`) must fit in `BUFFER_SIZE = 1024` samples, which limits the sample rate to about 60 MS/s; `initialize()` logs a warning when it does not fit.

Default protocol timings, in 1/fc units (`Nfc.h`):

| Constant | Value | Use |
|---|---|---|
| `NFCA_FGT_DEF` | 1024 | Minimum guard before searching for the response (TR0) |
| `NFCA_FWT_ATQA` | 128·18 | Maximum wait for the response to REQA/WUPA/SELn |
| `NFCA_FWT_DEF` | 256·16·2⁴ | Default FWT (FWI = 4) |
| `NFCA_SFGT_DEF` | 256·16 | Default SFGT |
| `NFC_FWT_ACTIVATION` | 71680 | Maximum wait for ATS after RATS |

## 4. Reader frame detection (`detectModulation`)

Runs on every sample while there is no active modulation, and only when the envelope is above `powerLevelThreshold`. For each bit rate:

1. **Half symbol integrator**: `filterIntegrate` is the moving sum of `samplingValue` over `period2` samples.
2. **Correlation buffer**: `correlationData[t mod P1] = filterIntegrate`, a circular buffer one symbol long.
3. **Correlation**:
   - `S0 = I(t) − I(t − P/2)`: second half of the last symbol minus the first half.
   - `S1 = I(t − P/2) − I(t − 1)`: the same measure with inverted sign, `S1 ≈ −S0`.
   - `SD = (S0 − S1) / P2 ≈ 2·S0 / P2`, signed.

   This is a correlation with a square template (+1 over one half symbol, −1 over the other). Taking `SD = S0 − S1` doubles the correlation amplitude, and the thresholds (0.75·envelope, peak/2) are calibrated for it. The comparison `S0 > S1` used to classify symbols is equivalent to `S0 > 0`.

   An ASK pause entering the window produces a negative minimum of `SD`, and a positive maximum when it leaves.
4. **Phase 1, falling edge**: when `SD < −envelope·correlationThreshold` (0.75 by default), it records the minimum (`correlatedPeakTime`) and the maximum depth (`modulateDepth`). The search ends ¼ symbol after the last minimum. The pulse is discarded if its depth is below `minimumModulationDeep` (0.90) or above `maximumModulationDeep` (1.0).
5. **Phase 2, rising edge**: opens a ±⅛ symbol window around `peak + P/2` and searches for the positive maximum of `SD`.
6. **Validation**: the width `end − start` must be within `P1 ± P1/4`.
7. A valid pulse is taken as the SoF (pattern Z). Then it:
   - sets `decoder->bitrate` and `decoder->modulation`;
   - sets `frameStatus.frameType = NfcPollFrame`;
   - prepares the window for the next symbol (`searchSyncTime = end + P1`, ±P8);
   - sets the symbol threshold to `searchValueThreshold = peak/2`.

If the search for a symbol does not complete (`clock > peak + P1`), the state is reset.

## 5. Reader frame: 100% ASK + modified Miller

### 5.1 Symbols (`decodePollFrameSymbolAsk`)

Uses the same correlator as detection, but with `SD = |S0 − S1| / P2`. In each window `[sync − P8, sync + P8]`:

- Tracks the maximum of `SD` above the threshold, to resynchronize the symbol end.
- At `sync`, captures `SD`, `S0` and `S1`.
- When the window closes, it classifies the symbol:
  - `SD(sync) < threshold` → **pattern Y**, no pause. Timing is extrapolated from `sync`, without resynchronizing.
  - `S0 > S1` → **pattern Z**, pause at the start of the bit. Resynchronizes on the peak.
  - otherwise → **pattern X**, pause in the middle of the bit. Resynchronizes on the peak.
  - If there is modulation at `sync` but no peak was captured, `sync` is used as the peak so the next window is never placed in the past.
- The next window opens at `end + P1`.

### 5.2 Bits and bytes (`decodePollFrame`)

- Each bit is decoded one symbol late (`streamStatus.previous`). This way, when the EoF is detected, the "0" that precedes it is not counted as data. X is 1; Y and Z are 0.
- **EoF**: a Y pattern preceded by Y or Z. This is the "logic 0 + Y" of ISO 14443-2.
- Each byte has 8 data bits (LSB first) plus 1 odd parity bit (`checkParity`). A parity error sets `ParityError` on the frame.
- When the frame closes:
  - if 7 or more leftover bits remain, they are added as the last byte;
  - a frame of exactly 7 bits is flagged `ShortFrame` (REQA/WUPA);
  - when `maxPollFrameSize` is reached, the frame is flagged `Truncated`.
- Then `process()` is called, the frame is added to the list, and the demodulator state is cleared, keeping `decoder->bitrate` to search for the response.

## 6. Card frame (listen)

`process()` sets two instants from the end of the reader frame:

- `guardEnd = frameEnd + FGT`: no response is searched before this instant.
- `waitingEnd = frameEnd + FWT`: if there is no response by then, it is a timeout.

The card answers at the same bit rate as the reader.

### 6.1 106 kbps: subcarrier OOK (fc/16) + Manchester

**SoF, pattern D (`decodeListenFrameStartAsk`)**

- Signal: `filteredValue² · 10`, the subcarrier energy, integrated over ½ symbol.
- Correlation `S0 = I(t) − I(t − P/2)`.
- From `guardEnd`, the threshold is calibrated from the noise: `meanDeviation · P8`. It is calibrated again after a frame search reset.
- When `waitingEnd` is passed, or reader modulation appears again (`modulateDepth > 0.9`), it returns `NoPattern` (timeout).
- A valid pulse is a positive maximum followed by a negative minimum, with width `P1 ± P8`. Once validated, the threshold becomes 25% of the peak.

**Symbols (`decodeListenFrameSymbolAsk`)**

- Same correlator, with `SD = |S0 − S1|`.
- `SD(sync) > threshold`: if `S0 > S1` it is **pattern E** (0, subcarrier in the second half); otherwise **pattern D** (1, subcarrier in the first half). The threshold adapts to 25% of the last peak.
- No modulation → **pattern F** (EoF).

**Bytes (`decodeListenFrame`)**

- 8 data bits plus odd parity.
- 4-bit frames (MIFARE ACK/NAK) are accepted.
- A SoF followed by too few bits is treated as a spurious pulse: partial data is discarded and the SoF search restarts.

### 6.2 212/424 kbps: subcarrier BPSK + NRZ-L

**SoF, pattern S (`decodeListenFrameStartBpsk`)**

- Differential demodulation: `filtered(t) · filtered(t − P1) · 10`, integrated over ¼ symbol (`phaseIntegrate`).
- The value is positive when the current symbol has the same phase as the previous one, and negative when the phase is inverted.
- Searches for the subcarrier preamble without phase changes (32 subcarrier cycles, between 3 and 4 ETU at 106 kbps). It ends at the first phase inversion.

**Symbols (`decodeListenFrameSymbolBpsk`)**

- A zero-crossing detector, active only once per symbol, resynchronizes `searchSyncTime`.
- At `sync`:
  - if `|phase| < threshold`, it is the EoF (**pattern O**);
  - if `phase < −threshold`, there is a phase inversion: the bit value and the M/N pattern toggle;
  - otherwise the value is kept and the threshold becomes 25% of `phase`.
- Bytes: 8 data bits plus 1 parity bit. The last byte is checked with even parity, inverted with respect to the others.

## 7. Protocol layer (`process`)

Before classification, a reader frame restores the default timings from `protocolStatus`. Then the handlers are tried in this order:

| Function | Recognizes (poll) | Effect |
|---|---|---|
| `processREQA` | `0x26`/`0x52`, 7-bit short frame | Resets the protocol parameters and `chainedFlags`. FWT becomes `NFCA_FWT_ATQA` |
| `processHLTA` | `50 00` + CRC, 4 bytes | Checks CRC, resets the parameters and the modulation (the card does not answer) |
| `processSELn` | `0x93`/`0x95`/`0x97` | Same timings as REQA. CRC is checked on SELECT (NVB = 0x70) and on its SAK, not on anticollision frames |
| `processRATS` | `0xE0`, 4 bytes | FSD from FSDI, FWT = activation. The ATS response gives FSC (FSCI in T0) and TB(1) → SFGI, FWI, parsed within `min(TL, length − 2)` |
| `processPPSr` | `0xDx` | Checks CRC |
| `processAUTH` | `0x60`/`0x61` | After the response sets `Encrypted` in `chainedFlags`. From then on every frame is an application frame without valid parity |
| `processIBlock` | PCB `000x x01x`, 3 bytes or more | Checks CRC |
| `processRBlock` | PCB `101x x01x`, 3–4 bytes | Checks CRC |
| `processSBlock` | PCB `11xx x010`, 3–5 bytes | Checks CRC. An S(WTX) response from the reader extends the FWT of the next response to `FWT × WTXM` |
| `processOther` | anything else | Application phase, checks CRC, forgets the last command |

FSDI/FSCI values 'D' to 'F' (RFU) are interpreted as 'C' (4096 bytes), and frame sizes are limited to the stream buffer capacity (4096 + 2 bytes).

Responses are classified according to `frameStatus.lastCommand`, which is cleared after each response and on response timeout.

At the end, `process()` computes `guardEnd`/`waitingEnd` for the response (poll frames) or `guardEnd` for the next command (listen frames). It also sets `frameType` back to detection mode.

---

## 8. Errors found

They are sorted by severity. Line numbers refer to `tech/NfcA.cpp` before the fixes.

### Fix status

| Error | Status | Applied fix |
|---|---|---|
| E1 Buffer overflow | **Fixed** | `NfcStreamStatus::buffer` grows to 4096 + 2 bytes (this also protects NFC-B, which had the same problem). Frame size is limited to the buffer capacity. `buildFrame` allocates the `RawFrame` with the actual size: the default constructor allocates 512 bytes and `put()` aborted on an `assert` with larger frames |
| E2 RFU FSDI | **Fixed** | `frameSizeFromIndex()` interprets FSDI/FSCI 'D'–'F' as 'C' (ISO/IEC 14443-4) |
| E3 S-Blocks | **Fixed** | `NFCA_SBLOCK = 0xC2`, length 3 to 5 bytes (with and without CID and WTXM) |
| E4 `resetFrameSearch` | **Fixed** | Clears `streamStatus` and forces the threshold to be recalibrated from the noise level |
| E5 Out of range reads | **Fixed** | RATS requires 4 bytes. The ATS is parsed within `min(TL, limit − 2)`; when TB is missing the defaults are used |
| E6 Window lock | **Fixed** | If there is modulation at the sync point but no peak was captured, `searchSyncTime` is used as the peak. Threshold calibration no longer depends on an exact `clock == guardEnd` |
| E7 `S1 ≈ −S0` | **Not an error** | Intended: square template correlation, `S1` is the inverse of `S0` (confirmed by the author). Documented in the code and in section 4 |
| E8 32-bit clock | Pending | Affects the whole decoder (NFC-A/B/F/V and `RawFrame`); requires a cross-cutting change |
| E9 `maximumModulationDeep` | **Fixed** | Detection discards pulses deeper than the maximum. With the default value (1.0) nothing changes |
| E10 RawFrame::Nil | **Fixed** | `return false` |
| E10 HLTA | **Fixed** | Removed the useless `CrcError` check; requires `50 00` and 4 bytes |
| E10 REQA/WUPA | **Fixed** | Requires `ShortFrame` (7 bits) |
| E10 I-Block / R-Block | **Fixed** | I-block from 3 bytes; R-block 3 or 4 bytes (with CID) |
| E10 SELECT and SAK CRC | **Fixed** | CRC is checked on SELECT (NVB = 0x70) and its SAK; not on anticollision frames |
| E10 FSC for reader frames | **Fixed** | Reader frames are limited by the FSC from the ATS (FSCI in T0). Until the ATS is received the FSD from RATS is used, so valid frames are not truncated |
| E10 S(WTX) | **Fixed** | The reader S(WTX) response extends the FWT of the next response to `FWT × WTXM` (WTXM 1–59) |
| E10 `lastCommand` after timeout | **Fixed** | Cleared in `resetModulation()` and when an unknown command is received |
| E10 Leftover bits in anticollision | Pending | `RawFrame` has no field for the number of valid bits in the last byte |
| E10 Sample rate limit | **Warned** | `initialize()` logs a warning when the delay line does not fit in `BUFFER_SIZE` |
| E10 Unused fields | Pending | `startUpGuardTime`, `requestGuardTime`, `lastFrameEnd`, `detectIntegrate`: kept in case they are used in the future |

Related issue outside NFC-A: NFC-B maps RFU FSDI values to a frame size of 0, like E2. It was not changed.

### E1. `streamStatus.buffer` overflow with FSD ≥ 512. **Severe**

`streamStatus.buffer` was 512 bytes (`NfcTech.h`), but `processRATS` (l. 1716) set `maxFrameSize = NFC_FDS_TABLE[fsdi]`, which can be up to 4096 (FSDI 9–C).

- With FSD ≥ 1024, the byte loop wrote past the array: `bytes < maxFrameSize` allowed it.
- With FSD = 512, the truncation path wrote to `buffer[512]`. This happened for `bits >= 7` (l. 455), `bits == 4` (l. 615) and `bits == 9` (l. 728), because `buffer[bytes++]` is written right when `bytes == maxFrameSize`.

The write corrupted `frameStatus`, the next member in `Impl`. A single RATS with FSDI ≥ 9, real or corrupted by noise, was enough. With the synthetic test, the original decoder hangs.

**Fix**: limit `maxFrameSize` to `sizeof(streamStatus.buffer) − 1`, or grow the buffer to 4096 plus a margin.

### E2. RFU FSDI (D–F) locks the decoder until the next REQA. **Severe**

`NFC_FDS_TABLE[13..15] = 0`. With `maxFrameSize = 0`, every frame was truncated on its first symbol, or aborted with `resetModulation()`, until a REQA, WUPA or HLTA arrived. ISO 14443-4 and NFC Forum require an RFU FSDI to be treated as the maximum (or 256).

**Fix**: map FSDI > 0xC to 256 or 4096.

### E3. S-Blocks are never recognized. **Medium**

`NFCA_SBLOCK` was `0xC0`, but an S-block PCB satisfies `(pcb & 0xC7) == 0xC2`, because bit b2 is always 1. It also required `limit() == 4`, but S(DESELECT) without CID is 3 bytes long.

Result: DESELECT (`C2`) and WTX (`F2`) ended up in `processOther`. The application parser (`ParserNfc.cpp:245`) does use 0xC2 with 3–4 bytes.

**Fix**: `NFCA_SBLOCK = 0xC2` and `limit() >= 3 && limit() <= 4` (or 5 with CID).

### E4. `resetFrameSearch()` does not clear the bit state or the threshold. **Medium**

If a spurious pulse was taken as SoF and a few bits were decoded before the EoF (l. 653), only the modulation times and `frameStart` were reset. These were left uncleared:

- `streamStatus.bits`, `data` and `flags`: the next real response started shifted and with wrong parity;
- `searchValueThreshold`: it stayed at 25% of the spurious peak. Since it was only recalibrated when `clock == guardEnd`, and that instant had already passed, the new SoF was searched with an arbitrary threshold.

**Fix**: `streamStatus = {}` in `resetFrameSearch()`, and recompute the threshold from `meanDeviation`.

### E5. Out of range reads in RATS/ATS. **Medium**

- `processRATS` (poll) read `frame[1]` without checking `limit() >= 2`.
- In the ATS, `T0`, `TA` and `TB` were read without comparing against `TL` or `limit()`. With a short ATS or a wrong TL, CRC bytes (or memory outside the frame, because `Buffer::operator[]` does not check bounds) were used as TB. That configured FWT/SFGT with arbitrary values, and FWI could result in an FWT of several seconds.

**Fix**: check `offset < tl && offset < limit() − 2` before each read.

### E6. Lock when the search window falls in the past. **Low (robustness)**

Searches end with `signalClock == searchEndTime` or `== searchSyncTime` (l. 321, 873, 1029, 1162, 1304, BPSK). If the window end is ever computed in the past, the loop consumes samples forever and the decoder stays on that frame until the end of the stream. Examples:

- `correlatedPeakTime == 0` when resynchronizing in `decodePollFrameSymbolAsk`. This is possible when `SD(sync) == threshold`, because the peak requires `>` and the classification requires `<`.
- `signalClock` wrap-around (E8).

The same applied to the `clock == guardEnd` calibration (l. 987, 1253): if `guardEnd` had already passed, the threshold was not recalibrated.

**Fix**: use `>=` to close the windows, and abort the frame if `searchEndTime < signalClock`.

### E7. `S1` is the inverse of `S0`. **Not an error (by design)**

`filterPoint3 = (t + P1 − 1) % P1` points to the slot written on the previous sample (`t − 1`), not to the oldest slot in the buffer (`t + 1 ≡ t − P1 + 1`). Therefore:

```
S1 = I(t−P/2) − I(t−1) ≈ −S0
SD = S0 − S1 = 2·S0 − (s[t] − s[t−P/2])
```

The review flagged this because `S1` is not an independent measurement and the comparison `S0 > S1` is equivalent to `S0 > 0`. The author confirmed it is intended: the correlation function is a square template, so `S1` is the inverse of `S0`, and the thresholds (0.75·envelope, peak/2) are calibrated for `SD ≈ 2·S0`. The code now explains this next to each correlator.

### E8. 32-bit clock. **Low (affects the whole decoder)**

`signalClock`, `frameStart`/`frameEnd` and all `search*Time` fields are `unsigned int`. At 10 MS/s they wrap every ~7.1 min, and at 40 MS/s every ~1.8 min. In long live captures:

- frame timestamps (`timeStart = frameStart / fs`) go back to 0;
- the `<`/`>` window comparisons fail around the wrap-around.

**Fix**: use `uint64_t` for the clock and the times, or compare with signed differences.

### E9. `maximumModulationDeep` is not used in NFC-A. **Low**

It is configured from the API and the UI (`setModulationThresholdNfcA(min, max)`) and appears in the log, but no NFC-A code path read it. NFC-B does use it. The NFC-A "max" parameter had no effect.

### E10. Other minor details

- `decodeListenFrame` returned `RawFrame::Nil` from a `bool` function (l. 593). It worked through `operator bool()` (returns `false`), but it should be `return false;`.
- `processHLTA` checked `!hasFrameFlags(CrcError)` before computing the CRC, so the condition was always true. It did not check that the second byte is `0x00` either.
- `processREQA` accepted `0x26`/`0x52` as an 8-bit frame. It should require `ShortFrame` (7 bits).
- `processIBlock` required `limit() > 4`, so it did not recognize I-blocks with 0–1 byte of INF (e.g. chaining). `processRBlock`, with `limit() == 3`, did not recognize R-blocks with CID.
- `processSELn` did not check CRC on SELECT (NVB = 0x70) or on SAK.
- Bit-oriented anticollision frames (NVB ≠ 0x70) can have 1 to 6 bits in the last byte, and those bits are discarded without any flag. The same happens with responses that are neither 4 nor 8·n bits.
- `maxFrameSize` (the FSD, maximum frame the reader receives) was also applied to reader frames, when the correct limit for those is the FSC (FSCI in T0 of the ATS).
- S(WTX) was not handled. A response after WTX that exceeded the negotiated FWT was lost as a timeout.
- When a command had no response (timeout), `lastCommand` was not reset. If an unrecognized frame came next (`processOther`, which did not modify `lastCommand`), its response would be interpreted according to the previous command. For example, it would be parsed as an ATS and change FWT/SFGT.
- Some fields are computed but never used: `startUpGuardTime`, `requestGuardTime`, `lastFrameEnd` and `detectIntegrate`.
- The delay line length (`symbolDelayDetect` + P1 at 424 kbps ≈ 224/fc) must fit in `BUFFER_SIZE = 1024`, which limits fs to about 60 MS/s. This was not checked.

## 9. Optimizations

### 9.1 Applied

All of them keep the decoder output exactly the same: on the 19 captures in `wav/` the generated frames (data, flags, phase and start and end samples) are byte-identical before and after, both with all technologies enabled and with NFC-A only.

1. **One division per sample instead of three.** `correlationPoints()` computes `filterPoint1 = signalIndex % P1` and derives the other two points with a comparison and a subtraction (`point2 = p1 + P2`, subtracting P1 on overflow; `point3 = p1 − 1`, wrapping to `P1 − 1`). It is used in detection and in the ASK demodulators.
2. **Precomputed inverse.** `period2Inverse[rate] = 1 / P2` is computed in `initialize()`, and the `SD` normalization becomes a multiplication.
3. **Local copies in `detectModulation`.** `signalClock` and the debug pointer are read once per call. Before, the compiler had to reload them from memory on every use, because writes to `NfcModulationStatus` could alias them.
4. **Parity without a loop.** `checkParity` uses an 8-bit XOR fold, portable to MSVC.
5. **Duplicated code removed.**
   - `resetSearch(modulation)` replaces the 5 search reset blocks, including `resetFrameSearch`. Some blocks did not clear `searchPulseWidth` or `symbolRiseTime`; both are always written before being read, so clearing them does not change the result.
   - `buildFrame(type, rate, flags)` replaces the 3 blocks that built a `RawFrame`.

**Measured performance** (best of 40 repetitions, 8 interleaved rounds, on a shared and fairly noisy machine):

| Scenario | Average improvement |
|---|---|
| NFC-A only, `test_NFC-A_106kbps_424kbps_001` | ≈ +14% |
| NFC-A only, `test_NFC-F_212kbps_003` (NFC-A detector without A frames) | ≈ +16% |
| All four technologies enabled | ≈ +5–10% (B/F/V and `nextSample` are unchanged) |

The bug fixes in section 8 did not change these figures.

### 9.2 Discarded or pending

- **Incremental circular indexes** (one counter per bit rate instead of `%`): tried, and it turned out **slower** (~5%). Keeping and validating the counter state costs more than an integer division on a current CPU.
- **Simplify the correlator to `SD = 2·S0`** (E7): the behavior is intended, but this simplification would drop the `s[t] − s[t−P/2]` term, changing `SD` slightly and therefore the results, for one memory read less per sample. Not applied.
- **Early exit in detection**: the no-signal path already reduces to a few comparisons, so the expected gain is small.
- **Cheaper resets**: `resetModulation()` clears 3 × 8 KB and `decodePollFrame` does an 8 KB `memset`. `integrationData` is indexed over the whole buffer and old values are read after the reset, so clearing less would change the output. It runs once per frame, so it is not worth it.
- **Compile-time debug** (template or `constexpr`): after the local copy the check is a single, highly predictable branch.
- Outside the NFC-A scope: `NfcDecoderStatus::nextSample()` takes ~30% of the time (with a division by `signalEnvelope` on every sample) and is shared by all technologies.

## 10. Tests

### 10.1 Running the tests

The decoder regression tests live in `src/nfc-test/test-decoder` and are registered with CTest:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target test-decoder
ctest --test-dir build -R decoder
```

`test-decoder` can also be run directly on one or more captures:

```
build/src/nfc-test/test-decoder/test-decoder [-v] wav/test_NFC-A_106kbps_001.wav ...
```

For each `<capture>.wav` it decodes the signal with `lab::NfcDecoder` and compares the decoded frames with `<capture>.json`, which uses the trace storage format. Only the fields present in each expected frame are compared (`techType`, `frameType`, `framePhase`, `frameFlags`, `frameRate`, `sampleStart`, `sampleEnd`, `frameData`), and carrier frames are ignored. The exit code is 0 when every capture matches.

### 10.2 Reference captures

`wav/` holds reference captures (`test_*.wav`) with their expected output (`*.json`), one CTest test per capture (`decoder-test_*`). Any demodulator change should be validated against them. With the current code all 19 captures reproduce their `.json` exactly, and the output is byte-identical to the code before the optimizations and fixes.

### 10.3 Synthetic NFC-A captures

None of the reference captures contains the cases of errors E1–E6, so `scripts/nfca_synthetic.py` generates 106 kbps NFC-A captures with their expected frames. The reader uses 100% ASK with modified Miller coding, the card uses subcarrier load modulation with Manchester coding, and deterministic gaussian noise is added. The expected frames check content, type, phase and flags, not exact sample positions.

CTest runs the generator into the build directory (`decoder-synthetic-generate`) and then one test per scenario (`decoder-nfca-synthetic-*`). They need Python 3; without it they are skipped.

| Scenario | Before the fixes | After the fixes |
|---|---|---|
| `basic`: full activation (REQA, ATQA, RATS, ATS, I-block) | Pass | Pass |
| `fsdi_4096`: RATS with FSDI = C and a 703-byte reader frame (E1) | **Fails**: hangs (memory corruption), stopped by the test timeout | 703-byte frame complete and without errors |
| `fsdi_rfu`: RATS with FSDI = F (E2) | **Fails**: all following frames are lost | All frames decoded |
| `wtx`: S(WTX) with WTXM = 4 and response at 12 ms, over the 4.8 ms FWT | **Fails**: response lost | Response decoded |
| `spurious`: 2-bit spurious burst before ATQA (E4) | **Fails**: ATQA `13:00` with parity error | ATQA `04:00` correct |
| `ats_short`: truncated ATS, TL = 5 but only TL and T0 (E5) | Pass (CRC bytes were used as TB; only internal timing changes) | Pass, default timings used |
| `sblock`: 3-byte S(DESELECT) (E3) | Pass (classified as a generic frame with the same phase and flags) | Pass, classified as S-block |

The last two scenarios do not change the decoded frames, so they act as non-regression tests rather than detecting the original errors.
