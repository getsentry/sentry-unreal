---
name: ue-build
description: Build the SentryPlayground sample project for a target platform
---

Build the SentryPlayground sample project.

1. Check the `UNREAL_ENGINE_ROOT` environment variable (use PowerShell `.NET` method on Windows: `[System.Environment]::GetEnvironmentVariable('UNREAL_ENGINE_ROOT')`). If unset or the path doesn't exist, ask the user for the engine path.

2. Determine the target platform from conversation context using this lookup table. If no platform is mentioned, default to the host platform (`Win64` on Windows, `Mac` on macOS, `Linux` on Linux). If ambiguous, ask the user.

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

3. Determine the build configuration: use `Development` by default, or `Shipping` if the user mentions it.

4. Select the UAT script based on host OS:
   - Windows: `RunUAT.bat`
   - macOS/Linux: `RunUAT.sh`

5. Run the build command from the repository root:

```bash
"$UNREAL_ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun \
    -project="$PWD/sample/SentryPlayground.uproject" \
    -archivedirectory="$PWD/sample/dist" \
    -platform=Win64 -clientconfig=Development \
    -build -cook -stage -package -archive -nop4
```

Replace `RunUAT.bat` with `RunUAT.sh` on macOS/Linux, and substitute the platform and config values as determined above.

For Android Shipping builds, add the `-distribution` flag.
