# Sentry Unreal Integration Tests

This directory contains integration tests for the Sentry Unreal SDK using Pester (PowerShell testing framework).

## Prerequisites

- **PowerShell 7+** (Core edition)
- **CMake 3.20+**
- **Pester 5+** - Install with: `Install-Module -Name Pester -Force -SkipPublisherCheck`
- **Pre-built SentryPlayground application** (from local build or CI artifact)
- **Environment variables**:
  - `SENTRY_UNREAL_TEST_DSN` - Sentry test project DSN
  - `SENTRY_AUTH_TOKEN` - Sentry API authentication token
  - `SENTRY_UNREAL_TEST_APP_PATH` - Path to the SentryPlayground executable

## Setup

### 1. Configure Integration Test Environment

Run CMake to download required PowerShell modules (app-runner):

```bash
cd integration-test
cmake -B build -S .
```

This will:
- Download `app-runner` from GitHub (contains SentryApiClient and test utilities)
- Generate `TestConfig.local.ps1` with module paths

**Note**: CMake configuration can be run without setting environment variables. The environment variables are only required at test runtime and will be validated by the test script itself.

### 2. Get SentryPlayground Application

#### Option A: Download from CI Artifacts

1. Go to [GitHub Actions](https://github.com/getsentry/sentry-unreal/actions/workflows/ci.yml)
2. Find a successful workflow run
3. Download the appropriate artifact:
   - `UE X.X sample build (Windows)` for Windows testing
   - `UE X.X sample build (Linux)` for Linux testing
4. Extract to a known location

#### Option B: Build Locally

Follow the standard Unreal Engine build process for the [sample](./sample/) project.

## Running Tests

Before running tests, ensure you have set the required environment variables:

### Windows

```powershell
# Set environment variables
$env:SENTRY_UNREAL_TEST_DSN = "https://key@org.ingest.sentry.io/project"
$env:SENTRY_AUTH_TOKEN = "sntrys_your_token_here"
$env:SENTRY_UNREAL_TEST_APP_PATH = "path/to/SentryPlayground.exe"

# Run tests
cd integration-test
Invoke-Pester Integration.Tests.ps1
```

### Linux

```bash
# Set environment variables
export SENTRY_UNREAL_TEST_DSN="https://key@org.ingest.sentry.io/project"
export SENTRY_AUTH_TOKEN="sntrys_your_token_here"
export SENTRY_UNREAL_TEST_APP_PATH="./path/to/SentryPlayground.sh"

# Run tests
cd integration-test
pwsh -Command "Invoke-Pester Integration.Tests.ps1"
```

## Test Coverage

The integration tests cover:

### Crash Capture Tests
- Application crashes with non-zero exit code
- Event ID is captured from output (set via `test.crash_id` tag)
- Crash event appears in Sentry
- Exception information is present
- Stack traces are captured
- User context is included
- Integration test tags are set
- Breadcrumbs are collected

### Message Capture Tests
- Application exits cleanly (exit code 0)
- Event ID is captured from output
- TEST_RESULT indicates success
- Message event appears in Sentry
- Message content is correct
- User context is included
- Integration test tags are set
- Breadcrumbs are collected

## Output

Test outputs are saved to `integration-test/output/`:
- `*-crash-stdout.log` - Crash test standard output
- `*-crash-stderr.log` - Crash test standard error
- `*-crash-result.json` - Full crash test result
- `*-message-stdout.log` - Message test standard output
- `*-message-stderr.log` - Message test standard error
- `*-message-result.json` - Full message test result
- `event-*.json` - Events fetched from Sentry API

## CI Integration

See `.github/workflows/integration-test-windows.yml` and `.github/workflows/integration-test-linux.yml` for CI usage examples.
