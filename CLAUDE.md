# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This project is the Sentry SDK for Unreal Engine which provides crash and error monitoring as well as performance tracking for games built with Unreal Engine. It wraps multiple Sentry SDKs for supported platforms (`sentry-native` for Windows/Linux/Xbox/PlayStation/Nintendo, `sentry-cocoa` for macOS/iOS, `sentry-java` for Android) into a single Unreal Engine plugin and extends them with Unreal Engine-specific functionality.

## Plugin Architecture

### Plugin Modules

1. **Sentry** - Main plugin API with platform-specific implementations (Runtime module)
2. **SentryEditor** - Editor utilities, settings UI, build tools (Editor module)

### Source Structure

```
plugin-dev/Source/
├── Sentry/
│   ├── Public/              # Public API headers (SentrySubsystem.h, SentrySettings.h, SentryLibrary.h)
│   └── Private/
│       ├── Android/         # Android and JNI/Java interop (sentry-java wrapper)
│       ├── Apple/           # macOS + iOS (sentry-cocoa wrapper)
│       ├── GenericPlatform/ # Common code for sentry-native wrappers
│       ├── HAL/             # Hardware Abstraction Layer
│       ├── Interface/       # Abstract interfaces
│       ├── IOS/             # iOS (sentry-cocoa wrapper)
│       ├── Linux/           # Linux (sentry-native wrapper)
│       ├── Mac/             # macOS (sentry-cocoa wrapper)
│       ├── Microsoft/       # Windows + Xbox (sentry-native wrapper)
│       ├── Null/            # Implementation stubs for unsupported platforms
│       ├── Tests/           # Unit test specs
│       ├── Utils/           # Common utilities
│       └── Windows/         # Windows (sentry-native wrapper)
├── SentryEditor/            # Editor utilities
└── ThirdParty/              # Pre-built platform SDKs
```

### Platform Implementation Pattern

- Public API in `SentrySubsystem` is platform-agnostic
- Platform-specific implementations in `Private/{Platform}/` directories
- Abstract interfaces in `Interface/` define contracts
- Wrapper classes translate platform-specific SDKs to Unreal APIs

### Supported Platforms

- Win64
- Mac
- Linux
- LinuxArm64
- Android
- iOS
- PlayStation (via private extensions)
- Xbox (via private extensions)
- Switch (via private extensions)

## Development Guidelines

- When introducing a new public API that becomes part of the common interface, ensure that a corresponding stub is added to its Null implementation to avoid compilation errors on unsupported platforms.

- Package plugin and update snapshot after adding/removing files:

```powershell
./scripts/packaging/pack.ps1
./scripts/packaging/test-contents.ps1 accept
```

- When working on game console-related tasks refer to the `Building for Consoles` section in `CONTRIBUTING.md`. If access to plugin extension source code is required check paths defined in the corresponding environment variables (`SENTRY_PLAYSTATION_PATH`, `SENTRY_XBOX_PATH`, `SENTRY_SWITCH_PATH`) to locate it. Ask explicitly for the path if the environment variable isn't set or the path doesn't exist. If a plugin extension is found, it contains Unreal Engine–specific content under the `unreal` directory, which follows a structure similar to this plugin.

### Project Setup

```powershell
# Initialize (first-time setup - downloads SDK dependencies via GitHub CLI)
./scripts/init-win.ps1      # Windows
./scripts/init.sh           # macOS/Linux

# Build platform SDKs locally (optional - CI provides pre-built binaries)
./scripts/build-deps.ps1 -All
```

### Code Style

- Source files require copyright notice: `Copyright (c) YYYY Sentry. All Rights Reserved.`
- Uses `.clang-format` (Allman braces, tabs, 4-space indent)
- No BOM (Byte Order Mark) in source files
- Files must end with a single empty line (newline at EOF)

### Building

Building requires Unreal Engine. When a build is needed, check the `UE_ROOT` environment variable for the engine installation path. If `UE_ROOT` is not set or the path doesn't exist, ask for the engine path explicitly.

```powershell
# Example: Build and package the sample project (Win64, Development)
& "$env:UE_ROOT\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="$PWD\sample\SentryPlayground.uproject" `
    -archivedirectory="$PWD\sample\Builds" `
    -platform=Win64 `
    -clientconfig=Development `
    -build -cook -stage -package -archive -nop4
```

Note: For UE 4.27, the editor binary is `UE4Editor.exe`; for UE 5.x it's `UnrealEditor.exe`.

### Testing

#### Unit Tests

Run unit tests via command line (headless):

```powershell
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\sample\SentryPlayground.uproject" `
    -ExecCmds="Automation RunTests Sentry;quit" `
    -TestExit="Automation Test Queue Empty" `
    -Unattended -NoPause -NoSplash -NullRHI
```

Test results are written to `sample/Saved/Automation/`. To run tests interactively in the editor, see `CONTRIBUTING.md`.

#### Integration Tests

Refer to detailed instructions how to setup and run integration tests in `integration-test/README.md`.

Test source:
- `integration-test/Integration.Desktop.Tests.ps1` (Windows, Linux, Mac)
- `integration-test/Integration.Android.Tests.ps1` (Android)

The integration test infrastructure is built on top of the [Pester](https://github.com/pester/pester) framework and the [app-runner](https://github.com/getsentry/app-runner) PowerShell module, which provides tooling to launch the test application on different platforms, parse its output, and communicate with the Sentry API to fetch the data required to verify test results.

The exact version of the `app-runner` module used for testing is specified by a commit SHA in `integration-test\CMakeLists.txt`.

The integration tests expect the sample application to be pre-built using the Development configuration, as this is required for the application to generate the log files that the tests parse to verify output. The sample application logic is defined in sample/Source/SentryPlayground/SentryPlaygroundGameInstance.cpp and is triggered based on a set of input arguments.

Sample application output and data fetched from the Sentry API can be found in `integration-test\output` - these artifacts are useful for debugging and investigating test failures.

Pester documentation: https://pester.dev/docs/quick-start

## Sample Project

Refer to detailed sample project documentation in `sample/README.md`.

## Related Code & Repositories

[sentry-native](https://github.com/getsentry/sentry-native): crash and error monitoring on Windows/Linux and game consoles
[sentry-java](https://github.com/getsentry/sentry-java): crash and error monitoring on Android
[sentry-cocoa](https://github.com/getsentry/sentry-cocoa): crash and error monitoring on macOS and iOS
[sentry-cli](https://github.com/getsentry/sentry-cli): uploading debug symbols for symbolicating stack traces gathered via the SDK
[sentry-android-gradle-plugin](https://github.com/getsentry/sentry-android-gradle-plugin): uploading Android debug symbols

## Useful Resources

- Main SDK documentation: https://develop.sentry.dev/sdk/overview/
- Internal contributing guide: https://docs.sentry.io/internal/contributing/
- Unreal Engine SDK documentation: https://docs.sentry.io/platforms/unreal/
