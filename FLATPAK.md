# Flatpak Integration

NFC Laboratory is available as a Flatpak package for Linux systems.

## Quick Install (from Flathub)

Once published on Flathub:

```bash
flatpak install flathub io.github.josevcm.nfc-laboratory
flatpak run io.github.josevcm.nfc-laboratory
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
flatpak-builder --user --install --force-clean build-dir io.github.josevcm.nfc-laboratory.yml
```

### Run

```bash
flatpak run io.github.josevcm.nfc-laboratory
```

## Validation

```bash
# Standard validation
appstreamcli validate --pedantic dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml
desktop-file-validate dat/flatpak/io.github.josevcm.nfc-laboratory.desktop

# Flatpak-builder lint (optional, for Flathub submission)
flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest io.github.josevcm.nfc-laboratory.yml
flatpak run --command=flatpak-builder-lint org.flatpak.Builder appstream dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml
```

## Testing

```bash
# Test flags
flatpak run io.github.josevcm.nfc-laboratory --help
flatpak run io.github.josevcm.nfc-laboratory --version

# Check library dependencies
flatpak run --command=sh io.github.josevcm.nfc-laboratory -c "ldd /app/bin/nfc-lab | grep 'not found'"

# Build with local repository (for testing installation)
flatpak-builder --force-clean --repo=repo build-dir io.github.josevcm.nfc-laboratory.yml
flatpak --user remote-add --no-gpg-verify test-repo repo
flatpak --user install test-repo io.github.josevcm.nfc-laboratory

# Lint build artifacts (may show screenshot warnings - acceptable)
flatpak run --command=flatpak-builder-lint org.flatpak.Builder builddir build-dir
flatpak run --command=flatpak-builder-lint org.flatpak.Builder repo repo
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

- `io.github.josevcm.nfc-laboratory.yml` - Flatpak manifest
- `dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml` - AppStream metadata
- `dat/flatpak/io.github.josevcm.nfc-laboratory.desktop` - Desktop entry
- `dat/flatpak/README.md` - Additional documentation

## Configuration

User configuration is stored in:

```
~/.var/app/io.github.josevcm.nfc-laboratory/.nfc-lab/
```

## Support

For issues specific to the Flatpak package, please report at:
<https://github.com/josevcm/nfc-laboratory/issues>
