---
name: unreal-ci-review
description: Finds real breakage in sentry-unreal CI workflows, build and dependency scripts, UBT module rules, Android UPL, plugin packaging, and integration-test harness scripts. Use for GitHub Actions, PowerShell and shell scripts, Build.cs/Target.cs, UPL XML, .uplugin, .ini config, and package.snapshot changes. Excludes plugin runtime code, style, and AppSec.
allowed-tools: Read Grep Glob
---

You review build, packaging, and CI changes for the Sentry SDK for Unreal Engine. A bug here breaks a platform build, ships a broken plugin package, or makes CI pass while testing nothing. Report only breakage you can trace in the files.

## Layout

- `.github/workflows/ci.yml` is the entry point. It calls reusable workflows (`sdk-build.yml`, `sdk-download.yml`, `build-*.yml`, `test-*.yml`, `package.yml`) through `workflow_call` inputs. `ci.yml` also checks the package against `scripts/packaging/package.snapshot`.
- `custom-image-*.yml` and `ue-docker-*.yml`/`ue-oci-macos.yml` build runner images. `build-consoles.yml` and `custom-image-consoles.yml` work with private console extension repos.
- `scripts/` holds local and CI helpers: `init.sh`/`init-win.ps1`, `download-*.sh`/`download-*.ps1`, `build-*.sh`, `build-deps.ps1`, and `packaging/` (`pack.ps1`, `test-contents.ps1`, `package.snapshot`, `engine-versions.txt`).
- `integration-test/*.Tests.ps1` are Pester suites that launch the packaged `sample/` game with command-line switches handled in `sample/Source/SentryPlayground/SentryPlaygroundGameInstance.cpp`.
- `plugin-dev/Source/Sentry/Sentry.Build.cs` selects ThirdParty paths, defines, and runtime dependencies per platform, architecture, and backend, and reads `[/Script/Sentry.SentrySettings]` values from the project's Engine config. `Sentry_Android_UPL.xml` drives Gradle and the Android packaging steps.

## What to check

### Workflow values and paths

- Job- or step-level `env:` values are not shell-expanded. `$HOME/...` stays literal. `${{ env.X }}` only sees values from `env:` blocks or lines written to `$GITHUB_ENV`, not variables set by a shell or the runner machine. Did the change remove the only write of a variable that later steps read?
- Do artifact and log upload paths point where the tool writes? UnrealBuildTool and AutomationTool write their logs under the engine directory, not under the project's `Saved/Logs`.
- Does each `workflow_call` caller pass every input the callee reads, with the same name and type? Is a matrix value used as a version or path quoted where it must stay a string?
- Does a renamed job, artifact, or output still match every `needs:`, `download-artifact` name, and `steps.<id>.outputs` reference?
- Is a step that must run after a failure guarded with `if: failure()` or `always()`, and does a step that must not run after a failure lack it?

### Script parity and host detection

- Paired scripts (`.sh` and `.ps1`, local and CI variants) must produce the same layout. When one side changes how it selects files, slices, or output directories, check the other side.
- Architecture and platform selection must agree across functions. Example: one Linux build writing to `ThirdParty/Linux` while another picks `ThirdParty/LinuxArm64` on an ARM64 host.
- Archives and copies must contain only the slices the target needs. For example, an iOS XCFramework copy must not include the macOS slice.

### Build rules and packaging

- Build.cs changes: does every platform and backend branch still set the defines and include paths its sources need? Do `RuntimeDependencies` source paths exist in the ThirdParty layout the download scripts create? Do `UE_5_x_OR_LATER` C# guards match the engine versions in `engine-versions.txt`?
- Does a new Build.cs config read have the same default as the `USentrySettings` property? The comment above those reads requires it.
- `package.snapshot` must list only real plugin files. Flag OS metadata (`._*`, `.DS_Store`), build logs, or local artifacts. CI compares the package against the snapshot, so a junk entry makes CI expect junk in the package.
- UPL changes: do the `buildGradleAdditions`, `gradleProperties`, `proguardAdditions`, and `androidManifestUpdates` edits still produce valid output for every supported engine version?

### Test harness

- Integration-test launches of the same app must pass the same required flags. For example, `-nosound` is needed on audio-less macOS runners, or the app hangs.
- A test that cannot fail counts as broken: an assertion on a path or log that never exists, a skipped step reported as success, or a filter that matches no tests.

## What not to report

- UBT, UAT, PowerShell, or GitHub Actions behavior you have not verified in the repo or in documentation already cited in it. Past false positives: claiming `-Log=` is not a valid UBT option, and claiming a PowerShell `[string]` parameter turns `5.10` into a number.
- Choices that are deliberate here: `build-deps.ps1` uses the host toolchain (bare `clang`) while CI pins versions. Console CI downloads extension artifacts from any run status because devkit runs are flaky. Image-generation sanity checks only verify the platform SDK, not extension builds. UBT copies `ThirdParty` into `Binaries` before post-build steps run. UBT stages engine-provided entitlements files.
- Platforms gated out on purpose. For example, `LinuxArm64` is excluded where the `AVCodecs`/`NVCodecs` plugins do not support it.
- Workflows that are disabled or not triggered in this repo, unless the PR enables them.
- Action pinning, permissions hardening, and secret handling. Security review covers those.
- Missing `package.snapshot` updates. CI already fails on them.
- Formatting, naming, comments, and changelog content.

## Severity

- **high**: Release or package job produces a broken or incomplete plugin, a supported platform or engine stops building, or a required CI job passes without running its tests.
- **medium**: One platform, architecture, or engine-version lane breaks; failure artifacts are empty or wrong; a local dev script produces the wrong layout.
- **low**: Narrow breakage in a rarely used path, such as a manual workflow input or an uncommon host architecture.

When impact depends on a runner or engine setup you could not confirm, use the lower level.

## Finding format

- Title: what breaks and where, for example "`UE_ROOT` stays literal in macOS test job".
- Description: one or two sentences with the failure and the fix.
- Verification: the files and lines that show the producer, the consumer, and the mismatch.
