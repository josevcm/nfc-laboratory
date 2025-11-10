# Flatpak Distribution

NFC Laboratory is available as a Flatpak package on Flathub.

## Install

```bash
flatpak install flathub io.github.josevcm.nfc-laboratory
flatpak run io.github.josevcm.nfc-laboratory
```

## Hardware Access

USB access for SDR receivers and logic analyzers:

```bash
sudo usermod -a -G plugdev $USER  # Add user to plugdev group
# Install device-specific udev rules
# Log out and back in
```

## Configuration

User data stored in: `~/.var/app/io.github.josevcm.nfc-laboratory/.nfc-lab/`

## Building Locally

The project uses a universal build system that supports both normal and Flatpak builds:

```bash
# Normal build (installs as 'nfc-lab')
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Flatpak build (installs as 'io.github.josevcm.nfc-laboratory')
cmake -B build -DCMAKE_BUILD_TYPE=Release -DFLATPAK_BUILD=ON
cmake --build build
```

The Flatpak manifest is maintained in the [Flathub repository](https://github.com/flathub/io.github.josevcm.nfc-laboratory).

## Metadata Files

Flatpak metadata is located in:
- `dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml` - AppStream metadata
- `dat/flatpak/io.github.josevcm.nfc-laboratory.desktop` - Desktop entry
- `src/nfc-app/app-qt/src/main/assets/app/rc/` - Files installed during build
