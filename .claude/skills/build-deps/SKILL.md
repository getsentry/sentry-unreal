---
name: build-deps
description: Build plugin SDK dependencies locally and refresh ThirdParty binaries
---

Build plugin dependencies from local SDK checkouts and replace the pre-built binaries under `plugin-dev/Source/ThirdParty/`. Use when testing unreleased SDK changes.

## Steps

### 1. Select dependencies to build

Determine which dependencies to build from conversation context:

| Dependency          | Flag             | Path parameter       | Env var fallback             | Notes                                                                                     |
|---------------------|------------------|----------------------|------------------------------|-------------------------------------------------------------------------------------------|
| sentry-native       | `-Native`        | `-NativePath`        | `SENTRY_NATIVE_PATH`         | Builds for host platform: Windows/Linux (Crashpad + Native backends), macOS (Native backend) |
| sentry-cocoa        | `-Cocoa`         | `-CocoaPath`         | `SENTRY_COCOA_PATH`          | macOS host only; produces iOS + Mac artifacts                                              |
| sentry-java         | `-Java`          | `-JavaPath`          | `SENTRY_JAVA_PATH`           | Gradle build; also downloads the matching sentry-native NDK release                        |
| crash reporter      | `-CrashReporter` | `-CrashReporterPath` | `SENTRY_CRASH_REPORTER_PATH` | .NET build for host platform                                                               |

`-All` builds everything supported on the host platform.

### 2. Verify source paths

For each selected dependency, check that its env var is set (verify non-empty only, never print values; on Windows use `[System.Environment]::GetEnvironmentVariable('VAR_NAME')`) or that an explicit path parameter was provided. If missing, ask the user for the repository path and pass it via the corresponding path parameter.

### 3. Run the build

Execute from the repo root:

```bash
pwsh ./scripts/build-deps.ps1 -All                                        # everything for host platform
pwsh ./scripts/build-deps.ps1 -Cocoa -Java                                # specific SDKs
pwsh ./scripts/build-deps.ps1 -Native -NativePath "/path/to/sentry-native" # explicit source path
```

The build can take longer than the default command timeout — run it in the background and monitor its output, or raise the timeout accordingly.

**Linux notes (sentry-native):** the build requires clang with libc++ (static libs are built against libc++ to match Unreal; the crash handler executables use libstdc++). The host machine's clang may not match the version Unreal uses — prefer the clang toolchain bundled with the engine by prepending its `bin` directory to `PATH` before running the script. Look for it under `$UNREAL_ENGINE_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/<toolchain-version>/x86_64-unknown-linux-gnu/bin` (the target architecture is auto-detected by the script).

### 4. Clean stale build artifacts

UnrealBuildTool does not reliably detect ThirdParty binary changes, so after a successful deps rebuild remove the plugin `Binaries`/`Intermediate` directories to avoid linking against stale binaries:

```bash
REPO=$(git rev-parse --show-toplevel)
rm -rf "$REPO/plugin-dev/Binaries" "$REPO/plugin-dev/Intermediate"
```

`sample/Plugins/sentry` is a symlink to `plugin-dev`, so the plugin artifacts live in `plugin-dev/` itself.

### 5. Rebuild the project

Suggest running `/ue-build` so the project is recompiled against the freshly built dependencies.
