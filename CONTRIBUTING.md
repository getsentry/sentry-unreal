# Contributing

We love receiving PRs from the community with features and fixes.
For big feature it's advised to raise an issue to discuss it first.

## Getting Started

### Prerequisites

- Unreal Engine 4.27 or newer
- [GitHub CLI](https://cli.github.com/)

### First Setup

To get started, clone the source code and run the initialization script:

```pwsh
# Clone the repository
git clone https://github.com/getsentry/sentry-unreal.git
cd sentry-unreal

# Run initialization (macOS/Linux)
./scripts/init.sh

# Run initialization (Windows)
./scripts/init-win.ps1
```

This script links the checked out version of the plugin (the [plugin-dev](./plugin-dev/) directory) to the playground project (the [sample](./sample/) directory) and downloads the latest pre-built SDK dependencies from CI using GitHub CLI.

> [!TIP]
> When working with a `sentry-unreal` fork, run `gh repo set-default` to select the default remote repository used for GitHub API queries (required for pulling plugin dependencies from CI). If you don't plan to configure the CI pipeline for your fork, it's recommended to set `getsentry/sentry-unreal` as the default remote.

After that, you can generate project files for SentryPlayground.uproject to open it in your IDE and make changes to the SDK as you would with any other Unreal project.

### Modifying Plugin Content

All files that belong to the plugin are listed in the snapshot file:

- `/scripts/packaging/package.snapshot`

If you add, delete or move files within the `plugin-dev` directory this snapshot file must be updated to reflect the changes. To do that, run:

```pwsh
pwsh ./scripts/packaging/pack.ps1
pwsh ./scripts/packaging/test-contents.ps1 accept`
```

Once completed, make sure to commit the updated snapshot files to Git.

CI will run a separate check to compare the actual plugin package contents against the snapshot file.

### Building for Consoles

The Sentry Unreal SDK provides [consoles support](https://docs.sentry.io/platforms/unreal/game-consoles/) via platform extensions:

- [PlayStation](https://github.com/getsentry/sentry-playstation)
- [Xbox](https://github.com/getsentry/sentry-xbox)
- [Nintendo Switch](https://github.com/getsentry/sentry-switch)

Once you've obtained the extension source code, run the following commands to complete the setup:

```pwsh
# One-time: Set environment variables
$env:SENTRY_SWITCH_PATH = "D:\projects\sentry-switch"
$env:SENTRY_PLAYSTATION_PATH = "D:\projects\sentry-playstation"
$env:SENTRY_XBOX_PATH = "D:\projects\sentry-xbox"

# Option 1: Setup all platforms
./scripts/init-consoles.ps1 -All

# Option 2: Setup specific platforms
./scripts/init-consoles.ps1 -Switch -PS5 -XSX -XB1

# Option 3: Setup individual platform without configuring environment variables
./scripts/init-console-ext.ps1 -Platform Switch -ExtensionPath D:\projects\sentry-switch
```

These scripts pre-build the binaries required for consoles and copy them to sample project dir accordingly. They also symlink extension source files so that changes made while working with `sentry-unreal` are automatically tracked in the corresponding extension repositories.

> [!TIP]
> For more details about the environment and Unreal Engine setup required for console development, refer to the corresponding plugin extension repositories' `unreal/CONTRIBUTING.md` files.

## Plugin Structure

Sentry Unreal SDK consists of two modules:

1. `SentrySDK` - it declares the plugin API and contains separate implementations for each platform that is supported.
2. `SentryEditor` - contains editor utilities like customizations for plugin settings menu, tools allowing to re-compile or download dependencies, etc.

Other Sentry libraries are provided along with the plugin source code and can be found in `Source/ThirdParty` directory.

## Sample Project

There is a demo level (`SentryDemo.umap`) in project's Content folder which presents a simple UI allowing to send some test events to Sentry. `W_SentryDemo` blueprint implementation shows how to call the plugin API and can be used as a reference.

In order to run the demo level navigate to `Content Browser -> Content -> Maps` and open `SentryDemo` map. Hit play to launch the demo.

Refer to sample project [README.md](sample/README.md) for more details.

> [!IMPORTANT]
> When modifying binary assets in the sample project (e.g., Blueprints), use the **oldest** supported Unreal Engine version to maintain their compatibility across **all** supported versions. Refer to the `EngineAssociation` property in `sample/SentryPlayground.uproject` on the `main` branch for the correct version.

## Automation Testing

In order to run automation tests via Editor navigate to `Settings -> Plugins -> Testing` and enable all the plugins available there. Editor restart will be required for the changes to take effect. After that `Windows -> Test Automation` menu will appear which opens the `Session Frontend` window. Switch to `Automation` tab there and select `Sentry` within the list of existing tests. Hit `Start Tests` button to run the tests and check the result.

Tests source code can be found at `plugin-dev/Source/Sentry/Private/Tests`.

## Documentation

Sentry Unreal SDK has the official [documentation](https://docs.sentry.io/platforms/unreal/). When introducing a new feature, make sure it's properly documented by opening a corresponding pull request in the [sentry-docs](https://github.com/getsentry/sentry-docs) repository and linking it to your PR here.
