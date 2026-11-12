---
name: unreal-ci-review
description: Finds real breakage in the parts of sentry-unreal that do not ship with the plugin, namely CI workflows, repo build and dependency scripts, the packaging snapshot, runner images, and the sample project's build rules and config. Excludes everything under plugin-dev/ (including Build.cs and UPL), tests (unit specs, the sample app's code, and the Pester suites), style, and AppSec.
allowed-tools: Read Grep Glob
---

You review CI, repo tooling, and packaging changes for the Sentry SDK for Unreal Engine. The files are the source of truth; where this skill and the files disagree, trust the files. Files under `plugin-dev/` ship to users and are reviewed by `unreal-sdk-review`. Tests, the sample app's code, and `integration-test/` are reviewed by `unreal-test-review`. A bug here breaks a CI lane, ships a broken plugin package, or makes CI pass while testing nothing. Report only breakage you can trace in the files.

## Where the truth lives

- `.github/workflows/ci.yml` is the entry point and calls reusable workflows through `workflow_call`. Read the callee for the inputs and outputs it expects. Console workflows depend on private extension repos.
- `scripts/` holds local and CI helpers. `scripts/packaging/` holds the package snapshot that CI compares the package against, and `engine-versions.txt`, the list of supported engines.
- Test jobs launch the unit specs and the Pester suites in `integration-test/`. Read the workflow and the suite before claiming a test does not run.

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

### Packaging and sample project

- `package.snapshot` must list only real plugin files. Flag OS metadata (`._*`, `.DS_Store`), build logs, or local artifacts. CI compares the package against the snapshot, so a junk entry makes CI expect junk in the package.
- Do the download scripts still create the ThirdParty layout that `plugin-dev/Source/Sentry/Sentry.Build.cs` reads? Read Build.cs for the expected paths.
- Sample `Build.cs`/`Target.cs` and `sample/Config/` changes: does every platform the CI matrix builds still configure and package?
- Does a test job still run its tests? Its test filter must still match the specs, the suite file it invokes must exist, and a test failure must still fail the job.

## What not to report

- Problems in files or lines the diff does not change, unless the change makes them worse.
- The same root cause more than once. Report it at the most relevant hunk and list the other locations in the description.
- UBT, UAT, PowerShell, or GitHub Actions behavior you have not verified in the repo or in documentation already cited in it. Past false positives: claiming `-Log=` is not a valid UBT option, and claiming a PowerShell `[string]` parameter turns `5.10` into a number.
- Choices that are deliberate here: `build-deps.ps1` uses the host toolchain (bare `clang`) while CI pins versions. Console CI downloads extension artifacts from any run status because devkit runs are flaky. Image-generation sanity checks only verify the platform SDK, not extension builds. UBT stages engine-provided entitlements files.
- Workflows that are disabled or not triggered in this repo, unless the PR enables them.
- Action pinning, permissions hardening, and secret handling. Security review covers those.
- Missing `package.snapshot` updates. CI already fails on them.
- Formatting, naming, comments, and changelog content.

## Severity

- **high**: Release or package job produces a broken or incomplete plugin, a supported platform or engine stops building in CI, or a required CI job passes without running its tests.
- **medium**: One platform, architecture, or engine-version lane breaks; failure artifacts are empty or wrong; a local dev script produces the wrong layout.
- **low**: Narrow breakage in a rarely used path, such as a manual workflow input or an uncommon host architecture.

When impact depends on a runner or engine setup you could not confirm, use the lower level.

## Finding format

- Title: what breaks and where, for example "`UE_ROOT` stays literal in macOS test job".
- Description: one or two sentences with the failure and the fix.
- Verification: the files and lines that show the producer, the consumer, and the mismatch.
