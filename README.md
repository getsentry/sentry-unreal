<p align="center">
  <a href="https://sentry.io" target="_blank" align="left">
    <img src="https://raw.githubusercontent.com/getsentry/sentry-unity/main/.github/sentry-wordmark-dark-400x119.svg" width="280">
  </a>
  <br />
</p>
<p align="center">

Sentry SDK for Unreal Engine
===========

This project is an SDK for Unreal Engine that wraps different Sentry's SDKs for both desktop and mobile. Also, it [has a stable support for the Unreal Engine crash reporter](https://docs.sentry.io/platforms/unreal/setup-crashreport/).

## Downloads

The SDK can be downloaded from the [Releases] page, which also lists the
changelog of every version.

[releases]: https://github.com/getsentry/sentry-unreal/releases

## Supported Platforms and Unreal Engine version

The SDK currently supports and is tested on the following platforms:

- Windows
- macOS
- Android
- iOS

The SDK complies with three latest engine versions.

## Known Limitations

- Currently automatic crash capturing works only for mobile platforms. In order to capture crashes on Windows and Mac,
[Crash Reporter has to be configured](https://docs.sentry.io/platforms/unreal/setup-crashreport/)
  
- Using UGS binaries requires tagging of files to ensure the crashpad_handler.exe and sentry.dll is present. For inclusion in build graph, you'd want something like this: 
```
<Tag Files="#EditorBinaries$(EditorPlatform)" Filter="*.target" With="#TargetReceipts"/>
<TagReceipt Files="#TargetReceipts" RuntimeDependencies="true" With="#RuntimeDependencies"/>
<Tag Files="#RuntimeDependencies" Filter="sentry.dll;crashpad_handler.exe" With="#BinariesToArchive$(EditorPlatform)"/>
 ```

- In order to fix errors during the Android debug symbols upload in UE 5.0 or newer (Windows) the default `Android File Server` plugin has to be disabled first.

- To avoid issues with running certain plugin scripts on Windows using [PowerShell 7](https://learn.microsoft.com/en-us/powershell/scripting/install/installing-powershell-on-windows?WT.mc_id=THOMASMAURER-blog-thmaure&view=powershell-7.3&viewFallbackFrom=powershell-7) is recommended.

## Development

Please see the [contribution guide](./CONTRIBUTING.md).

## Resources

* [![Documentation](https://img.shields.io/badge/documentation-sentry.io-green.svg)](https://docs.sentry.io/platforms/unity/)
* [![Discussions](https://img.shields.io/github/discussions/getsentry/sentry-unreal.svg)](https://github.com/getsentry/sentry-unreal/discussions)
* [![Discord Chat](https://img.shields.io/discord/621778831602221064?logo=discord&logoColor=ffffff&color=7389D8)](https://discord.gg/PXa5Apfe7K)  
* [![Stack Overflow](https://img.shields.io/badge/stack%20overflow-sentry-green.svg)](http://stackoverflow.com/questions/tagged/sentry)
* [![Twitter Follow](https://img.shields.io/twitter/follow/getsentry?label=getsentry&style=social)](https://twitter.com/intent/follow?screen_name=getsentry)
