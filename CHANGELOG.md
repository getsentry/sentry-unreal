# Changelog

## Unreleased

### Features

- Added the option to automatically attach the game log to captured events ([#225](https://github.com/getsentry/sentry-unreal/pull/225))

### Dependencies

- Bump CLI from v2.14.3 to v2.16.0 ([#224](https://github.com/getsentry/sentry-unreal/pull/224), [#229](https://github.com/getsentry/sentry-unreal/pull/229), [#232](https://github.com/getsentry/sentry-unreal/pull/232), [#237](https://github.com/getsentry/sentry-unreal/pull/237))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2160)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.14.3...2.16.0)
- Bump Cocoa SDK (iOS) from v8.2.0 to v8.3.0 ([#226](https://github.com/getsentry/sentry-unreal/pull/226))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#830)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.2.0...8.3.0)
- Bump Cocoa SDK (iOS) from v8.3.0 to v8.3.1 ([#228](https://github.com/getsentry/sentry-unreal/pull/228))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#831)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.3.0...8.3.1)
- Bump Java SDK (Android) from v6.15.0 to v6.16.0 ([#233](https://github.com/getsentry/sentry-unreal/pull/233))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6160)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.15.0...6.16.0)

## 0.3.0

### Features

- Add automatic crash capturing for Windows (UE 5.1+) ([#175](https://github.com/getsentry/sentry-unreal/pull/175))
- Add extra event context params and and tags promotion ([#183](https://github.com/getsentry/sentry-unreal/pull/183))
- Add automatic crash capturing for Mac ([#190](https://github.com/getsentry/sentry-unreal/pull/190))
- Add environment property to plugin settings ([#204](https://github.com/getsentry/sentry-unreal/pull/204))
- Add native sources upload toggle to plugin settings ([#217](https://github.com/getsentry/sentry-unreal/pull/217))

### Fixes

- Fix Linux debug symbols upload when cross-compiling on Windows ([#196](https://github.com/getsentry/sentry-unreal/pull/196))
- Fix crashpad staging issue ([#211](https://github.com/getsentry/sentry-unreal/pull/211))
- Fix subsystem deinitialization ([#218](https://github.com/getsentry/sentry-unreal/pull/218))

### Dependencies

- Bump Java SDK (Android) from v6.8.0 to v6.14.0 ([#165](https://github.com/getsentry/sentry-unreal/pull/165)), [#168](https://github.com/getsentry/sentry-unreal/pull/168), [#172](https://github.com/getsentry/sentry-unreal/pull/172), [#177](https://github.com/getsentry/sentry-unreal/pull/177), [#179](https://github.com/getsentry/sentry-unreal/pull/179), [#184](https://github.com/getsentry/sentry-unreal/pull/184), [#193](https://github.com/getsentry/sentry-unreal/pull/193), [#195](https://github.com/getsentry/sentry-unreal/pull/195), [#199](https://github.com/getsentry/sentry-unreal/pull/199), [#209](https://github.com/getsentry/sentry-unreal/pull/209), [#212](https://github.com/getsentry/sentry-unreal/pull/212)
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6140)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.8.0...6.14.0)
- Bump Cocoa SDK (iOS) from v7.31.2 to v8.2.0 ([#167](https://github.com/getsentry/sentry-unreal/pull/167)), [#176](https://github.com/getsentry/sentry-unreal/pull/176), [#216](https://github.com/getsentry/sentry-unreal/pull/216)
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#820)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.31.2...8.2.0)
- Bump CLI from v2.9.0 to v2.13.0 ([#171](https://github.com/getsentry/sentry-unreal/pull/171), [#178](https://github.com/getsentry/sentry-unreal/pull/178), [#201](https://github.com/getsentry/sentry-unreal/pull/201), [#213](https://github.com/getsentry/sentry-unreal/pull/213))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2130)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.9.0...2.13.0)
- Bump Native SDK from v0.5.2 to v0.6.0 ([#173](https://github.com/getsentry/sentry-unreal/pull/173), [#198](https://github.com/getsentry/sentry-unreal/pull/198), [#210](https://github.com/getsentry/sentry-unreal/pull/210))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#060)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.5.2...0.6.0)

## 0.2.0

### Features

- Add debug symbols upload settings ([#94](https://github.com/getsentry/sentry-unreal/pull/94))
- Add conversion to FString for SentryId ([#161](https://github.com/getsentry/sentry-unreal/pull/161))
- Add editor menu for automatic CRC configuration ([#152](https://github.com/getsentry/sentry-unreal/pull/152))

### Fixes

- Packaged plugin `EngineVersion` should include `.0` patch version ([#101](https://github.com/getsentry/sentry-unreal/pull/101))
- Plugin packaging issues on Windows ([#110](https://github.com/getsentry/sentry-unreal/pull/110))
- Sentry libs linking for desktop ([#114](https://github.com/getsentry/sentry-unreal/pull/114))
- Fix sentry-cocoa SDK name ([#118](https://github.com/getsentry/sentry-unreal/pull/118))
- Fix scoped event/message capturing on Android ([#116](https://github.com/getsentry/sentry-unreal/pull/116))
- Fix event capturing on Linux ([#123](https://github.com/getsentry/sentry-unreal/pull/123))
- Fix incomplete type forward declaration ([#125](https://github.com/getsentry/sentry-unreal/pull/125))
- Fix Android packaging issue ([#133](https://github.com/getsentry/sentry-unreal/pull/133))

### Dependencies

- Bump CLI from v2.5.2 to v2.9.0 ([#117](https://github.com/getsentry/sentry-unreal/pull/117), [#126](https://github.com/getsentry/sentry-unreal/pull/126), [#135](https://github.com/getsentry/sentry-unreal/pull/135), [#141](https://github.com/getsentry/sentry-unreal/pull/141), [#154](https://github.com/getsentry/sentry-unreal/pull/154))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#290)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.5.2...2.9.0)
- Bump Java SDK (Android) from v6.4.1 to v6.8.0 ([#115](https://github.com/getsentry/sentry-unreal/pull/115)), [#127](https://github.com/getsentry/sentry-unreal/pull/127), [#129](https://github.com/getsentry/sentry-unreal/pull/129), [#134](https://github.com/getsentry/sentry-unreal/pull/134), [#137](https://github.com/getsentry/sentry-unreal/pull/137), [#137](https://github.com/getsentry/sentry-unreal/pull/137), [#144](https://github.com/getsentry/sentry-unreal/pull/144), [#156](https://github.com/getsentry/sentry-unreal/pull/156), [#158](https://github.com/getsentry/sentry-unreal/pull/158), 
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#680)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.4.1...6.8.0)
- Bump Cocoa SDK (iOS) from v7.24.1 to v7.31.2 ([#98](https://github.com/getsentry/sentry-unreal/pull/98)), [#106](https://github.com/getsentry/sentry-unreal/pull/106), [#120](https://github.com/getsentry/sentry-unreal/pull/120), [#122](https://github.com/getsentry/sentry-unreal/pull/122), [#128](https://github.com/getsentry/sentry-unreal/pull/128), [#130](https://github.com/getsentry/sentry-unreal/pull/130), [#136](https://github.com/getsentry/sentry-unreal/pull/136), [#143](https://github.com/getsentry/sentry-unreal/pull/143), [#145](https://github.com/getsentry/sentry-unreal/pull/145), [#151](https://github.com/getsentry/sentry-unreal/pull/151), [#153](https://github.com/getsentry/sentry-unreal/pull/153), [#157](https://github.com/getsentry/sentry-unreal/pull/157), [#159](https://github.com/getsentry/sentry-unreal/pull/159), 
- [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7312)
- [diff](https://github.com/getsentry/sentry-cocoa/compare/7.24.1...7.31.2)

## 0.1.1

- Internal re-release of v0.1.0 to resolve release CI issues.

## 0.1.0

### Features

- Add debug symbols upload ([#45](https://github.com/getsentry/sentry-unreal/pull/45), [#59](https://github.com/getsentry/sentry-unreal/pull/59))

### Dependencies

- Bump Cocoa SDK (iOS) from v7.14.0 to v7.24.1 ([#37](https://github.com/getsentry/sentry-unreal/pull/37), [#43](https://github.com/getsentry/sentry-unreal/pull/43), [#46](https://github.com/getsentry/sentry-unreal/pull/46), [#92](https://github.com/getsentry/sentry-unreal/pull/92), [#96](https://github.com/getsentry/sentry-unreal/pull/96))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7241)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.14.0...7.24.1)
- Bump Java SDK (Android) from v6.0.0 to v6.4.1 ([#38](https://github.com/getsentry/sentry-unreal/pull/38), [#51](https://github.com/getsentry/sentry-unreal/pull/51), [#70](https://github.com/getsentry/sentry-unreal/pull/70), [#90](https://github.com/getsentry/sentry-unreal/pull/90))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#641)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.0.0...6.4.1)
- Bump Native SDK from v0.4.17 to v0.5.0 ([#36](https://github.com/getsentry/sentry-unreal/pull/36), [#39](https://github.com/getsentry/sentry-unreal/pull/39), [#40](https://github.com/getsentry/sentry-unreal/pull/40), [#48](https://github.com/getsentry/sentry-unreal/pull/48), [#49](https://github.com/getsentry/sentry-unreal/pull/49), [#50](https://github.com/getsentry/sentry-unreal/pull/50))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#0418)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.4.17...0.5.0)

## 0.0.1

First release
