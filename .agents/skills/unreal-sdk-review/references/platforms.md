# Platform backends

## Selection

`Private/HAL/PlatformSentry*.h` picks one implementation per entity (event, scope, hint, metric, ...) at compile time:

```
PLATFORM_ANDROID                       -> Android/   (sentry-java via JNI)
PLATFORM_APPLE && !USE_SENTRY_NATIVE   -> Apple/     (sentry-cocoa, Objective-C++)
USE_SENTRY_NATIVE                      -> GenericPlatform/ (sentry-native)
otherwise                              -> Null/      (stubs)
```

`Sentry.Build.cs` sets `USE_SENTRY_NATIVE`:

- `1` on Windows, Linux, and macOS when `UseNativeBackend` is on.
- `0` on iOS, Android, and macOS with the default cocoa backend.

The subsystem is the exception: `HAL/PlatformSentrySubsystem.h` includes `COMPILED_PLATFORM_HEADER(SentrySubsystem.h)`, so each platform has its own subclass. `FMicrosoftSentrySubsystem` and `FLinuxSentrySubsystem` derive from `FGenericPlatformSentrySubsystem`, `FWindowsSentrySubsystem` from `FMicrosoftSentrySubsystem`, `FIOSSentrySubsystem` from `FAppleSentrySubsystem`, and `FMacSentrySubsystem` from either base depending on `USE_SENTRY_NATIVE`. A virtual added or changed in a base can be shadowed by an override. Console extensions (private repos) subclass `FGenericPlatformSentrySubsystem` or `FMicrosoftSentrySubsystem`.

## Contracts

- `Private/Interface/Sentry*Interface.h` defines each entity's contract. A new pure virtual needs an implementation in every backend, including `Null/`.
- Converters (`{Android,Apple,GenericPlatform,Microsoft}/Infrastructure/*SentryConverters.*`) must round-trip every `FSentryVariant` type in both directions, including arrays and maps.
- Android callbacks enter through `Android/Jni/AndroidSentryJni.cpp` from Java threads. Apple callbacks are Objective-C blocks set in `AppleSentrySubsystem.cpp`. sentry-native callbacks are static `Handle*` functions in `GenericPlatformSentrySubsystem.cpp`.
- When an option exists on all backends (attachment size limits, sample rates, enable flags), check that the change applies it on every backend that supports it, or that the PR says why not.

## Which run an event belongs to

Replays, game state, screenshots, and other crash attachments are stored in backend-specific directories. On Apple, many crash and fatal-hang events are captured on the next launch, not in-process. On Android, NDK crashes and ANR exits are also reported on the next launch, while uncaught Java exceptions run `beforeSend` in-process. Check which run's files a hook attaches.
