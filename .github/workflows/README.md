# GitHub Actions Workflows

This directory contains automated workflows for the NFC Laboratory project.

## Flatpak CI (`flatpak.yml`)

Automatically validates and builds the Flatpak package on every pull request and push to master/develop branches.

### Triggers

- Pull requests that modify:
  - `io.github.josevcm.nfc-laboratory.yml`
  - `dat/flatpak/**`
  - `.github/workflows/flatpak.yml`
- Pushes to `master` or `develop` branches with the same file changes

### Jobs

1. **validate-metadata**: Validates AppStream metadata and Desktop file
   - Uses `appstreamcli validate --pedantic`
   - Uses `desktop-file-validate`

2. **flatpak-builder**: Builds the Flatpak bundle
   - Runs in KDE 6.8 container
   - Uses official Flatpak GitHub Actions
   - Uploads build artifact for 7 days

### Benefits

- Ensures Flatpak manifest is always valid
- Catches errors before manual testing
- Provides ready-to-test Flatpak bundles
- Simplifies Flathub submission process

## Flatpak Update Check (`flatpak-update-check.yml`)

Automatically checks for new upstream releases and creates issues to track updates.

### Triggers

- Weekly on Sunday at midnight UTC (via cron)
- Manually via workflow dispatch

### Functionality

1. Fetches latest release from GitHub API
2. Compares with current version in Flatpak manifest
3. Creates GitHub issue if update is available
4. Includes step-by-step update instructions

### Issue Labels

Automated issues are created with these labels:
- `flatpak`
- `update`
- `automated`

### Manual Trigger

You can manually run this workflow from the Actions tab:
1. Go to Actions → Check Flatpak Updates
2. Click "Run workflow"
3. Select branch and run

## Testing Workflows Locally

### Validate metadata locally

```bash
appstreamcli validate --pedantic dat/flatpak/io.github.josevcm.nfc-laboratory.metainfo.xml
desktop-file-validate dat/flatpak/io.github.josevcm.nfc-laboratory.desktop
```

### Build Flatpak locally

```bash
flatpak-builder --user --install --force-clean build-dir io.github.josevcm.nfc-laboratory.yml
```

### Test the built Flatpak

```bash
flatpak run io.github.josevcm.nfc-laboratory
```

## Workflow Dependencies

- GitHub Actions marketplace actions:
  - `actions/checkout@v4`
  - `actions/upload-artifact@v4`
  - `actions/github-script@v7`
  - `flatpak/flatpak-github-actions/flatpak-builder@v6`

- Container images:
  - `ghcr.io/flathub-infra/flatpak-github-actions:kde-6.8`

## Troubleshooting

### Build fails with "Runtime not found"

The workflow uses KDE Platform 6.8, which is the current stable version. If you need to use a different version, update the container image:

```yaml
container:
  image: ghcr.io/flathub-infra/flatpak-github-actions:kde-<VERSION>
```

### Validation fails

Check the workflow logs for specific errors:
- AppStream validation errors: Review `metainfo.xml`
- Desktop file validation errors: Review `.desktop` file

### Update check creates duplicate issues

The workflow checks for existing open issues before creating new ones. If you see duplicates, check that the issue title matches exactly.
