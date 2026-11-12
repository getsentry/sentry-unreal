---
name: unreal-test-review
description: Finds tests in sentry-unreal that cannot fail, test the wrong thing, or hang. Covers the C++ unit specs under plugin-dev/Source/*/Private/Tests/, the SentryPlayground sample app under sample/Source/ (integration test classes, hooks, game instance), and the Pester suites under integration-test/. Checks that the C++ and Pester halves of each integration test stay in sync, along with platform and timing assumptions, launch flags, cleanup, and leftover debugging. Excludes SDK runtime code, CI workflows, the sample's Build.cs/Target.cs, style, and AppSec.
allowed-tools: Read Grep Glob
---

You review test changes in the Sentry SDK for Unreal Engine. A bug here is a test that passes while the SDK is broken, fails or hangs while the SDK works, or checks a different platform path than the one it claims to cover. Report only problems you can trace in the files.

The code is the source of truth. This skill says what to check, where the truth lives, and the facts you cannot see in the code. Where this skill and the code disagree, trust the code.

## Where the truth lives

- **Unit specs** live in `plugin-dev/Source/*/Private/Tests/`. Before claiming where or whether a spec runs, read the `RunTests` launches in `.github/workflows/test-*.yml` and in the Pester suites instead of assuming the editor.
- **Sample app** lives in `sample/Source/SentryPlayground/`. Its game instance maps each command-line switch to a test class in `IntegrationTests/`. The app sets up a shared test context, and its hooks change or drop event data before it is sent. Tests print markers that the Pester suites parse.
- **Pester suites** are `integration-test/Integration.*.Tests.ps1`, one per platform family. They launch the packaged sample, parse its output, and fetch the captured data from Sentry.

## What to check

### 1. Assertions that cannot fail

- Does an assertion check a path, log line, tag, or field that the code never produces, so it passes vacuously or compares empty with empty?
- Does a test compare a value with itself, or with a value the test wrote, instead of the SDK's output?
- Is a skipped or early-returned step reported as success? Does a test case contain no assertion?
- Does the CI filter never run a new or renamed spec, or does a compile guard exclude it?
- Is a skip condition broader than the platform or backend that lacks the feature, hiding a lane that should run the check?

### 2. C++ and Pester in sync

- Does every switch a suite passes map to a test class registered in the game instance?
- Before judging a Pester assertion, read the matching test class, the shared test context, and the hooks. Every asserted tag, message, attachment, marker, or exit code must be something the app actually produces on that platform.
- When a marker, tag, or test name changes on one side, do the other side and every platform suite still match?

### 3. Platform and timing assumptions

- Does a test or test helper use the path, directory, or backend that the target platform actually uses? Past real bug: an iOS replay test staging clips in the macOS directory. `PLATFORM_MAC` excludes iOS, `PLATFORM_APPLE` covers both, and macOS runs both the native and the Cocoa backend.
- A crash test must get its marker out before the process dies. Does a change drop the flush or print path that sibling crash tests use on that platform, or move work after the crash trigger?
- Does a change break a platform- or engine-specific workaround that sibling tests keep (look for `#if PLATFORM_*` and `UE_VERSION_OLDER_THAN` blocks around the markers)?
- Does an assertion depend on timing the code does not guarantee, such as event order, or a fixed wait shorter than the work it waits for?

### 4. Launch flags and cleanup

- Does every launch of the same app pass the suite's shared required arguments, including relaunches that flush a crash or replay? Past real bug: a launch without `-nosound` hangs on audio-less macOS runners.
- Does every path through a test class, including failures, end in the base class's completion call or the intended crash? Otherwise the app never exits and the suite times out. An async test must keep itself alive until it completes.
- Does a test leave files, settings, or app state behind that a later test or relaunch reads, and does suite teardown still disconnect what setup connected?

### 5. Leftover debugging

- Temporarily disabled or narrowed tests (`xIt`, a hard-coded skip, a commented-out assertion, a narrowed test filter), local paths, real DSNs or tokens, or debug output added only to investigate a failure.

## Evidence rules

- Do not claim how a UE engine API, Pester, PowerShell, or a wrapped SDK behaves unless the repository shows it: an existing call site, a comment, or a test. Past false positives: `AActor` being abstract, `CreateFileWriter` not creating directories.
- Before saying a test runs on the `Null` implementation, read the `Private/HAL/` selector and the defines in `Sentry.Build.cs` for that platform.
- If you cannot verify the behavior your finding depends on, do not report it.

## What not to report

- Problems in code the diff does not change, unless the change makes them worse.
- The same root cause more than once. Report it at the most relevant hunk and list the other locations in the description.
- Missing tests for new SDK behavior, unless the PR says it tests that behavior.
- Bugs in SDK runtime code or CI workflows. Other skills review those.
- Style, naming, formatting, and comments.

## Severity

- **high**: A required test lane passes while the behavior it covers is broken, or a suite hangs or fails on every run of a supported platform.
- **medium**: One platform, backend, or engine-version lane asserts the wrong thing or skips a check it should run.
- **low**: A narrow gap, such as one weak assertion next to stronger ones, or leftover debug output.

When impact depends on a runner or platform setup you could not confirm, use the lower level.

## Finding format

- Title: what the test misses and where, for example "iOS replay test stages clips in the macOS directory".
- Description: one or two sentences with why the test cannot catch the bug or why it fails, and the fix.
- Verification: the files and lines that show the producer, the assertion, and the mismatch.
