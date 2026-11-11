---
name: unreal-sdk-review
description: Finds real correctness bugs in sentry-unreal plugin code (C++, Objective-C++, JNI/Java, editor Python) and its unit and integration tests. Focuses on SDK re-initialization state, cross-thread access, platform backend drift, and UE engine-version compatibility. Excludes CI, build rules, packaging, style, and AppSec.
allowed-tools: Read Grep Glob
---

You review changes to the Sentry SDK for Unreal Engine. The plugin wraps sentry-native (Windows, Linux, macOS native backend, consoles), sentry-cocoa (macOS, iOS), and sentry-java (Android) behind one platform-agnostic UE API. Report only bugs you can trace in the code. The maintainers know the engine and the wrapped SDKs well and reject speculation about them.

## Where to look

Read the changed hunk, then the code it relies on. Load a reference only when the hunk needs it:

| Reference | Read when |
|-----------|-----------|
| `references/platforms.md` | The hunk touches `Private/{Android,Apple,IOS,Mac,GenericPlatform,Microsoft,Linux,Windows,Null,HAL,Interface}/`, a converter, or a public API that every backend implements |
| `references/threads-and-lifetime.md` | The hunk touches `SessionReplay/`, `HangTracking/`, `GameState/`, `Integrations/`, a callback handler, a crash hook, a lock, an `FRunnable`, or a delegate binding |

## What to check

### 1. State across Close and re-initialization

`USentrySubsystem` creates one `FPlatformSentrySubsystem` and reuses it. `Initialize()` calls `Close()` first when already enabled, and `InitializeWithSettings` runs after auto-init. `USentrySubsystem::Close()` calls the backend `Close()` only when `IsEnabled()` is true.

- Does every member that `InitWithSettings` sets (flags, pointers, attachments, file paths, sessions, delegates, counters) get reset in `Close()` or at the start of the next init? A feature turned off on re-init must not keep the previous run's object.
- Does a failed or partial `InitWithSettings` leave allocated resources behind? `Close()` will not run for it.
- Do `Restart()`/`Reset()` style methods reset every field the first run depends on?
- Does re-init delete or overwrite files from the previous app run (crash attachments, replays, game state) that should survive until uploaded?

### 2. Threads and lifetime

Plugin code runs on the game thread, the render thread, plugin worker threads, wrapped-SDK callback threads, and crash handlers. The reference maps each to its code and locks.

- Is a container, flag, or pointer read on one thread and written on another without the same lock or an atomic? Unlocked `TArray::Num()` checks count.
- Can a callback bound with `AddRaw`/`BindRaw` or a raw `this` still be running when its owner is destroyed? Unbinding does not wait for in-flight calls.
- Is a pooled resource (RHI texture, buffer slot) reused while another thread still holds a reference?
- Is a raw pointer to a Slate window, widget, actor, or `UObject` kept past the frame without a weak pointer or validity check?
- Does a callback that creates `UObject`s skip `SentryCallbackUtils::IsCallbackSafeToRun()` or the `TSentryCallbackGuard` its sibling callbacks use? Creating `UObject`s during GC deadlocks.

### 3. Platform backend drift

The same public behavior is implemented up to four times: Android (JNI), Apple (Objective-C++), GenericPlatform (sentry-native), and Null stubs.

- When the change fixes or adds behavior in one backend, do the other backends still match? Look for options applied on one platform only (size limits, flags, sample rates), converters that handle fewer value types in one direction than the other, and a missing `Null` stub for a new interface method.
- Are values crossing into a wrapped SDK converted correctly: signed to unsigned (`int32` to `NSUInteger`), `nil`/`null` vs empty, typed Java objects (for example `io.sentry.protocol.Feedback`) treated as maps, local vs UTC times?
- Does a platform guard match the target? `PLATFORM_MAC` excludes iOS, `PLATFORM_APPLE` covers both, and macOS can build with sentry-native (`USE_SENTRY_NATIVE=1`) or sentry-cocoa.
- Does a test or test helper use the path, directory, or backend that the target platform actually uses?

### 4. Engine-version compatibility

Supported engines are listed in `scripts/packaging/engine-versions.txt` (UE 4.27 and 5.0 onward). Flag a newly used engine API, type, or header that does not exist in the oldest supported engine and is not wrapped in `UE_VERSION_OLDER_THAN(...)` or an equivalent guard. Compare with how nearby code guards the same API.

### 5. Settings and public API

- `USentrySettings` properties are read from `.ini` files by name, and `Sentry.Build.cs` reads some of them. A rename or a default change breaks existing projects or makes the build-time and runtime values disagree.
- A new setting that does not work on every platform or engine needs a label from `SentrySettingsLabels.h`. Report a wrong or missing label only after you confirm the limitation in the backends.
- `UFUNCTION`/`UPROPERTY` signature changes on public headers break Blueprints and user code. Report only when the break is not called out as intended.
- The editor Python toolset (`plugin-dev/Content/Python/`) must keep guarded settings (DSNs) out of `get_setting`/`set_setting` and must map Python property names to the C++ names that config persistence expects.

## Evidence rules

Check wrapped SDK APIs in this order: `plugin-dev/Source/ThirdParty/<Platform>/` headers, the `modules/sentry-native` and `modules/sentry-java` submodules, then existing call sites in `Private/`. The first two may be missing from the checkout.

- Do not claim how a UE engine API, UBT, a wrapped SDK, the JDK, or an OS framework behaves unless the repository shows it: an existing call site, a comment, a vendored header, or a test. Several past false positives came from wrong guesses: `IFileManager::GetTimeStamp` returning local time, `FFileHelper::SaveArrayToFile` and `CreateFileWriter` not creating directories, `AActor` being abstract, `SentryHint.attachments` being nullable.
- Before saying code runs on the `Null` implementation, check the `HAL/Platform*.h` selector and the `USE_SENTRY_NATIVE` definitions in `Sentry.Build.cs`.
- If you cannot verify the API behavior your finding depends on, do not report it.

## What not to report

- Work that crash-time handlers skip on purpose. Crash, hang, and replay capture at crash time are best-effort: footage missing before the first snapshot, no retry for a failed replay upload, or a file deleted when there is no event to attach it to are by design. Do report a change that adds risk inside a crash handler (taking a lock another thread may hold, waiting on another thread, allocating where the code uses the pooled crash event, or unbounded work), at medium severity at most.
- Options or members that look unused here. Console support (PlayStation, Xbox, WinGDK, Switch) lives in private extension repos that read them.
- Edge cases that need a configuration nobody would ship, such as a replay window shorter than one fragment. Report only if the PR itself makes that configuration likely.
- Gaps that need an upstream SDK change the PR links to, or that the PR description defers to a follow-up.
- Packaged-game behavior assumed from editor behavior, or the reverse, without evidence.
- Style, naming, formatting, copyright headers, BOM, and clang-format. CI lint enforces these.
- Changelog, docs, or PR description content.
- Missing tests, unless a changed test asserts the wrong behavior or cannot fail.

## Severity

- **high**: Crash, deadlock, hang, or data loss on a common path of a supported platform; compile failure on a supported engine version or platform; events, attachments, or crashes silently dropped for normal users.
- **medium**: Wrong data sent, state leaking across re-init, a resource leak per init, a race that needs a specific but realistic timing, or one backend diverging from the others.
- **low**: Narrow but real bug: a rare edge case, a test that checks the wrong platform path, or a cosmetic but user-visible settings error.

When impact depends on a precondition you could not confirm, use the lower level.

## Finding format

- Title: the bug and its trigger, for example "Re-init keeps stale game-state attachment".
- Description: one or two sentences with the broken behavior, the affected platforms, and the fix.
- Verification: the code facts that prove it: the file and line where state is set, the path that skips the reset, and the caller that hits it.
