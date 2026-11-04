# Changelog

All notable changes to NFC Laboratory are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [3.5.0] - 2026-11-04

### Added

- HackRF One receiver support, with its own configuration page (#111, thanks @gentilkiwi).
- Sipeed SLogic Combo 8 logic analyzer support (driver, firmware and device detection).
- SigMF as a lossless real-time radio capture format; SigMF files can be opened from the Open file dialog (#115, #117).
- HydraSDR RF port selection (`rfPort`), including CABLE1/CABLE2 inputs (#120, thanks @OmegaRelay), and a HydraSDR default config for `nfc-rx`.
- New settings dialog: pages are now defined in XML, with dedicated pages for DSLogic/Sipeed analyzers (#119).
- Clock channel is shown as a frequency band instead of a waveform; clock frequency is measured between edges.
- Device disconnection detection for HackRF, RTL-SDR, MSI2500 and Sipeed devices.
- Optional gRPC remote control API (`ENABLE_GRPC_API`, off by default).
- `nfc-rx`: AirSpy and Spyverter options (#109, thanks @lcdunstan).
- Logger wildcards.
- `test-sync` decoder time base test.

### Changed

- Compact `.trz` trace storage: varint/zigzag delta encoding (APCM v3) plus zstd compression (#114).
- Windows build migrated to MSYS2 UCRT64.
- Hardware drivers reorganized into `hw::radio` and `hw::logic` namespaces.
- Updated bundled libraries (nlohmann json and others).
- Flatpak metainfo updated (#102, thanks @Steffen-W).

### Fixed

- Airspy/DSLogic signal desync, by aligning device offsets to a shared time epoch (#113).
- Capture offset bias calculation in logic and radio device tasks.
- Protocol markers drawn ahead of the signal after epoch alignment.
- Logic sample offsets are now stored relative to the saved range.
- Sipeed capture crashing on stop and refusing to restart.
- USB transfer list access from the libusb event thread; USB transfers are drained on stop to avoid a Windows fault.
- ISO-7816 decoder hardened against unexpected channel layouts; logic probe order aligned with the decoder.
- Logic device config no longer applies settings meant for another analyzer.
- Saving settings only applies the page of the connected receiver.
- `PARAM_TEST_MODE` no longer reuses the id of another parameter; `PARAM_RF_PORT` is answered by every radio device.
- Build fixes for macOS (#98, #100, thanks @bwachter), including `rt::Buffer` portability, zstd and libusb paths.

## [3.4.2] - 2025-11-28

### Added

- `nfc-rx` command line tool is shipped with the installers.

### Fixed

- Crash when the resampled buffer size equals the incoming data size (no resampling reduction) (#96).

## [3.4.1] - 2025-11-24

### Fixed

- Infinite loop in the ATR parser of the ISO-7816 decoder (#93).

## [3.4.0] - 2025-11-21

### Added

- Support for MSI2500 (Mirics) based receivers, such as the VTX3D card (#87, thanks @biemster).
- Portable ZIP package for Windows (#91, thanks @Steffen-W).
- Flatpak build option and metainfo (#89, thanks @Steffen-W).

### Changed

- Windows installer switched to NSIS (#92).
- Updated recommendations for using Renesas NFC readers with HydraSDR (#84).

### Fixed

- Capture problem with DSLogic: `PARAM_LIMIT_SAMPLES` overflow (#82, thanks @gentilkiwi).
- FFT processing crash with short buffers.
- Flatpak packaging issues.

## [3.3.2] - 2025-10-24

### Added

- Flatpak/Flathub and WinGet distribution support (#79, #80, thanks @Steffen-W).
- Console output of decoded frames and Python parser for stored and live data (#72, thanks @Steffen-W).
- `test-parser` tool and `--help` documentation for `nfc-rx` and `test-sdr` (#74, thanks @Steffen-W).
- `nfc-rx` compatibility with `nfc-lab` output formats (#77, thanks @Steffen-W).
- `CONTRIBUTING.md`.

### Changed

- Application id renamed to `io.github.josevcm.nfc-laboratory`.
- Logger and `rt` library refactor.
- Default acquire limit set to 60 seconds.

### Fixed

- Signal trace storage in `.trz` files.
- I/Q wave file sample offset reading.
- `--help` and `--version` now exit before hardware initialization.
- Thread safety issues, memory leaks and undefined behavior.

## [3.3.1] - 2025-09-21

### Changed

- HydraSDR library is now an external dependency.
- Linux build no longer fails when optional SDR libraries (airspy, hydrasdr, rtlsdr) are unavailable.

### Fixed

- Build errors with newer compilers (`QFile` initialization and missing includes).

[3.5.0]: https://github.com/josevcm/nfc-laboratory/compare/3.4.2...3.5.0
[3.4.2]: https://github.com/josevcm/nfc-laboratory/compare/3.4.1...3.4.2
[3.4.1]: https://github.com/josevcm/nfc-laboratory/compare/3.4.0...3.4.1
[3.4.0]: https://github.com/josevcm/nfc-laboratory/compare/3.3.2...3.4.0
[3.3.2]: https://github.com/josevcm/nfc-laboratory/compare/3.3.1...3.3.2
[3.3.1]: https://github.com/josevcm/nfc-laboratory/compare/3.3.0...3.3.1
