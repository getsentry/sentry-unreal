---
name: ue-unit-test
description: Run Sentry unit tests via Unreal automation
---

Run Sentry unit tests via Unreal Engine automation framework.

Prerequisite: the project must be built first. If unsure whether it's been built, ask the user. If not built, suggest running `/ue-build` first.

1. Check the `UNREAL_ENGINE_ROOT` environment variable (use PowerShell `.NET` method on Windows: `[System.Environment]::GetEnvironmentVariable('UNREAL_ENGINE_ROOT')`). If unset or the path doesn't exist, ask the user for the engine path.

2. Determine the engine version by reading the `EngineAssociation` field in `sample/SentryPlayground.uproject` (e.g., `"EngineAssociation": "5.7"` - UE5, `"EngineAssociation": "4.27"` - UE4). Select the editor binary based on host OS and engine version:

| Host OS | UE5 binary path                          | UE4 binary path                       |
|---------|------------------------------------------|---------------------------------------|
| Windows | `Engine/Binaries/Win64/UnrealEditor.exe` | `Engine/Binaries/Win64/UE4Editor.exe` |
| macOS   | `Engine/Binaries/Mac/UnrealEditor`       | `Engine/Binaries/Mac/UE4Editor`       |
| Linux   | `Engine/Binaries/Linux/UnrealEditor`     | `Engine/Binaries/Linux/UE4Editor`     |

3. Run the automation command from the repository root:

```bash
# Windows UE5
"$UNREAL_ENGINE_ROOT/Engine/Binaries/Win64/UnrealEditor.exe" \
    "$PWD/sample/SentryPlayground.uproject" \
    -ReportExportPath="$PWD/sample/Saved/Automation" \
    -ExecCmds="Automation RunTests Sentry;quit" \
    -TestExit="Automation Test Queue Empty" \
    -Unattended -NoPause -NoSplash -NullRHI

# macOS UE5
"$UNREAL_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor" \
    "$PWD/sample/SentryPlayground.uproject" \
    -ReportExportPath="$PWD/sample/Saved/Automation" \
    -ExecCmds="Automation RunTests Sentry;quit" \
    -TestExit="Automation Test Queue Empty" \
    -Unattended -NoPause -NoSplash -NullRHI
```

4. Report that test results are written to `sample/Saved/Automation/`.
