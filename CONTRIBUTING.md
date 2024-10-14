# Contributing

We love receiving PRs from the community with features and fixes.
For big feature it's advised to raise an issue to discuss it first.

## Getting started

### Dependencies

- [GitHub CLI](https://cli.github.com/)

### Setup Unreal Engine

In order to get the Unreal Engine we recommend using [Epic Games Launcher](https://store.epicgames.com/en-US/download).
To use the [sample project](./sample), use the UnrealEngine version specified in `EngineAssociation` property in [SentryPlayground.uproject](sample/SentryPlayground.uproject).

#### Android build support

To be able to build for Android, make sure to configure the SDK & NDK according to the [documentation](https://docs.unrealengine.com/4.27/en-US/SharingAndReleasing/Mobile/Android/Setup/AndroidStudio/). You will need to install NDK specifically at version 21.1.6352462 with UnrealEngine 5.0.

### First setup

> Currently this method is available only for C++ UE projects! Blueprint project can be converted to a C++ one simply by adding an empty class via Editor.

To get started, we recommend running the init script:

* `./scripts/init.sh` on macOS/Linux
* `./scripts/init-win.ps1` on Windows

This script links the checked out version of the plugin (the [plugin-dev](./plugin-dev/) directory) to the sample app and downloads the latest builds of native SDKs.

## Plugin structure

### Source

Sentry Unreal plugin consists of a single module `SentrySDK`. It declares the plugin API and contains separate implementations for each platform that is supported (Android/iOS/Desktop).
Other Sentry libraries are provided along with the plugin source code and can be found in `Source/ThirdParty` directory.

### Tests

In order to run automation tests navigate to `Settings -> Plugins -> Testing` and enable all the plugins available there. Editor restart will be required for the changes to take effect. After that `Windows -> Test Automation` menu will appear which opens the `Session Frontend` window. Switch to `Automation` tab there and select `Sentry` within the list of existing tests. Hit `Start Tests` button to run the tests and check the result.

## Sample project

There is a demo level (`SentryDemo.umap`) in project's Content folder which presents a simple UI allowing to send some test events to Sentry. `W_SentryDemo` blueprint implementation shows how to call the plugin API and can be used as a reference.

In order to run the demo level navigate to `Content Browser -> Content -> Maps` and open `SentryDemo` map. Hit play to launch the demo.
