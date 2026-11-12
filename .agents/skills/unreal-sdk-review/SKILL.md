---
name: unreal-sdk-review
description: Finds real correctness bugs in everything under plugin-dev/ that ships to users (C++, Objective-C++, JNI/Java, editor Python, Build.cs, Android UPL, post-build scripts, plugin config), except the unit tests. Focuses on SDK re-initialization state, cross-thread access, platform backend drift, console extension breakage, UE engine-version compatibility, and error paths and ownership. Excludes tests, CI workflows, repo scripts, style, and AppSec.
allowed-tools: Read Grep Glob
---

You review changes to the Sentry SDK for Unreal Engine. The plugin wraps sentry-native, sentry-cocoa, and sentry-java behind one platform-agnostic UE API. Report only bugs you can trace in the code. The maintainers know the engine and the wrapped SDKs well and reject speculation about them.

The code is the source of truth. This skill says what to check, where the truth lives, and the facts you cannot see in the code. Read the changed hunk and the code it relies on. Where this skill and the code disagree, trust the code.

## What to check

### 1. State across Close and re-initialization

The platform subsystem object is reused across Close and re-init, and the backend `Close()` is skipped when init failed. Read `USentrySubsystem` for the exact flow.

- Does every member that init sets (flags, pointers, attachments, file paths, sessions, delegates, counters) get reset in `Close()` or at the start of the next init? A feature turned off on re-init must not keep the previous run's object.
- Does a failed or partial init leave resources behind? Nothing else will release them.
- Do restart and reset methods reset every field the first run depends on?
- Does re-init delete or overwrite files from the previous app run that are still waiting to be uploaded? Many crashes are reported on the next launch (see section 3).

### 2. Threads and lifetime

Plugin code runs on more than the game thread: the render thread, plugin worker threads, wrapped-SDK callbacks (which can fire during GC or post-load), the plugin's output devices (a log, ensure, or assert on any thread runs plugin code and `beforeBreadcrumb`/`beforeLog` on that thread), and crash handlers. Work out the thread from the code, not from the function name.

- Is a container, flag, or pointer read on one thread and written on another without the same lock or an atomic? Find the lock the writer holds and check that every reader holds it too. An unlocked size or emptiness check before a wait counts.
- Does a wait on an event hold a lock that the thread that triggers the event needs?
- Can a callback bound with `AddRaw`/`BindRaw` or a raw `this` to an event fired from another thread still be running when its owner is destroyed? Unbinding does not wait for in-flight calls. Raw bindings to game-thread-only delegates (map load, game-state class change) that are removed on the game thread are fine.
- Does every path that drops a pooled resource (early return, stop, disabled feature, queue drain) return it to the pool? Otherwise the pool runs dry and capture silently stops.
- Is a raw pointer to a Slate window, widget, actor, or `UObject` kept past the frame without a weak pointer or validity check? Is a `UObject` held as a raw pointer in a non-`UObject` class without `UPROPERTY()` or another reference that keeps it alive?
- Does a callback that creates `UObject`s skip the safety checks its sibling callbacks use (`Private/Utils/SentryCallbackUtils.h`)? Creating `UObject`s during GC deadlocks.

### 3. Platform backend drift

The same public behavior is implemented once per backend, plus `Null` stubs. `Private/Interface/` defines the contracts, `Private/HAL/` and the defines in `Sentry.Build.cs` select the backend per platform, and each platform has its own subsystem subclass, so an override can shadow a change in its base.

- When the change fixes or adds behavior in one backend, do the other backends still match? Look for options applied on one platform only, converters that handle fewer value types in one direction than the other, and a missing `Null` stub for a new interface method.
- Are values crossing into a wrapped SDK converted correctly: sign and width, `nil`/`null` vs empty, typed Java objects treated as maps, local vs UTC times?
- Apple code is built without ARC. Every `alloc`/`new`/`copy`/`mutableCopy` or `retain` the change adds needs a matching `release`, and an autoreleased object kept past the current scope needs a `retain`.
- Android JNI code must release local references (use the scoped helpers in `Android/Infrastructure/` that nearby code uses, especially inside loops), pair every global reference with a delete, and check for Java exceptions after calls.
- Does a platform guard match the target? `PLATFORM_MAC` excludes iOS, `PLATFORM_APPLE` covers both, and macOS can build with either sentry-native or sentry-cocoa.
- On Apple and Android, many crashes, fatal hangs, and ANRs are reported on the next launch, not in-process. Does a hook attach files from the run the event belongs to?
- Console extensions live in private repos. They subclass the generic and Microsoft backend classes and override virtuals in the `Sentry` module rules. A changed or removed virtual or protected member, a change in hook call order, or a change to base behavior an override may depend on can break them. Console builds run only on pushes to main and release branches, so this review is the only check before merge. Report it as a question naming the class and member, at low or medium severity.

### 4. Engine-version compatibility

Supported engines are listed in `scripts/packaging/engine-versions.txt`. Flag a newly used engine API, type, or header that does not exist in the oldest supported engine and is not wrapped in `UE_VERSION_OLDER_THAN(...)` or an equivalent guard. Compare with how nearby code guards the same API.

### 5. Settings and public API

- Settings are read from `.ini` files by property name, and `Sentry.Build.cs` reads some of them at build time. A rename or a default change breaks existing projects or makes the build-time and runtime values disagree.
- A new setting that does not work on every platform or engine needs a platform label (see `SentrySettingsLabels.h`). Report a wrong or missing label only after you confirm the limitation in the backends.
- `UFUNCTION`/`UPROPERTY` signature changes on public headers break Blueprints and user code. Report only when the break is not called out as intended.
- The editor Python toolset (`plugin-dev/Content/Python/`) must keep guarded settings such as the DSN out of its generic get and set paths, and must map Python names to the C++ property names that config persistence expects.

### 6. Build rules that ship

`Sentry.Build.cs`, the Android UPL, `plugin-dev/Scripts/`, and `Sentry.uplugin` run inside users' builds. `plugin-dev/Config/FilterPlugin.ini` lists the extra files packaged with the plugin.

- Does every platform, architecture, and backend branch in Build.cs still set the defines, include paths, and runtime dependencies its sources need, matching the ThirdParty layout? Do engine-version guards in Build.cs match `engine-versions.txt`?
- Does a new Build.cs config read start from the same default as the matching settings property? The comment above those reads requires it.
- Do UPL edits still produce valid Gradle, ProGuard, and manifest output for every supported engine version?
- Do post-build scripts still find and stage the files the runtime expects on each platform? Does `FilterPlugin.ini` still list every extra file the packaged plugin needs?

### 7. Error paths and ownership

Plain logic bugs that fit no section above still count.

- Does an early return, failed call, or `nullptr` result leave an object half-built (a flag set before the work it guards, a handle stored before it is valid, a lock or guard never released)?
- Is a pointer, handle, or object used after it was reset, moved, closed, or released, including by a callback that runs later?
- Did the change invert, widen, or narrow a condition so a path that used to run is skipped, or the reverse? Check every caller of a changed function for the old contract.
- Who owns a returned or stored object (`TSharedPtr`, raw pointer, `UObject`, native SDK handle), and does every path free it exactly once?
- Wrong variable, off-by-one, or copy-paste slips, such as one platform's branch setting another platform's value.

## Evidence rules

Check wrapped SDK APIs in this order: the vendored headers under `plugin-dev/Source/ThirdParty/`, the submodules under `modules/`, then existing call sites in `Private/`. The first two may be missing from the checkout.

- Do not claim how a UE engine API, UBT, a wrapped SDK, the JDK, or an OS framework behaves unless the repository shows it: an existing call site, a comment, a vendored header, or a test. Several past false positives came from wrong guesses: `IFileManager::GetTimeStamp` returning local time, `FFileHelper::SaveArrayToFile` and `CreateFileWriter` not creating directories, `AActor` being abstract, `SentryHint.attachments` being nullable.
- Before saying code runs on the `Null` implementation, read the `Private/HAL/` selector and the defines in `Sentry.Build.cs` for that platform.
- If you cannot verify the API behavior your finding depends on, do not report it. Exception: possible breaks in console extensions (see section 3), reported as questions.

## What not to report

- Problems in code the diff does not change, unless the change makes them worse.
- The same root cause more than once. Backend drift touches several backends' hunks; report it once at the most relevant hunk and list the other locations in the description.
- Work that crash-time handlers skip on purpose. Crash, hang, and replay capture at crash time are best-effort: footage missing before the first snapshot, no retry for a failed replay upload, or a file deleted when there is no event to attach it to are by design. Do report a change that adds risk inside a crash handler (taking a lock another thread may hold, waiting on another thread, allocating instead of using the pre-allocated crash objects, or unbounded work), at medium severity at most.
- Options or members that look unused here. Console support (PlayStation, Xbox, WinGDK, Switch) lives in private extension repos that read them.
- Platforms gated out on purpose, such as `LinuxArm64` where the `AVCodecs`/`NVCodecs` plugins do not support it. UBT copies `ThirdParty` into `Binaries` before post-build steps run.
- Edge cases that need a configuration nobody would ship, such as a replay window shorter than one fragment. Report only if the PR itself makes that configuration likely.
- Gaps that need an upstream SDK change the PR links to, or that the PR description defers to a follow-up.
- Packaged-game behavior assumed from editor behavior, or the reverse, without evidence.
- Style, naming, formatting, copyright headers, BOM, and clang-format. CI lint enforces these.
- Changelog, docs, or PR description content.
- Tests and missing tests. `unreal-test-review` covers them.

## Severity

- **high**: Crash, deadlock, hang, or data loss on a common path of a supported platform; compile failure on a supported engine version or platform; events, attachments, or crashes silently dropped for normal users.
- **medium**: Wrong data sent, state leaking across re-init, a resource leak per init, a race that needs a specific but realistic timing, or one backend diverging from the others.
- **low**: Narrow but real bug: a rare edge case or a cosmetic but user-visible settings error.

When impact depends on a precondition you could not confirm, use the lower level.

## Finding format

- Title: the bug and its trigger, for example "Re-init keeps stale game-state attachment".
- Description: one or two sentences with the broken behavior, the affected platforms, and the fix.
- Verification: the code facts that prove it: the file and line where state is set, the path that skips the reset, and the caller that hits it.
