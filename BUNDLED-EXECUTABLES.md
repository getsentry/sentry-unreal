# Bundled executables & scripts

This plugin ships a set of executables and scripts that are required for its crash-reporting and debug-symbol-upload functionality. This notice documents each one — its location, source, whether it was modified, and when and why the plugin runs it — as required for publishing the plugin on FAB.

Platforms referenced below as `<Platform>` are: `Win64`, `WinArm64`, `Linux`, `LinuxArm64`, `Mac`.

## Versions

To keep this notice accurate without manual upkeep on every dependency bump, exact version numbers are not listed here. Each dependency's version for a given release is recorded in the plugin's `CHANGELOG.md` (included in this package; also published at https://github.com/getsentry/sentry-unreal/releases) under each release's `### Dependencies` heading. Since a release lists only the dependencies it changed, a dependency's current version is its most recent entry there at or before the plugin version in `Sentry.uplugin` (`VersionName`).

## Executables

### `crashpad_handler`, `crashpad_handler.exe`

- **Location:** `Source/ThirdParty/<Platform>/Crashpad/bin/`
- **Source:** [getsentry/sentry-native](https://github.com/getsentry/sentry-native) (vendors Google Crashpad)
- **Version:** see the "Native SDK" entry under `### Dependencies` in `CHANGELOG.md`
- **Modified:** No — unmodified upstream release build
- **Purpose / when it runs:** Out-of-process crash handler. Spawned by the SDK **at runtime on the player's machine** when the Crashpad backend is enabled; it monitors the game process and writes a minidump when the game crashes. Running out-of-process is required by design — the handler must outlive the crashing process.

### `crashpad_wer.dll`

- **Location:** `Source/ThirdParty/{Win64,WinArm64}/Crashpad/bin/`
- **Source:** [getsentry/sentry-native](https://github.com/getsentry/sentry-native)
- **Version:** see the "Native SDK" entry under `### Dependencies` in `CHANGELOG.md`
- **Modified:** No — unmodified upstream release build
- **Purpose / when it runs:** Windows Error Reporting helper module loaded by `crashpad_handler` to capture certain Windows crash types. Runtime, on the player's machine.

### `sentry-crash`, `sentry-crash.exe`

- **Location:** `Source/ThirdParty/<Platform>/Native/bin/`
- **Source:** [getsentry/sentry-native](https://github.com/getsentry/sentry-native)
- **Version:** see the "Native SDK" entry under `### Dependencies` in `CHANGELOG.md`
- **Modified:** No — unmodified upstream release build
- **Purpose / when it runs:** The sentry-native out-of-process crash handler. Serves the same role as `crashpad_handler`, but is used when the **native** backend is selected instead of Crashpad. Runtime, on the player's machine.

### `sentry-wer.dll`

- **Location:** `Source/ThirdParty/{Win64,WinArm64}/Native/bin/`
- **Source:** [getsentry/sentry-native](https://github.com/getsentry/sentry-native)
- **Version:** see the "Native SDK" entry under `### Dependencies` in `CHANGELOG.md`
- **Modified:** No — unmodified upstream release build
- **Purpose / when it runs:** Windows Error Reporting helper module loaded when the **native** backend is used — the native-backend counterpart to `crashpad_wer.dll` — to capture certain Windows crash types. Runtime, on the player's machine.

### `sentry-cli-Windows-x86_64.exe`, `sentry-cli-Linux-x86_64`, `sentry-cli-Darwin-universal`

- **Location:** `Source/ThirdParty/CLI/`
- **Source:** [getsentry/sentry-cli](https://github.com/getsentry/sentry-cli)
- **Version:** see the "CLI" entry under `### Dependencies` in `CHANGELOG.md`
- **Modified:** No — unmodified upstream release build
- **Purpose / when it runs:** Uploads debug information files (symbols) to Sentry so crash reports can be symbolicated. **Build-time only, on the developer's / CI machine** — invoked by the post-build scripts during packaging. It does not ship in, and never executes in, the packaged game.

### `Sentry.CrashReporter.exe`, `Sentry.CrashReporter.app`, `Sentry.CrashReporter`

- **Location:** `Source/ThirdParty/<Platform>/`
- **Source:** [getsentry/sentry-desktop-crash-reporter](https://github.com/getsentry/sentry-desktop-crash-reporter)
- **Version:** see the "Crash Reporter" entry under `### Dependencies` in `CHANGELOG.md`
- **Modified:** No — unmodified upstream release build
- **Purpose / when it runs:** Optional desktop crash-reporter UI shown to the end user after a crash (to confirm upload / collect feedback). Launched by the SDK **at runtime only if the developer enables the crash-reporter UI**. The macOS `.app` bundle also contains this application's own .NET and Skia/Uno runtime libraries (`.dylib`); those are part of this single application, not separate tools.

## Scripts (first-party, build-time only)

### `post-build-steps-win.bat`, `post-build-steps-linux.sh`, `post-build-steps-mac.sh`

- **Location:** `Scripts/`
- **Source:** Authored by Sentry (first-party); version matches the plugin (`Sentry.uplugin` `VersionName`)
- **Modified:** N/A — first-party
- **Purpose / when it runs:** Registered as `PreBuildSteps` / `PostBuildSteps` in `Sentry.uplugin`, so Unreal Build Tool runs them automatically after the plugin is built — **on the developer's or CI machine, never inside a shipped title**. They (1) copy the crash-handler / crash-reporter binaries into `Binaries/<Platform>/` (Epic's Fab pre-build step strips "extra" binaries out of `Source/ThirdParty`, so they are restored to the location the packaged plugin expects); (2) `chmod +x` the bundled binaries, because some distribution channels do not preserve the executable bit; (3) upload debug symbols to Sentry via `sentry-cli`.
