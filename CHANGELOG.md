# Changelog

## Unreleased

### Features

- Add debug symbols upload settings ([#94](https://github.com/getsentry/sentry-unreal/pull/94))

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

- Bump Cocoa SDK (iOS) from v7.24.1 to v7.25.0 ([#98](https://github.com/getsentry/sentry-unreal/pull/98))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7250)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.24.1...7.25.0)
- Bump Cocoa SDK (iOS) from v7.25.0 to v7.25.1 ([#106](https://github.com/getsentry/sentry-unreal/pull/106))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7251)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.25.0...7.25.1)
- Bump Java SDK (Android) from v6.4.1 to v6.4.2 ([#115](https://github.com/getsentry/sentry-unreal/pull/115))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#642)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.4.1...6.4.2)
- Bump CLI from v2.5.2 to v2.8.1 ([#117](https://github.com/getsentry/sentry-unreal/pull/117), [#126](https://github.com/getsentry/sentry-unreal/pull/126), [#135](https://github.com/getsentry/sentry-unreal/pull/135), [#141](https://github.com/getsentry/sentry-unreal/pull/141))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#281)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.5.2...2.8.1)
- Bump Cocoa SDK (iOS) from v7.25.1 to v7.26.0 ([#120](https://github.com/getsentry/sentry-unreal/pull/120))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7260)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.25.1...7.26.0)
- Bump Cocoa SDK (iOS) from v7.26.0 to v7.27.0 ([#122](https://github.com/getsentry/sentry-unreal/pull/122))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7270)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.26.0...7.27.0)
- Bump Cocoa SDK (iOS) from v7.27.0 to v7.27.1 ([#128](https://github.com/getsentry/sentry-unreal/pull/128))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7271)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.27.0...7.27.1)
- Bump Java SDK (Android) from v6.4.2 to v6.4.3 ([#127](https://github.com/getsentry/sentry-unreal/pull/127))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#643)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.4.2...6.4.3)
- Bump Java SDK (Android) from v6.5.0-1-g4ca1d7b2 to v6.5.0 ([#129](https://github.com/getsentry/sentry-unreal/pull/129))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#650)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.5.0-1-g4ca1d7b2...6.5.0)
- Bump Native SDK from v0.5.0 to v0.5.2 ([#131](https://github.com/getsentry/sentry-unreal/pull/131), [#134](https://github.com/getsentry/sentry-unreal/pull/134))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#052)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.5.0...0.5.2)
- Bump Cocoa SDK (iOS) from v7.27.1 to v7.28.0 ([#130](https://github.com/getsentry/sentry-unreal/pull/130))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7280)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.27.1...7.28.0)
- Bump Java SDK (Android) from v6.5.0 to v6.6.0 ([#137](https://github.com/getsentry/sentry-unreal/pull/137))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#660)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.5.0...6.6.0)
- Bump Cocoa SDK (iOS) from v7.28.0 to v7.29.0 ([#136](https://github.com/getsentry/sentry-unreal/pull/136))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7290)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.28.0...7.29.0)
- Bump Java SDK (Android) from v6.5.0 to v6.6.0 ([#137](https://github.com/getsentry/sentry-unreal/pull/137))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#660)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.5.0...6.6.0)
- Bump Cocoa SDK (iOS) from v7.30.0-1-gd73ebd01 to v7.30.0 ([#143](https://github.com/getsentry/sentry-unreal/pull/143))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7300)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.30.0-1-gd73ebd01...7.30.0)
- Bump Cocoa SDK (iOS) from v7.30.0 to v7.30.1 ([#145](https://github.com/getsentry/sentry-unreal/pull/145))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/master/CHANGELOG.md#7301)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/7.30.0...7.30.1)

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
