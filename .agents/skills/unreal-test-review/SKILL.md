---
name: unreal-test-review
description: Finds tests in sentry-unreal that cannot fail, test the wrong thing, or hang. Covers the C++ unit specs under plugin-dev/Source/*/Private/Tests/, the SentryPlayground sample app under sample/Source/ (integration test classes, hooks, game instance), and the Pester suites under integration-test/. Checks that the C++ and Pester halves of each integration test stay in sync, along with platform and timing assumptions, launch flags, cleanup, and leftover debugging. Excludes SDK runtime code, CI workflows, the sample's Build.cs/Target.cs, style, and AppSec.
allowed-tools: Read Grep Glob
---

You review test changes in the Sentry SDK for Unreal Engine. A bug here is a test that passes while the SDK is broken, fails or hangs while the SDK works, or checks a different platform path than the one it claims to cover. Report only problems you can trace in the files.

## Layout

- **Unit specs**: `plugin-dev/Source/{Sentry,SentryEditor}/Private/Tests/*.spec.cpp`. UE automation specs (`BEGIN_DEFINE_SPEC`) inside `#if WITH_AUTOMATION_TESTS`, with flags from `SentryTests.h`. CI runs them inside the packaged sample, not the editor, with `Automation RunTests Sentry` on all five platforms: Linux, macOS, and Windows in `test-*.yml`, and Android and iOS in RUN 0 of their Pester suites. Only specs whose name starts with `Sentry` run, and each spec must pass in a packaged game build on every one of those platforms.
- **Sample app**: `USentryPlaygroundGameInstance::CheckForPendingIntegrationTest` maps each command-line switch (for example `-crash-capture`) to an `FSentryBaseIntegrationTest` subclass in `sample/Source/SentryPlayground/IntegrationTests/`. `ConfigureTestContext` sets the user, tags, and contexts on every run, and `Hooks/` holds the before-send style handlers that edit or drop data. Tests print markers (`EVENT_CAPTURED:`, `TEST_RESULT:`, `TRACE_CAPTURED:` and similar), and `CompleteWithResult` prints `TEST_RESULT` and exits the app.
- **Pester**: `integration-test/Integration.{Desktop,Android,iOS}.Tests.ps1` launch the packaged sample with switches, parse the markers from its output, and fetch the captured data from Sentry (`Get-SentryTestEvent`, `Get-SentryTestLog`, and so on). `It ... -Skip:(...)` conditions turn off assertions per platform or backend.

## What to check

### 1. Assertions that cannot fail

- Does an assertion check a path, log line, tag, or field that the code never produces, so it passes vacuously or checks `$null` against `$null`?
- Does a test compare a value with itself, or with a value the test wrote, instead of the SDK's output?
- Is a skipped or early-returned step reported as success? Does an `It` block lack a `Should`, or does a spec `It` lack a `TestEqual`/`TestTrue` style check?
- Is a spec outside `WITH_AUTOMATION_TESTS` or named so `Automation RunTests Sentry` never runs it?
- Is a `-Skip:` condition broader than the platform or backend that lacks the feature, hiding a lane that should run it?

### 2. C++ and Pester in sync

- Does every switch the Pester suite passes match a key in `CheckForPendingIntegrationTest`, and is every new test class registered there?
- Before judging a Pester assertion, read the matching class in `IntegrationTests/`, plus `ConfigureTestContext` and `Hooks/` for tags, contexts, and removals. Every asserted tag, message, attachment, marker, or exit code must be something the app actually produces on that platform.
- When a marker, tag, or test name changes on one side, do the other side and all three platform suites (Desktop, Android, iOS) still match? Event IDs are formatted with `FormatEventIdWithHyphens` before printing.

### 3. Platform and timing assumptions

- Does a test or test helper use the path, directory, or backend that the target platform actually uses? Past real bug: an iOS replay test staging clips in the macOS directory. Check `PLATFORM_MAC` (macOS only) vs `PLATFORM_APPLE` (macOS and iOS), and that macOS runs both the native and the Cocoa backend (`test.native_backend` tag).
- Crash tests must get their marker out before the process dies: `GLog->Flush()`, the `LowLevelOutputDebugStringf` path for the native backend on UE 4.27 Linux, and the sleep on Android and iOS. Does a change drop one of these or move work after the crash trigger?
- Does a change break the UE 4.27 Linux workaround for duplicated log lines (`Log` instead of `Display` verbosity for markers)?
- Does an assertion depend on timing the code does not guarantee, such as event order, or a fixed wait shorter than the work it waits for?

### 4. Launch flags and cleanup

- Do all launches of the same app pass the same required flags, including relaunches that flush a crash or replay? The shared desktop arguments include `-nullrhi`, `-unattended`, and `-nosound`. A launch without `-nosound` hangs on audio-less macOS runners.
- Does every path through a test class, including failures, end in exactly one `CompleteWithResult` or the intended crash? Otherwise the app never exits and the suite times out. Async tests must keep themselves alive with `AsShared()`.
- Does a test leave files, settings, or app state behind that a later test or relaunch reads, and do `AfterAll` blocks still disconnect devices and the Sentry API?

### 5. Leftover debugging

- Temporarily disabled or narrowed tests (`xIt`, a hard-coded `-Skip`, a commented-out assertion, a narrowed test filter), local paths, real DSNs or tokens, or debug output added only to investigate a failure.

## Evidence rules

- Do not claim how a UE engine API, Pester, PowerShell, or a wrapped SDK behaves unless the repository shows it: an existing call site, a comment, or a test. Past false positives: `AActor` being abstract, `CreateFileWriter` not creating directories.
- Before saying a test runs on the `Null` implementation, check the `HAL/Platform*.h` selector and the `USE_SENTRY_NATIVE` definitions in `Sentry.Build.cs`.
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
