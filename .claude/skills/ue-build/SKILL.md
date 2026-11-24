---
name: ue-build
description: Build the SentryPlayground sample project for a target platform
---

Build the SentryPlayground sample project.

1. Check the `UNREAL_ENGINE_ROOT` environment variable (use PowerShell `.NET` method on Windows: `[System.Environment]::GetEnvironmentVariable('UNREAL_ENGINE_ROOT')`). If unset or the path doesn't exist, ask the user for the engine path.

2. Choose the build mode from conversation context:

   - **Compile-only** (default) - when the goal is verifying that code compiles after changes, or as a prerequisite for `/ue-unit-test`. Invokes UnrealBuildTool directly and is much faster than a full package.
   - **Full package** - when the user wants a packaged, runnable game build (e.g. as a prerequisite for `/ue-integration-test`, or if they mention packaging/cooking/distribution).

3. Determine the target platform from conversation context using this lookup table. If no platform is mentioned, default to the host platform (`Win64` on Windows, `Mac` on macOS, `Linux` on Linux). If ambiguous, ask the user.

   | Alias                | Platform flag |
   |----------------------|---------------|
   | Win64, Windows       | Win64         |
   | Mac, macOS           | Mac           |
   | Android              | Android       |
   | iOS                  | IOS           |
   | Linux                | Linux         |
   | LinuxArm64           | LinuxArm64    |
   | XSX, Xbox Series     | XSX           |
   | XB1, Xbox One        | XB1           |
   | PS5, PlayStation     | PS5           |
   | Switch, NX           | Switch        |

4. Determine the build configuration: use `Development` by default, or `Shipping` if the user mentions it.

## Compile-only build

Resolve the repo root and invoke UnrealBuildTool with the editor target for the host platform:

```bash
REPO=$(git rev-parse --show-toplevel)

# Windows
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/Build.bat" SentryPlaygroundEditor Win64 Development \
    -project="$REPO/sample/SentryPlayground.uproject" -waitmutex

# macOS
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh" SentryPlaygroundEditor Mac Development \
    -project="$REPO/sample/SentryPlayground.uproject" -waitmutex

# Linux
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" SentryPlaygroundEditor Linux Development \
    -project="$REPO/sample/SentryPlayground.uproject" -waitmutex
```

To compile-check a non-host or non-editor platform (e.g. Android, iOS, consoles), build the game target instead, substituting the platform and configuration: `SentryPlayground <Platform> <Config>`.

## Full package build

Resolve the repo root and run the BuildCookRun command:

```bash
REPO=$(git rev-parse --show-toplevel)

# Windows
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun \
    -project="$REPO/sample/SentryPlayground.uproject" \
    -archivedirectory="$REPO/sample/dist" \
    -platform=Win64 -clientconfig=Development \
    -build -cook -stage -package -archive -nop4

# macOS
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="$REPO/sample/SentryPlayground.uproject" \
    -archivedirectory="$REPO/sample/dist" \
    -platform=Mac -clientconfig=Development \
    -build -cook -stage -package -archive -nop4

# Linux
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="$REPO/sample/SentryPlayground.uproject" \
    -archivedirectory="$REPO/sample/dist" \
    -platform=Linux -clientconfig=Development \
    -build -cook -stage -package -archive -nop4
```

Substitute the platform and config values as determined above.

For Android `Shipping` builds, add the `-distribution` flag.

## Execution notes

- Builds regularly take longer than the default command timeout. Run the build command in the background and monitor its output, or raise the timeout accordingly.
- On failure, check the logs for the root cause:
  - UnrealBuildTool: `$UNREAL_ENGINE_ROOT/Engine/Programs/UnrealBuildTool/Log.txt`
  - UAT (BuildCookRun): `$UNREAL_ENGINE_ROOT/Engine/Programs/AutomationTool/Saved/Logs/`
