# NFC-V decoder (ISO/IEC 15693)

This document describes how the NFC-V decoder in `lab-radio` works, and records the errors and optimizations found while reviewing it. It follows the structure of `doc/nfca.md`, where the common signal preprocessing is described (section 2).

Files:

| File | Contents |
|---|---|
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcV.cpp` | Demodulator, frame decoder and protocol logic (`NfcV::Impl`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcTech.h` | Shared structures, correlation helper and pulse slot parameters |
| `src/nfc-lib/lib-lab/lab-radio/src/main/include/lab/nfc/Nfc.h` | Timing constants (`NFCV_*`) |
| `src/nfc-test/test-decoder/scripts/nfcv_synthetic.py` | Synthetic NFC-V capture generator (section 8) |

---

## 1. Overview

| | Reader (VCD) | Card (VICC) |
|---|---|---|
| Modulation | 100% ASK pauses of 128/fc (9.44 µs) | Load modulation with a single fc/32 subcarrier (423.75 kHz) |
| Coding | Pulse position: 1 out of 4 (2 bits per symbol, 26.48 kbps) or 1 out of 256 (8 bits per symbol, 1.65 kbps) | Manchester, high data rate (26.48 kbps) |
| SOF | Pauses at 0 and 5 (1/4) or 7 (1/256) units of 128/fc | 768/fc unmodulated, 24 pulses (768/fc), logic 1 |
| EOF | Pause at 2 units | Logic 0, 24 pulses, 768/fc unmodulated |

Bits are LSB first in both directions, and frames end with a CRC_B (ISO/IEC 13239).

There is a single bit rate configuration. `symbolDelayDetect` is 512/fc, the longest of all technologies; the decoder reads `sample[]` up to 640/fc in the past, so the sample rate must stay below about 21 MS/s (`initialize()` warns otherwise). The integration ring buffers keep the right relative delay even beyond that, so 20 MS/s works (verified with a synthetic capture).

## 2. Reader frame detection (`detectModulation`)

- Half symbol integrator of `samplingValue` and a one symbol correlation ring. `S0 = (I(older half) − I(newer half)) / P2` is positive when a pause enters the window.
- A pulse needs `S0 > envelope · correlationThreshold` (0.5), a modulation depth of at least `minimumModulationDeep` (0.9, which means 100% ASK only) and the signal back high at the end of the search.
- The first pulse starts the SOF. The second pulse ends at 3 symbols (1 out of 4) or 4 symbols (1 out of 256) ± ⅛ symbol, which selects `decoder->pulse`.

## 3. Reader frame (`decodePollFrameSymbolPpm`, `decodePollFrame`)

- Each symbol window is the pulse length (4 or 256 slots of 256/fc). The correlation peak gives the pause position; a slot matches when the peak is at its end ± ¼ symbol, and its index is the symbol value.
- A peak in the first part of the second slot is the **EOF**. Any other position is a stream error (**pattern E**).
- Values are accumulated 2 or 8 bits at a time, LSB first; a byte is stored when the next symbol arrives or at the end of the frame.

## 4. Card frame

### 4.1 SOF (`decodeListenFrameStartAsk`)

- Energy detector: `filteredValue² · 10` integrated over one symbol (256/fc), and a ring of two symbols. `S0` is the difference between the two consecutive symbol halves.
- From `guardEnd` the threshold is the signal deviation at that point. After `waitingEnd` it returns `NoPattern`.
- Two stages: the 24 pulse burst (768/fc ± 32) and the logic 1 burst (256/fc ± 32). The threshold for symbol decoding is 25% of the last peak.

### 4.2 Symbols and frame (`decodeListenFrameSymbolAsk`, `decodeListenFrame`)

- At each symbol (512/fc) the largest `|S0|` in `[sync − ¼, sync + ¼]` gives the bit: subcarrier in the first half is 0, in the second half is 1.
- No peak above the threshold is the EOF (**pattern S**). The logic 0 that starts the EOF is decoded as an extra bit and discarded because it never completes a byte.

## 5. Protocol layer (`process`)

There is no protocol processing: every frame is in the application phase and only the CRC is checked. Default timings: FGT 1024/fc, FWT `256·16·2⁴`/fc (≈4.8 ms).

---

## 6. Errors found

Line numbers refer to `tech/NfcV.cpp` before the fixes.

### Fix status

| Error | Status | Applied fix |
|---|---|---|
| V1 Responses over 256 bytes | **Fixed** | Frame limit is the stream buffer capacity (4097 bytes) and `buildFrame` allocates the `RawFrame` with the actual size |
| V2 Threshold at exact `guardEnd` | **Fixed** | Calibrated on the first sample after the guard time (output unchanged) |
| V3 Sample rate limit | **Warned** | `initialize()` warns when the delayed sample reads exceed `BUFFER_SIZE` |
| V4 Guard time sign | Not changed | Inconsistent with the other technologies, but real captures depend on it (see below) |
| V5 Reader frame during the response wait | Pending | Design trade-off (question for the author) |
| V6 No protocol phases | Pending | Inventory and other anticollision commands are in the application phase (question for the author) |
| V7 Low data rate and dual subcarrier | Pending | Responses requested with these flags are not decoded |
| V8 10% ASK readers | Pending | Only 100% ASK is detected with the default `minimumModulationDeep` |
| V9 Long response times | Pending | Write commands can answer up to 20 ms later; the 4.8 ms FWT loses them |
| V10 32-bit clock and unused fields | Pending | As in NFC-A; `maximumModulationDeep` (1.0) never triggers |

### V1. Responses over 256 bytes are truncated. **Medium**

ISO/IEC 15693 has no frame size negotiation, and a Read Multiple Blocks response can carry many blocks (128 blocks of 4 bytes is 515 bytes with flags and CRC). The limit was 256 bytes, so those responses were cut and flagged `Truncated`. Raising the limit alone would have hit the 512-byte `RawFrame` capacity `assert` (same as NFC-A E1 and NFC-B B1).

### V2 and V3. Robustness. **Low**

- The listen threshold was calibrated only when `clock == guardEnd` exactly.
- The sample rate limit (about 21 MS/s) was not checked. A first estimate based on `symbolDelayDetect + period1` predicted a failure at 20 MS/s, but a synthetic capture at 20 MS/s decodes correctly: the longest delay only indexes the integration ring buffers, which keep the right relative delay. The warning uses the real limit (`symbolDelayDetect + period2` samples read from `sample[]`).

### V4. Guard time sign. **Not an error in practice**

`process()` computes `guardEnd` and `waitingEnd` subtracting `symbolDelayDetect`, while NFC-A, B and F add it. Taken at face value the guard ends right at the end of the reader EOF instead of 1024/fc later. Changing the sign broke the reference capture `test_NFC-V_26kbps_002`: the listen threshold is calibrated from the signal deviation at `guardEnd`, and at the end of the reader pause it is still high, which gives a threshold that works; 75 µs later it is at the noise level and the SOF detector locks on the wrong part of the burst. The sign was kept and the dependency is documented in the code.

### V5 to V10. Pending

- **Reader frame during the wait**: the wait for a response is only interrupted by modulation deeper than `maximumModulationDeep` (1.0), which never happens. A request sent before `waitingEnd` is lost: a synthetic Inventory sent 0.8 ms after an unanswered one is not decoded. This looks deliberate: in a 16-slot Inventory the reader sends an EOF before each slot and the responses come after those EOFs, so stopping the wait on any reader modulation would lose them. Telling a slot EOF from a new request SOF would need the detector to look back, so it was left unchanged.
- **Protocol phases**: Inventory, Stay Quiet, Select and Reset to Ready are anticollision or selection commands, but every NFC-V frame is in the application phase. Classifying them would change the reference captures.
- **Low data rate and dual subcarrier**: the request flags can ask the card to answer at 6.62 kbps or with two subcarriers; the decoder only supports the high data rate with one subcarrier.
- **10% ASK**: ISO/IEC 15693 also allows 10% reader modulation; it is detected only by lowering `minimumModulationDeep`.
- **Long response times**: write-alike commands can answer up to 20 ms after the request, or after an extra EOF when the option flag is set.

## 7. Optimizations

All of them keep the output byte-identical on the 19 reference captures and on the synthetic captures.

1. **Shared correlation helper** `NfcTech::correlationPoints()` in detection, reader symbols and card SOF/symbols (one integer division instead of two).
2. **Precomputed inverse** of `period2SymbolSamples` and **local copies** of the signal clock and debug pointer in `detectModulation`.
3. **Duplicated code removed**: `resetSearch()`, `resetListenSearch()` and `buildFrame()`.

**Measured performance**: no measurable change (within ±2%). As with NFC-F, the common preprocessing (`nextSample`) dominates when the detector is this light.

## 8. Tests

The decoder tests are described in `doc/nfca.md`, section 10. NFC-V adds `scripts/nfcv_synthetic.py`, which uses the helpers in `scripts/synthetic_common.py`. It generates reader frames with 100% ASK pulse position coding and card frames with a single fc/32 subcarrier, SOF and EOF. CTest runs it into the build directory (`decoder-nfcv-synthetic-generate`) and then one test per scenario.

| Scenario | Before the fixes | After the fixes |
|---|---|---|
| `basic`: Inventory and Read Single Block, 1 out of 4 | Pass | Pass |
| `code256`: same with 1 out of 256 | Pass | Pass |
| `large_response`: Read Multiple Blocks with a 515-byte response (V1) | **Fails**: truncated at 256 bytes | Pass |
| `rate_20m`: Inventory and Read Single Block at 20 MS/s (V3) | Pass | Pass |
