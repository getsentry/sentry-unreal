---
name: ue-unit-test
description: Run Sentry unit tests via Unreal automation
---

Run Sentry unit tests via Unreal Engine automation framework.

Prerequisite: the project must be built first (a compile-only editor build is sufficient). If unsure whether it's been built, ask the user. If not built, suggest running `/ue-build` first.

1. Check the `UNREAL_ENGINE_ROOT` environment variable (use PowerShell `.NET` method on Windows: `[System.Environment]::GetEnvironmentVariable('UNREAL_ENGINE_ROOT')`). If unset or the path doesn't exist, ask the user for the engine path.

2. Determine the engine version by reading the `EngineAssociation` field in `sample/SentryPlayground.uproject` (e.g., `"EngineAssociation": "5.7"` - UE5, `"EngineAssociation": "4.27"` - UE4). Select the headless editor binary based on host OS and engine version:

| Host OS | UE5 binary path                              | UE4 binary path                           |
|---------|----------------------------------------------|-------------------------------------------|
| Windows | `Engine/Binaries/Win64/UnrealEditor-Cmd.exe` | `Engine/Binaries/Win64/UE4Editor-Cmd.exe` |
| macOS   | `Engine/Binaries/Mac/UnrealEditor-Cmd`       | `Engine/Binaries/Mac/UE4Editor-Cmd`       |
| Linux   | `Engine/Binaries/Linux/UnrealEditor-Cmd`     | `Engine/Binaries/Linux/UE4Editor-Cmd`     |

3. Resolve the repo root and run the automation command:

```bash
REPO=$(git rev-parse --show-toplevel)

# Windows UE5
"$UNREAL_ENGINE_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" \
    "$REPO/sample/SentryPlayground.uproject" \
    -ReportExportPath="$REPO/sample/Saved/Automation" \
    -ExecCmds="Automation RunTests Sentry;quit" \
    -TestExit="Automation Test Queue Empty" \
    -Unattended -NoPause -NoSplash -NullRHI

# macOS UE5
"$UNREAL_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd" \
    "$REPO/sample/SentryPlayground.uproject" \
    -ReportExportPath="$REPO/sample/Saved/Automation" \
    -ExecCmds="Automation RunTests Sentry;quit" \
    -TestExit="Automation Test Queue Empty" \
    -Unattended -NoPause -NoSplash -NullRHI

# Linux UE5
"$UNREAL_ENGINE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" \
    "$REPO/sample/SentryPlayground.uproject" \
    -ReportExportPath="$REPO/sample/Saved/Automation" \
    -ExecCmds="Automation RunTests Sentry;quit" \
    -TestExit="Automation Test Queue Empty" \
    -Unattended -NoPause -NoSplash -NullRHI
```

The test run can take longer than the default command timeout — run it in the background and monitor its output, or raise the timeout accordingly.

4. Report the results. The editor's exit code is not a reliable pass/fail signal for automation runs — read the report at `sample/Saved/Automation/index.json` instead:

   - Summarize the number of passed and failed tests (`succeeded` / `failed` counts).
   - For each failed test, report its full name and the error messages from its report entries.

   If the report is missing or the run crashed, check `sample/Saved/Logs/` for the root cause.
