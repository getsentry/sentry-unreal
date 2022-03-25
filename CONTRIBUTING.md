# Contributing

We love receiving PRs from the community with features and fixes. 
For big feature it's advised to raise an issue to discuss it first.

## Getting started

### Setup Unreal Engine

In order to get the Unreal Engine we recommend using [Epic Games Launcher](https://store.epicgames.com/en-US/download) which allows to download and install specific UE versions and their components. Sentry Unreal is compatible with UE 4.25 and newer. Enabling Android and iOS target platforms support during engine installation will allow using this plugin in mobile applications as well.

### Install the plugin

> Currently this method is available only for C++ UE projects! Blueprint project can be converted to a C++ one simply by adding an empty class via Editor.

To install the plugin `cd` to UE project root and create a new `Plugins` directory there (`mkdir Plugins`). Then clone the repo into it `cd Plugins`, `git@github.com:getsentry/sentry-unreal.git`. On the next project launch UE will prompt to build SentrySDK module.

### Configure project

Before starting to use Sentry Unreal make sure that the plugin is enabled. In UE editor navigate to `Settings -> Plugins -> Code Plugins' menu and check its status.

In order to send events and crash data to Sentry, UE project has to be associated with the corresponding Sentry project first. Open `Settings -> Project Settings -> Plugins -> Sentry SDK` and provide the valid DSN (more info about DSN [here](https://docs.sentry.io/product/sentry-basics/dsn-explainer/)).

## Plugin structure

### Source

Sentry Unreal plugin consists of a single module `SentrySDK`. It declares the plugin API and contains separate implementations for each platform that is supported (Android/iOS/Desktop).
Other Sentry libraries are provided along with the plugin source code and can be found in `Source/ThirdParty` directory.

### Content

There is a demo level (`SentryDemo.umap`) in plugin's Content folder which presents a simple UI allowing to send some test events to Sentry. `W_SentryDemo` blueprint implementation shows how to call the plugin API and can be used as a reference.
