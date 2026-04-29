# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This project is the Sentry SDK for Unreal Engine which provides crash and error monitoring as well as performance tracking for games built with Unreal Engine. It wraps multiple Sentry SDKs for supported platforms (`sentry-native` for Windows/Linux/Xbox/WinGDK/PlayStation/Nintendo, `sentry-cocoa` for macOS/iOS, `sentry-java` for Android) into a single Unreal Engine plugin and extends them with Unreal Engine-specific functionality.

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
│       ├── Microsoft/       # Windows/Xbox/WinGDK base, overrides GenericPlatform
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
│   ├── Microsoft/            # Shared Windows + Xbox + WinGDK code
│   │   ├── Windows/          # Windows-specific overrides
│   │   └── Xbox/             # Xbox-specific overrides (private extension)
│   │   └── WinGDK/           # WinGDK-specific overrides (private extension)
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

- **Integration test dispatch**: `sample/Source/SentryPlayground/SentryPlaygroundGameInstance.cpp` — `CheckForPendingIntegrationTest` maps command-line switches (e.g. `-crash-capture`) to test factories
- **Integration test implementations**: `sample/Source/SentryPlayground/IntegrationTests/` — one class per test, each extending `FSentryBaseIntegrationTest`
- **Sentry configuration**: `sample/Config/DefaultEngine.ini` under `[/Script/Sentry.SentrySettings]` section
- **Logs**: `sample/Saved/Logs/` - runtime logs, useful for debugging

**Adding a new integration test:**

1. Create `Sentry<Name>Test.{h,cpp}` under `IntegrationTests/` as a subclass of `FSentryBaseIntegrationTest` and implement `Run()`
2. Add an entry to the dispatch table in `USentryPlaygroundGameInstance::CheckForPendingIntegrationTest` mapping the command-line switch to a factory lambda
3. Document the switch in `sample/README.md` under the Integration Tests section

Refer to `sample/README.md` for detailed documentation.

## Console Support

Console support (PlayStation, Xbox/WinGDK, Switch) is provided via private plugin extensions stored in separate repositories.

**Extension paths**

Path to extensions source code may be set via environment variables. When working on console-related tasks, check them first, only ask to provide path explicitly if these aren't available or invalid.

- `SENTRY_PLAYSTATION_PATH` → PS5
- `SENTRY_XBOX_PATH` → XSX, XB1, WinGDK
- `SENTRY_SWITCH_PATH` → Switch

**Setup console extensions:**

```bash
pwsh ./scripts/init-consoles.ps1 -All        # All platforms
pwsh ./scripts/init-consoles.ps1 -PS5 -XSX   # Specific platforms
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

## Crash Reporter

The plugin includes an optional external crash reporter - a .NET desktop application that shows a user-facing dialog when a crash occurs, allowing players to submit feedback and review crash details (tags, context, stacktrace). Supported on Windows and Linux (x64 and ARM64).

- **Sources**: `modules/sentry-crash-reporter` submodule or local checkout via `SENTRY_CRASH_REPORTER_PATH` env var

## Development Guidelines

### Project Setup

To work with the Unreal project, it has to be properly set up (symlink plugin source code into `sample/Plugins`, download dependencies).

```bash
# Initialize (first-time setup - downloads SDK dependencies via GitHub CLI, CI provides pre-built binaries)
pwsh ./scripts/init-win.ps1      # Windows
./scripts/init.sh                # macOS/Linux

# Build platform SDKs and other dependencies locally (optional - useful when testing unreleased changes)
pwsh ./scripts/build-deps.ps1 -All
```

Supported Unreal Engine versions are listed in `scripts/packaging/engine-versions.txt`. When using an engine built from source, the `.uproject` file will contain a GUID instead of a version number in the `EngineAssociation` field.

### Common Instructions

- Before implementating new plugin features that wrap existing native SDK functionality, examine the relevant SDK's API (`sentry-native`, `sentry-cocoa`, or `sentry-java`) to understand its usage, check platform availability, and identify interop requirements (JNI for Android, Objective-C++ for Apple). Refer to `Native SDK API Lookup Order` in Quick References for where to find SDK APIs.

- When introducing a new public API that becomes part of the common interface, ensure that a corresponding stub is added to its `Null` implementation to avoid compilation errors on unsupported platforms.

- Package plugin and update snapshot after adding/removing files (see `/pack` skill).

### Code Style

- Source files require copyright notice: `Copyright (c) YYYY Sentry. All Rights Reserved.` (for `YYYY` use file creation year)
- Use file naming pattern `{Platform}SentryXxx.cpp` for platform implementations (e.g., AndroidSentrySubsystem.cpp)
- Use `.clang-format`
- No BOM (Byte Order Mark) in source files
- Files must end with a single empty line (newline at EOF)
- Avoid giving a `UENUM` and a `USTRUCT`/`UCLASS` the same base name after prefix stripping (e.g., `ESentryFoo` + `FSentryFoo` both become `SentryFoo` in Python). If unavoidable, add `meta = (ScriptName = "...")` to the `UENUM` to resolve the collision.

### Project Build & Testing

Before constructing shell commands manually, run `make help` to check for ready-to-use Makefile targets (e.g. `make build-macos`, `make build-ios`, `make stage-framework`).

Build and test workflows are also available as skills:

- `/ue-build` - Build sample project for a target platform
- `/ue-unit-test` - Run Sentry unit tests
- `/ue-integration-test` - Run integration tests

### Security

- Never commit secrets, credentials, API keys, or tokens to the repository
- Never print or display environment variables that may contain secrets (passwords, API keys, auth tokens, DSNs)
- When checking environment variables, only verify if they are set (non-empty), don't output their values

### Troubleshooting

- If the build, test, or script execution fails, try to understand the root cause of the error and suggest a fix. Check logs for additional issue insights.

## Quick References & Tips

### Native SDK API Lookup Order

When implementing features that wrap native SDK functionality, check APIs in the following order (proceed to next source only if previous is unavailable or lacks context):

1. **ThirdParty headers** - pre-built SDK headers and libraries:
    - `plugin-dev/Source/ThirdParty/Win64/` - sentry-native (C headers + .lib)
    - `plugin-dev/Source/ThirdParty/Linux/`, `LinuxArm64/` - sentry-native (C headers + .a)
    - `plugin-dev/Source/ThirdParty/Mac/` - sentry-cocoa (Objective-C headers)
    - `plugin-dev/Source/ThirdParty/IOS/` - sentry-cocoa (Objective-C framework)
    - `plugin-dev/Source/ThirdParty/Android/` - sentry-java (.aar/.jar)

2. **Repo submodules** - full SDK sources in `modules/`:
    - `modules/sentry-native` - sentry-native submodule
    - `modules/sentry-java` - sentry-java submodule

3. **Local source** - Check env vars specifying local repository path; if not set, prompt user; if not provided, proceed to next source:
    - `SENTRY_NATIVE_PATH` - sentry-native repository
    - `SENTRY_COCOA_PATH` - sentry-cocoa repository
    - `SENTRY_JAVA_PATH` - sentry-java repository

4. **GitHub** - Fetch from GitHub as last resort:
    - [sentry-native](https://github.com/getsentry/sentry-native)
    - [sentry-cocoa](https://github.com/getsentry/sentry-cocoa)
    - [sentry-java](https://github.com/getsentry/sentry-java)

### Unreal Engine Source Lookup

When researching how Unreal Engine implements specific features (subsystems, modules, plugins, build pipeline, etc.), check the `UNREAL_ENGINE_ROOT` environment variable for the path to a local UE source tree. Key source locations within the engine:

- `Engine/Source/Runtime/` - Core runtime modules (Core, CoreUObject, Engine, etc.)
- `Engine/Source/Editor/` - Editor modules
- `Engine/Source/Developer/` - Developer tools and utilities
- `Engine/Source/Programs/` - Standalone programs (UnrealBuildTool, AutomationTool, etc.)
- `Engine/Plugins/` - Built-in engine plugins

If the env var is not set, ask the user for the path before falling back to online documentation.

### Reading Environment Variables

On Windows, when checking env vars via PowerShell through the Bash tool, use the .NET method with single-quoted strings to avoid shell interpolation issues:

```powershell
[System.Environment]::GetEnvironmentVariable('VAR_NAME')
```

### Useful Resources

- Main SDK documentation: https://develop.sentry.dev/sdk/overview/
- Internal contributing guide: https://docs.sentry.io/internal/contributing/
- Unreal Engine SDK documentation: https://docs.sentry.io/platforms/unreal/

### Other Related Code & Repositories

- [sentry-native](https://github.com/getsentry/sentry-native): crash and error monitoring on Windows/Linux and game consoles
- [sentry-java](https://github.com/getsentry/sentry-java): crash and error monitoring on Android
- [sentry-cocoa](https://github.com/getsentry/sentry-cocoa): crash and error monitoring on macOS and iOS
- [sentry-desktop-crash-reporter](https://github.com/getsentry/sentry-desktop-crash-reporter): external crash reporter dialog for Windows/Linux
- [sentry-cli](https://github.com/getsentry/sentry-cli): uploading debug symbols for symbolicating stack traces gathered via the SDK
- [sentry-android-gradle-plugin](https://github.com/getsentry/sentry-android-gradle-plugin): uploading Android debug symbols
- [app-runner](https://github.com/getsentry/app-runner): PowerShell module used in integration tests

## Maintaining This Document

- When completing a task that reveals new patterns, conventions, best practices, or solutions to recurring issues not yet documented here, suggest adding these insights to `CLAUDE.md`.
- When using compaction (which condenses context by summarizing older messages), make sure to re-read `CLAUDE.md` afterward to keep it fully available in context.
