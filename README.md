<p align="center">
  <a href="https://sentry.io" target="_blank" align="left">
    <img src="https://raw.githubusercontent.com/getsentry/sentry-unity/main/.github/sentry-wordmark-dark-400x119.svg" width="280">
  </a>
  <br />
</p>
<p align="center">

Sentry SDK for Unreal Engine
===========

This project is an SDK for Unreal Engine that wraps different Sentry's SDKs for both desktop, mobile and consoles. Also, it [has a stable support for the Unreal Engine crash reporter](https://docs.sentry.io/platforms/unreal/setup-crashreport/).

## Downloads

The SDK can be downloaded from the [Releases] page, which also lists the
changelog of every version, or from the [UE Marketplace] page via Epic Games launcher.

[releases]: https://github.com/getsentry/sentry-unreal/releases
[UE Marketplace]: https://www.unrealengine.com/marketplace/en-US/product/sentry-01

## Supported Platforms and Unreal Engine version

The SDK currently supports and is tested on the following platforms:

- Windows
- macOS
- Linux 
- Android
- iOS
- PlayStation
- Xbox
- Nintendo Switch

The SDK compiles with three latest engine versions.

Blog posts:
* [Building the Sentry Unreal Engine SDK with GitHub Actions](https://blog.sentry.io/building-the-sentry-unreal-engine-sdk-with-github-actions/)
* [Game Console Support GA](https://blog.sentry.io/error-monitoring-and-crash-reporting-for-gaming-consoles/)

## Known Limitations

- On all platforms captured crashes are uploaded to Sentry only after relaunching the crashed app since the in-process handler cannot do this within the same session. The only exceptions are Windows (if using the GitHub package) and Linux for which the out-of-process crashpad handler is used and crashes are uploaded immediately.

- To automatically capture crashes in Windows game builds that were made using engine versions prior to UE 5.2, the [Crash Reporter has to be configured](https://docs.sentry.io/platforms/unreal/setup-crashreport/) first.
  
- Using UGS binaries requires tagging of files to ensure the crashpad_handler.exe is present. For inclusion in build graph, you'd want something like this: 
```
<Tag Files="#EditorBinaries$(EditorPlatform)" Filter="*.target" With="#TargetReceipts"/>
<TagReceipt Files="#TargetReceipts" RuntimeDependencies="true" With="#RuntimeDependencies"/>
<Tag Files="#RuntimeDependencies" Filter="sentry.dll;crashpad_handler.exe" With="#BinariesToArchive$(EditorPlatform)"/>
 ```

- In UE 5.2 or newer game log attached to crashes captured with `sentry-native` integration instead of [crash reporter](https://docs.sentry.io/platforms/unreal/setup-crashreport/) could be truncated. This is caused by current `crashpad` behavior which sends crashes to Sentry right away while UE is still about to write some bits of information to the log file.

- Only crash events captured on Android contain the full callstack. Events that were captured manually won't have the native C++ part there.

- `BeforeSendHandler` and `BeforeBreadcrumbHandler` hooks will not be invoked during garbage collection.

- It may be required to upgrade the C++ standard library (`libstdc++`) on older Linux distributions (such as Ubuntu 18.04 and 20.04) to ensure crashpad handler proper functionality within the deployment environment. This can be achieved with something like this:
```
sudo apt-get update
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get install -y libstdc++6
```

- Plugin supports Linux arm64 platform for UE 5.0 and newer.

- Fast-fail crash capturing is currently supported only in packaged game builds. When a fast-fail crash occurs the `HandleBeforeSend` hook will not be invoked and any custom event pre-processing will be skipped.

## Development

Please see the [contribution guide](./CONTRIBUTING.md).

## Resources

* [![Documentation](https://img.shields.io/badge/documentation-sentry.io-green.svg)](https://docs.sentry.io/platforms/unreal/)
* [![Discussions](https://img.shields.io/github/discussions/getsentry/sentry-unreal.svg)](https://github.com/getsentry/sentry-unreal/discussions)
* [![Discord Chat](https://img.shields.io/discord/621778831602221064?logo=discord&logoColor=ffffff&color=7389D8)](https://discord.gg/PXa5Apfe7K)  
* [![Stack Overflow](https://img.shields.io/badge/stack%20overflow-sentry-green.svg)](http://stackoverflow.com/questions/tagged/sentry)
* [![X Follow](https://img.shields.io/twitter/follow/sentry?label=sentry&style=social)](https://x.com/intent/follow?screen_name=sentry)
