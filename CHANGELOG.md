# Changelog

## Unreleased

### Features

- Add HTTP proxy for desktop ([#322](https://github.com/getsentry/sentry-unreal/pull/322))
- Add scope support for Windows/Linux ([#328](https://github.com/getsentry/sentry-unreal/pull/328))
- Add extra crash context for native integration ([#342](https://github.com/getsentry/sentry-unreal/pull/342))
- Add missing plugin settings ([#335](https://github.com/getsentry/sentry-unreal/pull/335))
- Update event context categories for desktop ([#356](https://github.com/getsentry/sentry-unreal/pull/356))
- Add sentry-cli downloading tool ([#362](https://github.com/getsentry/sentry-unreal/pull/362))

### Fixes

- Fix Linux Compile/Staging Error ([#327](https://github.com/getsentry/sentry-unreal/pull/327))
- Fix UE 5.3 compatibility issues ([#348](https://github.com/getsentry/sentry-unreal/pull/348))
- Fix plugin settings names ([#350](https://github.com/getsentry/sentry-unreal/pull/350))
- Fix plugin build errors when `sentry-native` is disabled on Win/Linux ([#359](https://github.com/getsentry/sentry-unreal/pull/359))

### Dependencies

- Bump Java SDK (Android) from v6.25.0 to v6.25.1 ([#323](https://github.com/getsentry/sentry-unreal/pull/323))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6251)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.25.0...6.25.1)
- Bump Cocoa SDK (iOS) from v8.8.0 to v8.9.0 ([#324](https://github.com/getsentry/sentry-unreal/pull/324))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#890)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.8.0...8.9.0)
- Bump Java SDK (Android) from v6.25.1 to v6.25.2 ([#325](https://github.com/getsentry/sentry-unreal/pull/325))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6252)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.25.1...6.25.2)
- Bump Cocoa SDK (iOS) from v8.9.0 to v8.9.1 ([#326](https://github.com/getsentry/sentry-unreal/pull/326))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#891)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.0...8.9.1)
- Bump Java SDK (Android) from v6.25.2 to v6.26.0 ([#330](https://github.com/getsentry/sentry-unreal/pull/330))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6260)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.25.2...6.26.0)
- Bump Cocoa SDK (iOS) from v8.9.1 to v8.9.2 ([#331](https://github.com/getsentry/sentry-unreal/pull/331))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#892)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.1...8.9.2)
- Bump CLI from v2.19.4 to v2.20.5 ([#333](https://github.com/getsentry/sentry-unreal/pull/333), [#338](https://github.com/getsentry/sentry-unreal/pull/338), [#341](https://github.com/getsentry/sentry-unreal/pull/341), [#343](https://github.com/getsentry/sentry-unreal/pull/343), [#347](https://github.com/getsentry/sentry-unreal/pull/347))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2205)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.19.4...2.20.5)
- Bump Cocoa SDK (iOS) from v8.9.2 to v8.9.3 ([#334](https://github.com/getsentry/sentry-unreal/pull/334))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#893)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.2...8.9.3)
- Bump Java SDK (Android) from v6.26.0 to v6.27.0 ([#337](https://github.com/getsentry/sentry-unreal/pull/337))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6270)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.26.0...6.27.0)
- Bump Cocoa SDK (iOS) from v8.9.3 to v8.9.4 ([#344](https://github.com/getsentry/sentry-unreal/pull/344))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#894)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.3...8.9.4)
- Bump Java SDK (Android) from v6.27.0 to v6.28.0 ([#346](https://github.com/getsentry/sentry-unreal/pull/346))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6280)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.27.0...6.28.0)
- Bump Cocoa SDK (iOS) from v8.9.4 to v8.9.5 ([#349](https://github.com/getsentry/sentry-unreal/pull/349))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#895)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.4...8.9.5)
- Bump Cocoa SDK (iOS) from v8.9.5 to v8.9.6 ([#351](https://github.com/getsentry/sentry-unreal/pull/351))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#896)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.5...8.9.6)
- Bump Cocoa SDK (iOS) from v8.9.6 to v8.10.0 ([#355](https://github.com/getsentry/sentry-unreal/pull/355))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8100)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.9.6...8.10.0)

## 0.8.0

### Features

- Add API allowing to set custom handler for `beforeSend` hook ([#318](https://github.com/getsentry/sentry-unreal/pull/318))
- Refactor initialization logic for Android ([#319](https://github.com/getsentry/sentry-unreal/pull/319))

### Fixes

- Fix automatic game log attachment (Android) ([#309](https://github.com/getsentry/sentry-unreal/pull/309))
  

### Dependencies

- Bump Java SDK (Android) from v6.23.0 to v6.25.0 ([#312](https://github.com/getsentry/sentry-unreal/pull/312), [#316](https://github.com/getsentry/sentry-unreal/pull/316))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6250)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.23.0...6.25.0)
- Bump Native SDK from v0.6.3 to v0.6.5 ([#313](https://github.com/getsentry/sentry-unreal/pull/313), [#317](https://github.com/getsentry/sentry-unreal/pull/317))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#065)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.3...0.6.5)
- Bump CLI from v2.19.1 to v2.19.4 ([#314](https://github.com/getsentry/sentry-unreal/pull/314), [#315](https://github.com/getsentry/sentry-unreal/pull/315))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2194)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.19.1...2.19.4)

## 0.7.0

### Features

- Add stack trace attachment for regular events ([#301](https://github.com/getsentry/sentry-unreal/pull/301))
- Add custom transport for Linux ([#297](https://github.com/getsentry/sentry-unreal/pull/297))

### Fixes

- Unify release name for all supported platforms ([#294](https://github.com/getsentry/sentry-unreal/pull/294))
- Update plugin initialization logic ([#299](https://github.com/getsentry/sentry-unreal/pull/299))

### Dependencies
- Bump Java SDK (Android) from v6.19.1 to v6.23.0 ([#291](https://github.com/getsentry/sentry-unreal/pull/291), [#293](https://github.com/getsentry/sentry-unreal/pull/293), [#296](https://github.com/getsentry/sentry-unreal/pull/296), [#308](https://github.com/getsentry/sentry-unreal/pull/308))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6230)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.19.1...6.23.0)
- Bump Cocoa SDK (iOS) from v8.7.3 to v8.8.0 ([#304](https://github.com/getsentry/sentry-unreal/pull/304), [#307](https://github.com/getsentry/sentry-unreal/pull/307))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#880)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.7.3...8.8.0)
- Bump Native SDK from v0.6.2 to v0.6.3 ([#292](https://github.com/getsentry/sentry-unreal/pull/292))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#063)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.2...0.6.3)
- Bump CLI from v2.18.1 to v2.19.1 ([#303](https://github.com/getsentry/sentry-unreal/pull/303), [#306](https://github.com/getsentry/sentry-unreal/pull/306))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2191)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.18.1...2.19.1)

## 0.6.0

### Features

- Add plugin settings allowing to disable event capturing for certain build configurations/targets ([#273](https://github.com/getsentry/sentry-unreal/pull/273))
- Add release health monitoring feature ([#270](https://github.com/getsentry/sentry-unreal/pull/270))
- Add UE 5.2 support for CI ([#280](https://github.com/getsentry/sentry-unreal/pull/280))

### Fixes

- Fix Android release name inititalization ([#274](https://github.com/getsentry/sentry-unreal/pull/274))
- Update dependencies loading mechanism ([#287](https://github.com/getsentry/sentry-unreal/pull/287))
- Fix issue with script execution policy for debug symbols uploading on Windows ([#290](https://github.com/getsentry/sentry-unreal/pull/290))

### Dependencies

- Bump Native SDK from v0.6.1 to v0.6.2 ([#269](https://github.com/getsentry/sentry-unreal/pull/269))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#062)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.1...0.6.2)
- Bump Cocoa SDK (iOS) from v8.6.0 to v8.7.2 ([#272](https://github.com/getsentry/sentry-unreal/pull/272), [#278](https://github.com/getsentry/sentry-unreal/pull/278))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#872)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.6.0...8.7.2)
- Bump Java SDK (Android) from v6.18.1 to v6.19.1 ([#279](https://github.com/getsentry/sentry-unreal/pull/279), [#283](https://github.com/getsentry/sentry-unreal/pull/283))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6191)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.18.1...6.19.1)
- Bump CLI from v2.17.5 to v2.18.1 ([#284](https://github.com/getsentry/sentry-unreal/pull/284), [#286](https://github.com/getsentry/sentry-unreal/pull/286))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2181)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.17.5...2.18.1)
- Bump Cocoa SDK (iOS) from v8.7.2 to v8.7.3 ([#288](https://github.com/getsentry/sentry-unreal/pull/288))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#873)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.7.2...8.7.3)

## 0.5.0

### Features

- Add script for building plugin dependencies locally ([#252](https://github.com/getsentry/sentry-unreal/pull/252))

### Features

- Update Android layer implementation ([#246](https://github.com/getsentry/sentry-unreal/pull/246))

### Fixes

- Fix snapshot update script ([#253](https://github.com/getsentry/sentry-unreal/pull/253))
- Fix debug symbol uploading scripts ([#261](https://github.com/getsentry/sentry-unreal/pull/261))

### Dependencies

- Bump CLI from v2.16.1 to v2.17.5 ([#250](https://github.com/getsentry/sentry-unreal/pull/250), [#251](https://github.com/getsentry/sentry-unreal/pull/251), [#254](https://github.com/getsentry/sentry-unreal/pull/254), [#260](https://github.com/getsentry/sentry-unreal/pull/260), [#263](https://github.com/getsentry/sentry-unreal/pull/263), [#266](https://github.com/getsentry/sentry-unreal/pull/266))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2175)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.16.1...2.17.5)
- Bump Cocoa SDK (iOS) from v8.3.3 to v8.6.0 ([#247](https://github.com/getsentry/sentry-unreal/pull/247), [#257](https://github.com/getsentry/sentry-unreal/pull/257), [#267](https://github.com/getsentry/sentry-unreal/pull/267))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#860)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.3.3...8.6.0)
- Bump Java SDK (Android) from v6.17.0 to v6.18.1 ([#264](https://github.com/getsentry/sentry-unreal/pull/264), [#265](https://github.com/getsentry/sentry-unreal/pull/265))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6181)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.17.0...6.18.1)

## 0.4.0

### Features

- Added the option to automatically attach the game log to captured events ([#225](https://github.com/getsentry/sentry-unreal/pull/225))

### Dependencies

- Bump Native SDK from v0.6.0 to v0.6.1 ([#240](https://github.com/getsentry/sentry-unreal/pull/240))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#061)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.0...0.6.1)
- Bump CLI from v2.14.3 to v2.16.1 ([#224](https://github.com/getsentry/sentry-unreal/pull/224), [#229](https://github.com/getsentry/sentry-unreal/pull/229), [#232](https://github.com/getsentry/sentry-unreal/pull/232), [#237](https://github.com/getsentry/sentry-unreal/pull/237), [#241](https://github.com/getsentry/sentry-unreal/pull/241))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2161)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.14.3...2.16.1)
- Bump Cocoa SDK (iOS) from v8.2.0 to v8.3.1 ([#226](https://github.com/getsentry/sentry-unreal/pull/226), [#228](https://github.com/getsentry/sentry-unreal/pull/228))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#831)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.2.0...8.3.1)
- Bump Java SDK (Android) from v6.15.0 to v6.17.0 ([#233](https://github.com/getsentry/sentry-unreal/pull/233), [#245](https://github.com/getsentry/sentry-unreal/pull/245))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6170)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.15.0...6.17.0)

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
