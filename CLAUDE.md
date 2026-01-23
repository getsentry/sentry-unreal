# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This project is the Sentry SDK for Unreal Engine which provides crash and error monitoring as well as performance tracking for games built with Unreal Engine. It wraps multiple Sentry SDKs for supported platforms (`sentry-native` for Windows/Linux/Xbox/PlayStation/Nintendo, `sentry-cocoa` for macOS/iOS, `sentry-java` for Android) into a single Unreal Engine plugin and extends them with Unreal Engine-specific functionality.

## Plugin Architecture

### Plugin Modules

1. **Sentry** - Main plugin API with platform-specific implementations (Runtime module)
2. **SentryEditor** - Editor utilities, settings UI, build tools (Editor module)

### Plugin Source Structure

```
plugin-dev/Source/
├── Sentry/
│   ├── Public/              # Public API headers (SentrySubsystem.h, SentrySettings.h, SentryLibrary.h)
│   └── Private/
│       ├── Android/         # Android implementation (sentry-java via JNI)
│       ├── Apple/           # Base for Mac and iOS (sentry-cocoa)
│       ├── GenericPlatform/ # Base for Microsoft, Linux, PS, Switch (sentry-native)
│       ├── HAL/             # Hardware Abstraction Layer
│       ├── Interface/       # Abstract interfaces
│       ├── IOS/             # iOS overrides for Apple
│       ├── Linux/           # Linux overrides for GenericPlatform
│       ├── Mac/             # macOS overrides for Apple
│       ├── Microsoft/       # Windows/Xbox base, overrides GenericPlatform
│       ├── Null/            # Stubs for unsupported platforms
│       ├── Tests/           # Unit test specs
│       ├── Utils/           # Common utilities
│       └── Windows/         # Windows overrides for Microsoft
├── SentryEditor/            # Editor utilities
└── ThirdParty/              # Pre-built platform SDKs
```

### Platform Hierarchy

```
sentry-native (C/C++ SDK)
├── GenericPlatform/          # Base implementation for all sentry-native platforms
│   ├── Microsoft/            # Shared Windows + Xbox code
│   │   ├── Windows/          # Windows-specific overrides
│   │   └── Xbox/             # Xbox-specific overrides (private extension)
│   ├── Linux/                # Linux-specific overrides
│   ├── PlayStation/          # PlayStation-specific (private extension)
│   └── Switch/               # Nintendo Switch-specific (private extension)

sentry-cocoa (Objective-C SDK)
├── Apple/                    # Base implementation for all Apple platforms
│   ├── Mac/                  # macOS-specific overrides
│   └── IOS/                  # iOS-specific overrides

sentry-java (Java/Kotlin SDK)
└── Android/                  # Android implementation (JNI interop)

Null/                         # Stubs for unsupported platforms
```

### Public API

**Core:**
- `SentrySubsystem` - Main SDK entry point (initialize, capture events, manage scope)
- `SentrySettings` - Plugin configuration (DSN, options, feature toggles)
- `SentryLibrary` - Blueprint function library for instantiating Sentry objects

**Sentry Entities:**
- `SentryEvent`, `SentryBreadcrumb`, `SentryScope`, `SentryUser`, `SentryAttachment`, `SentryHint`, `SentryFeedback` - Data objects representing Sentry concepts

**Performance Monitoring:**
- `SentryTransaction`, `SentrySpan`, `SentryTransactionContext`, `SentrySamplingContext` - Tracing and performance measurement

**Callbacks:**
- `SentryBeforeSendHandler`, `SentryBeforeBreadcrumbHandler`, `SentryBeforeLogHandler`, `SentryTraceSampler` - Hooks for customizing SDK behavior

**Utilities:**
- `SentryVariant` - Universal value type for passing data to Sentry APIs (tags, context, etc.)
- `SentryOutputDevice` / `SentryErrorOutputDevice` - UE log output integration

### Platform Implementation Pattern

- Public API is platform-agnostic
- Platform-specific implementations in `Private/{Platform}/` directories
- Abstract interfaces in `Interface/` define contracts
- Wrapper classes translate platform-specific SDKs to Unreal APIs

## Sample Project

The `sample/` directory contains `SentryPlayground` demo game used for manual testing (demo UI) and CI integration tests.

- **Integration test logic**: `sample/Source/SentryPlayground/SentryPlaygroundGameInstance.cpp`
- **Sentry configuration**: `sample/Config/DefaultEngine.ini` under `[/Script/Sentry.SentrySettings]` section
- **Logs**: `sample/Saved/Logs/` - runtime logs, useful for debugging

Refer to `sample/README.md` for detailed documentation.

## Console Support

Console support (PlayStation, Xbox, Switch) is provided via private plugin extensions stored in separate repositories.

**Extension paths**

Path to extensions source code may be set via environment variables. When working on console-related tasks, check them first, only ask to provide path explicitly if these aren't available or invalid.

- `SENTRY_PLAYSTATION_PATH` → PS5
- `SENTRY_XBOX_PATH` → XSX, XB1
- `SENTRY_SWITCH_PATH` → Switch

**Setup console extensions:**

```powershell
./scripts/init-consoles.ps1 -All        # All platforms
./scripts/init-consoles.ps1 -PS5 -XSX   # Specific platforms
```

This builds extensions and integrates them into `sample/Platforms/{Platform}/`:
- **Binaries copied** to `Plugins/Sentry/Source/ThirdParty/`
- **Sources symlinked** for live editing (changes go directly to extension repo)
- **Config symlinked** (`{Platform}Engine.ini`)

**Extension structure:**

```
sentry-{platform}/unreal/
├── Sentry/Source/Sentry/Private/   # Platform implementation ({Platform}SentrySubsystem.cpp)
└── sample-config/                  # Platform-specific sample project settings
```

**Adding console-specific features:** Edit symlinked sources in extension repo. Follow same patterns as main plugin (`{Platform}SentryXxx.cpp`).

Refer to `Building for Consoles` in `CONTRIBUTING.md` for build instructions.

## Development Guidelines

### Project Setup

To work with the Unreal project, it has to be properly set up (symlink plugin source code into `sample/Plugins`, download dependencies).

```powershell
# Initialize (first-time setup - downloads SDK dependencies via GitHub CLI, CI provides pre-built binaries)
./scripts/init-win.ps1      # Windows
./scripts/init.sh           # macOS/Linux

# Build platform SDKs locally (optional - useful when testing unreleased native SDK changes)
./scripts/build-deps.ps1 -All
```

Supported Unreal Engine versions are listed in `scripts/packaging/engine-versions.txt`. When using an engine built from source, the `.uproject` file will contain a GUID instead of a version number in the `EngineAssociation` field.

### Common Instructions

- New plugin features often wrap existing native SDK functionality. Before implementation, examine the relevant SDK's API (`sentry-native`, `sentry-cocoa`, or `sentry-java`) to understand its usage, check platform availability, and identify interop requirements (JNI for Android, Objective-C++ for Apple). Refer to `Native SDK API Lookup Order` in Quick References for where to find SDK APIs.

- When introducing a new public API that becomes part of the common interface, ensure that a corresponding stub is added to its `Null` implementation to avoid compilation errors on unsupported platforms.

- Package plugin and update snapshot after adding/removing files:

```powershell
./scripts/packaging/pack.ps1
./scripts/packaging/test-contents.ps1 accept
```

### Code Style

- Source files require copyright notice: `Copyright (c) YYYY Sentry. All Rights Reserved.` (for `YYYY` use file creation year)
- Use file naming pattern `{Platform}SentryXxx.cpp` for platform implementations (e.g., AndroidSentrySubsystem.cpp)
- Use `.clang-format`
- No BOM (Byte Order Mark) in source files
- Files must end with a single empty line (newline at EOF)

### Security

- Never commit secrets, credentials, API keys, or tokens to the repository
- Never print or display environment variables that may contain secrets (passwords, API keys, auth tokens, DSNs)
- When checking environment variables, only verify if they are set (non-empty), don't output their values

### Project Build

Since building requires Unreal Engine, check the `UNREAL_ENGINE_ROOT` environment variable for the engine installation path. If `UNREAL_ENGINE_ROOT` is not set or the path doesn't exist, ask for the engine path explicitly. Once provided, reuse the same path for the remainder of the session unless told otherwise.

- Build configurations: `Development`, `Shipping`
- Target platforms: `Win64`, `Mac`, `Android`, `IOS`, `Linux`, `LinuxArm64`, `XSX`, `XB1`, `PS5`, `Switch`

By default, build `Development` configuration for host target platform.

```powershell
# Example: Build and package the sample project (Win64, Development)
& "$env:UNREAL_ENGINE_ROOT\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="$PWD\sample\SentryPlayground.uproject" `
    -archivedirectory="$PWD\sample\dist" `
    -platform=Win64 `
    -clientconfig=Development `
    -build -cook -stage -package -archive -nop4
```

### Running Tests

**Unit Tests**

```powershell
& "$env:UNREAL_ENGINE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\sample\SentryPlayground.uproject" `
    -ExecCmds="Automation RunTests Sentry;quit" `
    -TestExit="Automation Test Queue Empty" `
    -Unattended -NoPause -NoSplash -NullRHI
```

Note: For older UE4 versions, the editor binary is `UE4Editor.exe`; for UE5 it's `UnrealEditor.exe`.

Test results are written to `sample/Saved/Automation/`.

**Integration Tests**

Refer to detailed instructions on how to set up and run integration tests in `integration-test/README.md`.

Test source:
- `integration-test/Integration.Desktop.Tests.ps1` (Windows, Linux, Mac)
- `integration-test/Integration.Android.Tests.ps1` (Android)

The integration test infrastructure is built on top of the [Pester](https://github.com/pester/pester) framework and the [app-runner](https://github.com/getsentry/app-runner) PowerShell module, which provides tooling to launch the test application on different platforms/devices, parse its output, and communicate with the Sentry API to fetch the data required to verify test results.

The exact version of the `app-runner` module used for testing is specified by a commit SHA in `integration-test/CMakeLists.txt`.

The integration tests expect the sample application to be pre-built using the `Development` configuration, as this is required for the application to generate the log files that the tests parse to verify output. The sample application logic is defined in `sample/Source/SentryPlayground/SentryPlaygroundGameInstance.cpp` and the test scenario triggered at startup is determined by command-line input arguments.

Sample application output and data fetched from the Sentry API can be found in `integration-test/output` - these artifacts are useful for debugging and investigating test failures.

Pester documentation: https://pester.dev/docs/quick-start

### Troubleshooting

- If the build, test, or script execution fails, try to understand the root cause of the error and suggest a fix. Check logs for additional issue insights.

## Related Code & Repositories

- [sentry-native](https://github.com/getsentry/sentry-native): crash and error monitoring on Windows/Linux and game consoles
- [sentry-java](https://github.com/getsentry/sentry-java): crash and error monitoring on Android
- [sentry-cocoa](https://github.com/getsentry/sentry-cocoa): crash and error monitoring on macOS and iOS
- [sentry-cli](https://github.com/getsentry/sentry-cli): uploading debug symbols for symbolicating stack traces gathered via the SDK
- [sentry-android-gradle-plugin](https://github.com/getsentry/sentry-android-gradle-plugin): uploading Android debug symbols
- [app-runner](https://github.com/getsentry/app-runner): PowerShell module used in integration tests

## Useful Resources

- Main SDK documentation: https://develop.sentry.dev/sdk/overview/
- Internal contributing guide: https://docs.sentry.io/internal/contributing/
- Unreal Engine SDK documentation: https://docs.sentry.io/platforms/unreal/

## Quick References & Tips

### Native SDK API Lookup Order

When implementing features that wrap native SDK functionality, check APIs in the following order (proceed to next source only if previous is unavailable or lacks context):

1. **ThirdParty headers** - `plugin-dev/Source/ThirdParty/{Platform}/` contains SDK headers

2. **Repo submodules** - `modules/` contains `sentry-native` and `sentry-java` sources

3. **Local source** - Check env vars specifying local repository path; if not set, prompt user; if not provided, proceed to next source:
    - `SENTRY_NATIVE_PATH` - sentry-native repository
    - `SENTRY_COCOA_PATH` - sentry-cocoa repository
    - `SENTRY_JAVA_PATH` - sentry-java repository

4. **GitHub** - Fetch from repositories listed in `Related Code & Repositories` section as last resort

### Reading Environment Variables

When checking env vars via PowerShell through the Bash tool, use the .NET method with single-quoted strings to avoid shell interpolation issues:

```powershell
[System.Environment]::GetEnvironmentVariable('VAR_NAME')
```

## Maintaining This Document

- When completing a task that reveals new patterns, conventions, best practices, or solutions to recurring issues not yet documented here, suggest adding these insights to `CLAUDE.md`.

- When using compaction (which condenses context by summarizing older messages), make sure to re-read `CLAUDE.md` afterward to keep it fully available in context.
