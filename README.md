<p align="center">
  <a href="https://sentry.io" target="_blank" align="left">
    <img src="https://raw.githubusercontent.com/getsentry/sentry-unity/main/.github/sentry-wordmark-dark-400x119.svg" width="280">
  </a>
  <br />
</p>
<p align="center">

Sentry SDK for Unreal Engine
===========

This project is an SDK for Unreal Engine that wraps different Sentry's SDKs for both desktop, mobile and consoles. Also, it has a stable support for the [Unreal Engine crash reporter](https://docs.sentry.io/platforms/unreal/setup-crashreport/).

## Downloads

The SDK can be downloaded from the [Releases] page, which also lists the
changelog of every version, or from the [FAB] page via Epic Games launcher.

[releases]: https://github.com/getsentry/sentry-unreal/releases
[FAB]: https://www.unrealengine.com/marketplace/en-US/product/sentry-01

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
- WinGDK

The SDK compiles with three latest engine versions.

## Blog Posts

* [Monitor Unreal Engine Game Performance with Application Metrics](https://blog.sentry.io/unreal-engine-performance-metrics/)
* [Game Console Support GA](https://blog.sentry.io/error-monitoring-and-crash-reporting-for-gaming-consoles/)
* [Building the Sentry Unreal Engine SDK with GitHub Actions](https://blog.sentry.io/building-the-sentry-unreal-engine-sdk-with-github-actions/)

## Known Limitations

- To automatically capture crashes in Windows game builds that were made using engine versions prior to UE 5.2, the [Crash Reporter has to be configured](https://docs.sentry.io/platforms/unreal/setup-crashreport/) or engine source code [modifications](https://github.com/EpicGames/UnrealEngine/pull/7976) must be applied (make sure to check the PRs linked there as well).
  
- Callbacks that are registered as hooks (such as `BeforeSendHandler`) will not be invoked during garbage collection or while objects are being post-loaded.

- Fast-fail crash capturing is currently supported only in packaged game builds. When a fast-fail crash occurs the `HandleBeforeSend` hook will not be invoked and any custom event pre-processing will be skipped.

## Development

Please see the [contribution guide](./CONTRIBUTING.md).

## Resources

* [![Documentation](https://img.shields.io/badge/documentation-sentry.io-green.svg)](https://docs.sentry.io/platforms/unreal/)
* [![Discussions](https://img.shields.io/github/discussions/getsentry/sentry-unreal.svg)](https://github.com/getsentry/sentry-unreal/discussions)
* [![Discord Chat](https://img.shields.io/discord/621778831602221064?logo=discord&logoColor=ffffff&color=7389D8)](https://discord.gg/PXa5Apfe7K)  
* [![Stack Overflow](https://img.shields.io/badge/stack%20overflow-sentry-green.svg)](http://stackoverflow.com/questions/tagged/sentry)
* [![X Follow](https://img.shields.io/twitter/follow/sentry?label=sentry&style=social)](https://x.com/intent/follow?screen_name=sentry)
