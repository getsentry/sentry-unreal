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
- Copy or symlink the Sentry plugin sources to `sample/Plugins/Sentry/`
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
│       ├── 📄 SentryPlaygroundGameInstance.cpp/.h  # Logic for running integration tests
│       ├── 📄 SentryPlaygroundUtils.cpp/.h         # Utilities for triggering different types of crashes
│       ├── 📄 CppBeforeSendHandler.cpp/.h          # Example C++ implementation of `beforeSend` hook handler
│       ├── 📄 CppBeforeBreadcrumbHandler.cpp/.h    # Example C++ implementation of `beforeBreadcrumb` hook handler
│       └── 📄 SentryGCCallback.cpp/.h              # Utility for capturing events during garbage collection
├── 📁 Content/
│   ├── 📁 Maps/
│   │   └── 📄 SentryDemo.umap                      # Main demo level)
│   ├── 📁 UI/
│   │   └── 📄 W_SentryDemo.uasset                  # Demo UI widget for testing SDK features
│   └── 📁 Misc/
│       ├── 📄 BP_BeforeSendHandler.uasset          # Example Blueprint implementation of `beforeSend` hook handler
│       ├── 📄 BP_BeforeBreadcrumbHandler.uasset    # Example Blueprint implementation of `beforeBreadcrumb` hook handler
│       └── 📄 BP_TraceSampler.uasset               # Example Blueprint implementation of traces sampling function
├── 📁 Config/
│   └── 📄 DefaultEngine.ini                        # Configuration file with Sentry plugin settings
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

To run integration tests, specify which test to run using the appropriate argument (e.g., `-crash-capture` or `-message-capture`). The game will close after the test is completed. Otherwise, the game will launch as usual and present the sample UI.

Optionally, you can override Sentry plugin settings specified in the configuration `.ini` file by passing additional input arguments in the following format:

```pwsh
# -ini:Engine:[/Script/Sentry.SentrySettings]:<SettingName>=<Value>`

SentryPlayground.exe -nullrhi -unattended -log -crash-capture -ini:Engine:[/Script/Sentry.SentrySettings]:Dsn="your-dsn-here"
```

## Example Content

The sample project contains example Blueprint implementations of various hook handlers under `Content -> Misc`:

- `BP_BeforeSendHandler` - Example Blueprint implementation of event filtering
- `BP_BeforeBreadcrumbHandler` - Custom breadcrumb processing logic
- `BP_TraceSampler` - Performance monitoring sampling configuration

These can be configured for the SDK to use in `Project Settings -> Plugins -> Sentry`.