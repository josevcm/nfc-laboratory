# Flatpak Integration

NFC Laboratory is available as a Flatpak package for Linux systems.

## Quick Install (from Flathub)

Once published on Flathub:

```bash
flatpak install flathub com.github.josevcm.nfc-laboratory
flatpak run com.github.josevcm.nfc-laboratory
```

## Local Build

### Prerequisites

```bash
sudo apt install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub org.kde.Platform//6.8 org.kde.Sdk//6.8
```

**Note:** Runtime version 6.8 is the current stable version. Check available versions:
```bash
flatpak remote-info flathub org.kde.Platform | grep Branch
```

### Build and Install

```bash
flatpak-builder --user --install --force-clean build-dir com.github.josevcm.nfc-laboratory.yml
```

### Run

```bash
flatpak run com.github.josevcm.nfc-laboratory
```

## Validation

```bash
appstreamcli validate --pedantic dat/flatpak/com.github.josevcm.nfc-laboratory.metainfo.xml
desktop-file-validate dat/flatpak/com.github.josevcm.nfc-laboratory.desktop
```

## Hardware Access

The Flatpak requires USB access for SDR receivers and logic analyzers:

1. Add user to plugdev group:
   ```bash
   sudo usermod -a -G plugdev $USER
   ```

2. Install udev rules for your hardware (device-specific)

3. Log out and log back in

## File Structure

- `com.github.josevcm.nfc-laboratory.yml` - Flatpak manifest
- `dat/flatpak/com.github.josevcm.nfc-laboratory.metainfo.xml` - AppStream metadata
- `dat/flatpak/com.github.josevcm.nfc-laboratory.desktop` - Desktop entry
- `dat/flatpak/README.md` - Additional documentation

## Configuration

User configuration is stored in:
```
~/.var/app/com.github.josevcm.nfc-laboratory/.nfc-lab/
```

## Support

For issues specific to the Flatpak package, please report at:
https://github.com/josevcm/nfc-laboratory/issues
