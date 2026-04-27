<p align="center">
  <a href="https://sentry.io" target="_blank" align="left">
    <img src="https://raw.githubusercontent.com/getsentry/sentry-unity/main/.github/sentry-wordmark-dark-400x119.svg" width="280">
  </a>
  <br />
</p>
<p align="center">

Sentry Unreal Engine SDK Sample Project
===========

This sample project demonstrates the capabilities of the Sentry Unreal Engine SDK and provides a comprehensive testing environment for all SDK features.

## Getting started

### Prerequisites

- Unreal Engine 4.27 or newer (version configured in `SentryPlayground.uproject`)
- Platform support: Windows, macOS, Linux, Android, iOS, PlayStation 5, Xbox, Nintendo Switch
- Sentry account with a project and DSN

### Setup

- Clone or download the Sentry Unreal SDK repository
- Provide the Sentry plugin under `sample/Plugins/Sentry/` — either extract a pre-made package from the [Releases page](https://github.com/getsentry/sentry-unreal/releases), or build locally from `plugin-dev/` sources (see [CONTRIBUTING.md](../CONTRIBUTING.md))
- Open `SentryPlayground.uproject` in Unreal Engine
- Configure your Sentry DSN in `Config/DefaultEngine.ini` or through the project settings menu
- Play the `SentryDemo` level to begin testing

## Project structure overview

Here's a breakdown of the important sample project files and folders:

```pwsh
📁 sample
├── 📄 SentryPlayground.uproject                    # Engine version configuration, supports UE 4.27 and newer
├── 📁 Source/
│   └── 📁 SentryPlayground/
│       ├── 📄 SentryPlaygroundGameInstance.cpp/.h       # Dispatches integration tests based on command-line switches
│       ├── 📄 SentryPlaygroundGameModeBase.cpp/.h       # Sample game mode
│       ├── 📁 IntegrationTests/                         # One class per integration test, sharing a common base
│       │   ├── 📄 SentryBaseIntegrationTest.cpp/.h      # Base class (FSentryBaseIntegrationTest) with shared helpers
│       │   ├── 📄 SentryCrashTest.cpp/.h                # Crash scenarios (null ptr, stack overflow, OOM, etc.)
│       │   ├── 📄 SentryMessageTest.cpp/.h              # Message capture with scope customization
│       │   ├── 📄 SentryLogTest.cpp/.h                  # Structured log capture
│       │   ├── 📄 SentryMetricTest.cpp/.h               # Counter/distribution/gauge metrics
│       │   ├── 📄 SentryTracingTest.cpp/.h              # Transactions and spans
│       │   ├── 📄 SentryEnsureTest.cpp/.h               # Non-fatal ensure() capture
│       │   ├── 📄 SentryHangTest.cpp/.h                 # Application hang detection
│       │   └── 📄 SentryInitOnlyTest.cpp/.h             # SDK init smoke test
│       ├── 📁 Hooks/                                    # Example C++ implementations of SDK hook handlers
│       │   ├── 📄 CppBeforeSendHandler.cpp/.h
│       │   ├── 📄 CppBeforeBreadcrumbHandler.cpp/.h
│       │   ├── 📄 CppBeforeLogHandler.cpp/.h
│       │   ├── 📄 CppBeforeMetricHandler.cpp/.h
│       │   └── 📄 CppTraceSampler.cpp/.h
│       └── 📁 Utils/
│           ├── 📄 SentryPlaygroundCrashUtils.cpp/.h     # Crash/ensure/hang trigger helpers + ESentryAppTerminationType enum
│           ├── 📄 SentryPlaygroundBlueprintUtils.cpp/.h # Misc Blueprint-callable helpers (string/bytes/file)
│           └── 📄 SentryGCCallback.cpp/.h               # Utility for capturing events during garbage collection
├── 📁 Content/
│   ├── 📁 Maps/
│   │   └── 📄 SentryDemo.umap                      # Main demo level
│   ├── 📁 UI/
│   │   └── 📄 W_SentryDemo.uasset                  # Demo UI widget for testing SDK features
│   └── 📁 Misc/
│       ├── 📄 BP_BeforeSendHandler.uasset          # Example Blueprint implementation of `beforeSend` hook handler
│       ├── 📄 BP_BeforeBreadcrumbHandler.uasset    # Example Blueprint implementation of `beforeBreadcrumb` hook handler
│       ├── 📄 BP_BeforeLogHandler.uasset           # Example Blueprint implementation of `beforeLog` hook handler
│       ├── 📄 BP_BeforeMetricHandler.uasset        # Example Blueprint implementation of `beforeMetric` hook handler
│       └── 📄 BP_TraceSampler.uasset               # Example Blueprint implementation of `tracesSampler` hook handler
├── 📁 Config/
│   └── 📄 DefaultEngine.ini                        # Sentry plugin settings live here under `[/Script/Sentry.SentrySettings]`
└── 📁 Plugins/                                     # Location for Sentry SDK sources - copy or symlink here
```

## Demo Level

The demo level (`SentryDemo.umap`) in the project's Content folder presents a simple UI for sending test events to Sentry. The `W_SentryDemo` Blueprint implementation demonstrates how to call the plugin API and serves as a reference.

To run the demo level, navigate to `Content Browser -> Content -> Maps` and open the `SentryDemo` map. Click Play to launch the demo.

## Unit Tests

To run automation tests, several engine plugins are enabled (see `Settings -> Plugins -> Testing`). Navigate to `Windows -> Test Automation` menu and open the `Session Frontend` window. Switch to the `Automation` tab and select `Sentry` from the list of available tests. Click the `Start Tests` button to run the tests and check the results.

## Integration Tests

The `SentryPlaygroundGameInstance.cpp` file contains logic that parses command line input used to launch the sample game build and runs test actions accordingly. Here are example commands:

```pwsh
# Windows - Crash capture test
SentryPlayground.exe -nullrhi -unattended -log -crash-capture

# Windows - Message capture test
SentryPlayground.exe -nullrhi -unattended -log -message-capture
```

The following test switches are supported:

- `-crash-capture` - generic nullptr-dereferencing crash
- `-crash-stack-overflow` - stack overflow crash
- `-crash-memory-corruption` - memory corruption crash
- `-crash-assert` - `check()` assertion failure
- `-crash-oom` - out-of-memory crash
- `-message-capture` - capture a message event
- `-log-capture` - capture a structured log
- `-metric-capture` - emit a metric
- `-tracing-capture` - capture a transaction/span
- `-ensure-capture` - trigger a non-fatal `ensure()`
- `-hang-capture` - simulate an application hang
- `-init-only` - initialize the SDK and exit without running a test action

The game will close after the test is completed. Otherwise, the game will launch as usual and present the sample UI.

After completing a test the game emits a single line to stdout in the form `TEST_RESULT: {"test":"<name>","success":<true|false>,"message":"<text>"}`, which the CI test harness parses to determine the outcome.

Optionally, you can override Sentry plugin settings specified in the configuration `.ini` file by passing additional input arguments in the following format:

```pwsh
# -ini:Engine:[/Script/Sentry.SentrySettings]:<SettingName>=<Value>`

SentryPlayground.exe -nullrhi -unattended -log -crash-capture -ini:Engine:[/Script/Sentry.SentrySettings]:Dsn="your-dsn-here"
```

## Example Content

The sample project contains example Blueprint implementations of various hook handlers under `Content -> Misc`:

- `BP_BeforeSendHandler` - Filter or modify events before they are sent
- `BP_BeforeBreadcrumbHandler` - Filter or modify breadcrumbs before they are recorded
- `BP_BeforeLogHandler` - Filter or modify structured logs before they are sent
- `BP_BeforeMetricHandler` - Filter or modify metrics before they are emitted
- `BP_TraceSampler` - Decide the sample rate for transactions

These can be configured for the SDK to use in `Project Settings -> Plugins -> Sentry -> Hooks`.