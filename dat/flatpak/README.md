# Flatpak Integration

This directory contains AppStream metadata and desktop entry files for Flatpak packaging.

## Files

- `io.github.josevcm.nfc-laboratory.metainfo.xml` - AppStream metadata
- `io.github.josevcm.nfc-laboratory.desktop` - Desktop entry file

## Main Manifest

The Flatpak manifest is located in the repository root:
- `io.github.josevcm.nfc-laboratory.yml`

## Local Build

### Install Prerequisites

```bash
sudo apt install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub org.kde.Platform//6.10 org.kde.Sdk//6.10
```

**Note:** Runtime version 6.10 is the current stable version. Check available versions with:
```bash
flatpak remote-info flathub org.kde.Platform | grep Branch
```

### Build and Install

```bash
flatpak-builder --user --install --force-clean build-dir io.github.josevcm.nfc-laboratory.yml
```

### Run

```bash
flatpak run io.github.josevcm.nfc-laboratory
```

## Validation

```bash
flatpak run --command=appstreamcli org.freedesktop.Platform validate \
  dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml

flatpak run --command=desktop-file-validate org.freedesktop.Platform \
  dat/flatpak/io.github.josevcm.nfc-laboratory.desktop
```

## Hardware Access

The Flatpak uses `--device=usb` for SDR receivers and logic analyzers.
User must be in the `plugdev` group and have appropriate udev rules installed.
