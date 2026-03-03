---
name: ue-integration-test
description: Run integration tests using the Pester framework
---

Run integration tests for the Sentry Unreal SDK using the Pester framework and app-runner module.

Refer to `integration-test/README.md` for detailed documentation on test coverage, CI integration, and SauceLabs configuration.

## Prerequisites

Before running tests, verify the following are available:

- **PowerShell 7+** (Core edition)
- **CMake 3.20+**
- **Pester 5+** - if not installed: `Install-Module -Name Pester -Force -SkipPublisherCheck`
- **Pre-built SentryPlayground application** in `Development` configuration (`Shipping` builds lack log output required by tests). If not built, suggest running `/ue-build` first.

## Steps

### 1. Setup app-runner module

Run CMake to download the app-runner PowerShell module (this step does not require env vars):

```bash
cd integration-test && cmake -B build -S .
```

### 2. Locate the test application

Look for the built application in `sample/dist/` first (the default output of `/ue-build`):

| Platform               | Expected path                              |
|------------------------|--------------------------------------------|
| Windows                | `sample/dist/SentryPlayground.exe`       |
| Linux                  | `sample/dist/SentryPlayground.sh`        |
| Android (emulator)     | `sample/dist/SentryPlayground-x64.apk`   |
| Android (device/cloud) | `sample/dist/SentryPlayground-arm64.apk` |

Resolve `SENTRY_UNREAL_TEST_APP_PATH` in this order:

1. If the expected file exists in `sample/dist/`, set the env var to its absolute path.
2. Otherwise, if `SENTRY_UNREAL_TEST_APP_PATH` is already set, use its current value.
3. Otherwise, ask the user for the path and set the env var to their input.

### 3. Verify environment variables

Check that required env vars are set (use PowerShell `.NET` method on Windows: `[System.Environment]::GetEnvironmentVariable('VAR_NAME')`). Verify non-empty only, never print values:

**Common (always required):**
- `SENTRY_UNREAL_TEST_DSN`
- `SENTRY_AUTH_TOKEN`
- `SENTRY_UNREAL_TEST_APP_PATH` - resolved automatically in step 2, or set manually

**Android via SauceLabs (required for cloud testing):**
- `SAUCE_USERNAME`
- `SAUCE_ACCESS_KEY`
- `SAUCE_REGION` - e.g., `us-west-1`, `eu-central-1`
- `SAUCE_DEVICE_NAME` - must match a device available in the specified region; datacenter suffix must align with region (`us-west-1` requires `_sjc1`, `eu-central-1` requires `_fra1`)

**Android via ADB (local testing):**
- Ensure a device or emulator is connected and visible via `adb devices`

If any required variable is missing, list which ones are missing and ask the user to set them before proceeding.

### 4. Select test file based on platform

Determine the platform from conversation context:
- Windows, Linux, macOS: `Integration.Desktop.Tests.ps1`
- Android: `Integration.Android.Tests.ps1`

If the platform is unclear, ask the user.

### 5. Run tests

Execute from the `integration-test/` directory:

```bash
cd integration-test
pwsh Invoke-Pester Integration.Desktop.Tests.ps1
```

Replace the test file name for Android as needed.

### 6. Check results

Test outputs are saved to `integration-test/output/`:

| Platform       | Output files                                                              |
|----------------|---------------------------------------------------------------------------|
| Desktop        | `*-crash-stdout.log`, `*-crash-stderr.log`, `*-crash-result.json`, `*-message-stdout.log`, `*-message-stderr.log`, `*-message-result.json`, `event-*.json` |
| Android        | `*-logcat.txt`, `event-*.json`                                            |

If tests fail, check these output files and `event-*.json` (events fetched from Sentry API) for debugging.

The `SENTRY_APP_RUNNER` environment variable, if set, points to the app-runner module source code for deeper investigation.
