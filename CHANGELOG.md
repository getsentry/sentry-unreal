# Changelog

## Unreleased

### Features

- Add user feedback capturing support for desktop ([#521](https://github.com/getsentry/sentry-unreal/pull/521))
- Add breadcrumbs automatically when printing to logs ([#522](https://github.com/getsentry/sentry-unreal/pull/522))
- Add proper log verbosity type for internal `sentry-native` messages ([#536](https://github.com/getsentry/sentry-unreal/pull/536))

### Fixes

- The SDK no longer prints symbol uploading related warnings when disabled ([#528](https://github.com/getsentry/sentry-unreal/pull/528))
- Fixed an issue when parsing the config file during symbol upload ([#541](https://github.com/getsentry/sentry-unreal/pull/541))

### Dependencies

- Bump CLI from v2.29.1 to v2.31.0 ([#512](https://github.com/getsentry/sentry-unreal/pull/512), [#515](https://github.com/getsentry/sentry-unreal/pull/515), [#517](https://github.com/getsentry/sentry-unreal/pull/517), [#524](https://github.com/getsentry/sentry-unreal/pull/524), [#525](https://github.com/getsentry/sentry-unreal/pull/525))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2310)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.29.1...2.31.0)
- Bump Java SDK (Android) from v7.5.0 to v7.8.0 ([#513](https://github.com/getsentry/sentry-unreal/pull/513), [#534](https://github.com/getsentry/sentry-unreal/pull/534), [#535](https://github.com/getsentry/sentry-unreal/pull/535))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#780)
  - [diff](https://github.com/getsentry/sentry-java/compare/7.5.0...7.8.0)
- Bump Native SDK from v0.7.0 to v0.7.2 ([#520](https://github.com/getsentry/sentry-unreal/pull/520), [#531](https://github.com/getsentry/sentry-unreal/pull/531))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#072)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.7.0...0.7.2)
- Bump Cocoa SDK (iOS) from v8.21.0 to v8.24.0 ([#539](https://github.com/getsentry/sentry-unreal/pull/539))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8240)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.21.0...8.24.0)
- Bump Cocoa SDK (iOS) from v8.24.0 to v8.25.0 ([#549](https://github.com/getsentry/sentry-unreal/pull/549))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8250)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.24.0...8.25.0)

## 0.16.0

### Features

- Add performance monitoring API ([#470](https://github.com/getsentry/sentry-unreal/pull/470))
- Add traces sampler function config for Android/Apple ([#488](https://github.com/getsentry/sentry-unreal/pull/488))
- Add `IsCrashedLastRun` allowing to check whether the app crashed during its last run ([#483](https://github.com/getsentry/sentry-unreal/pull/483))
- Improved crash capture backend handling based on package version (GitHub or Marketplace) ([#479](https://github.com/getsentry/sentry-unreal/pull/479))

### Fixes

- Fix Linux intermediates paths in `FilterPlugin.ini` ([#468](https://github.com/getsentry/sentry-unreal/pull/468))
- Fix casing for include of HAL/PlatformFileManager for Linux compilation ([#468](https://github.com/getsentry/sentry-unreal/pull/499))
- The message in events in the `SentryBeforeSendHandler` are no longer missing their message  ([#510](https://github.com/getsentry/sentry-unreal/pull/510))

### Dependencies

- Bump Cocoa SDK (iOS) from v8.17.1 to v8.20.0 ([#467](https://github.com/getsentry/sentry-unreal/pull/467), [#474](https://github.com/getsentry/sentry-unreal/pull/474), [#487](https://github.com/getsentry/sentry-unreal/pull/487), [#492](https://github.com/getsentry/sentry-unreal/pull/492))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8200)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.17.1...8.20.0)
- Bump Java SDK (Android) from v7.0.0 to v7.4.0 ([#469](https://github.com/getsentry/sentry-unreal/pull/469), [#481](https://github.com/getsentry/sentry-unreal/pull/481), [#491](https://github.com/getsentry/sentry-unreal/pull/491), [#501](https://github.com/getsentry/sentry-unreal/pull/501))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#740)
  - [diff](https://github.com/getsentry/sentry-java/compare/7.0.0...7.2.0)
- Bump CLI from v2.23.0 to v2.29.1 ([#471](https://github.com/getsentry/sentry-unreal/pull/471), [#472](https://github.com/getsentry/sentry-unreal/pull/472), [#475](https://github.com/getsentry/sentry-unreal/pull/475), [#477](https://github.com/getsentry/sentry-unreal/pull/477), [#484](https://github.com/getsentry/sentry-unreal/pull/484), [#485](https://github.com/getsentry/sentry-unreal/pull/485), [#486](https://github.com/getsentry/sentry-unreal/pull/486), [#490](https://github.com/getsentry/sentry-unreal/pull/490), [#493](https://github.com/getsentry/sentry-unreal/pull/493), [#494](https://github.com/getsentry/sentry-unreal/pull/494), [#495](https://github.com/getsentry/sentry-unreal/pull/495), [#507](https://github.com/getsentry/sentry-unreal/pull/507))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2291)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.23.0...2.29.1)
- Bump Native SDK from v0.6.7 to v0.7.0 ([#478](https://github.com/getsentry/sentry-unreal/pull/478))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#070)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.7...0.7.0)

## 0.15.1

### Fixes

- Fix Android build errors regarding `IScope` in `SentryBridgeJava` ([#464](https://github.com/getsentry/sentry-unreal/pull/464))

## 0.15.0

### Breaking Changes

- Change `USentrySubsystem` base class to `UEngineSubsystem` in order to capture editor crashes ([#436](https://github.com/getsentry/sentry-unreal/pull/436))\
    If you make use of plugin's blueprint API, you will need to recreate `Get Sentry Subsystem` nodes\
    If you make use of plugin's C++ API, you will need to update your implementation by accessing `USentrySubsystem` via `GEngine` pointer

### Fixes

- Fix issue with invalidating breadcrumbs during event capturing on Win/Linux ([#445](https://github.com/getsentry/sentry-unreal/pull/445))
- Fix build errors when cross-compiling for Linux on Windows with UE Marketplace plugin version ([#453](https://github.com/getsentry/sentry-unreal/pull/453))
- Fix build errors on Mac when using UE Marketplace plugin version ([#451](https://github.com/getsentry/sentry-unreal/pull/451))

### Dependencies

- Bump Java SDK (Android) from v6.33.0 to v7.0.0 ([#435](https://github.com/getsentry/sentry-unreal/pull/435), [#449](https://github.com/getsentry/sentry-unreal/pull/449), [#460](https://github.com/getsentry/sentry-unreal/pull/460))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#700)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.33.0...7.0.0)
- Bump CLI from v2.21.2 to v2.23.0 ([#437](https://github.com/getsentry/sentry-unreal/pull/437), [#441](https://github.com/getsentry/sentry-unreal/pull/441), [#444](https://github.com/getsentry/sentry-unreal/pull/444), [#454](https://github.com/getsentry/sentry-unreal/pull/454), [#457](https://github.com/getsentry/sentry-unreal/pull/457), [#462](https://github.com/getsentry/sentry-unreal/pull/462))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2230)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.21.2...2.23.0)
- Bump Cocoa SDK (iOS) from v8.15.0 to v8.17.1 ([#438](https://github.com/getsentry/sentry-unreal/pull/438), [#439](https://github.com/getsentry/sentry-unreal/pull/439), [#450](https://github.com/getsentry/sentry-unreal/pull/450), [#452](https://github.com/getsentry/sentry-unreal/pull/452), [#456](https://github.com/getsentry/sentry-unreal/pull/456), [#461](https://github.com/getsentry/sentry-unreal/pull/461))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8171)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.15.0...8.15.1)
- Bump Native SDK from v0.6.6 to v0.6.7 ([#442](https://github.com/getsentry/sentry-unreal/pull/442))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#067)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.6...0.6.7)

## 0.14.0

### Features

- Switch to using static libs on Windows ([#433](https://github.com/getsentry/sentry-unreal/pull/433))

### Dependencies

- Bump Java SDK (Android) from v6.32.0 to v6.33.0 ([#432](https://github.com/getsentry/sentry-unreal/pull/432))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6330)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.32.0...6.33.0)
- Bump Cocoa SDK (iOS) from v8.14.2 to v8.15.0 ([#434](https://github.com/getsentry/sentry-unreal/pull/434))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8150)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.14.2...8.15.0)

## 0.13.0

### Features

- Switch to using static libs on Linux ([#424](https://github.com/getsentry/sentry-unreal/pull/424))

### Fixes

- Fix invalid breadcrumbs level for Win/Linux ([#426](https://github.com/getsentry/sentry-unreal/pull/426))
- Fix build errors in UE4 ([#428](https://github.com/getsentry/sentry-unreal/pull/428))
- Fix iOS build errors ([#429](https://github.com/getsentry/sentry-unreal/pull/429))

## 0.12.1

### Fixes

- Fix missing plugin binaries in the UE Marketplace package ([#423](https://github.com/getsentry/sentry-unreal/pull/423))

### Dependencies

- Bump Cocoa SDK (iOS) from v8.13.1 to v8.14.2 ([#417](https://github.com/getsentry/sentry-unreal/pull/417), [#422](https://github.com/getsentry/sentry-unreal/pull/422))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8142)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.13.1...8.14.2)
- Bump Java SDK (Android) from v6.31.0 to v6.32.0 ([#416](https://github.com/getsentry/sentry-unreal/pull/416))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6320)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.31.0...6.32.0)

## 0.12.0

### Fixes

- Fix issue with overwriting `__sentry` attribute in crash context object ([#401](https://github.com/getsentry/sentry-unreal/pull/401))
- Fix event level being always overwritten by the current scope's level on Win/Linux ([#412](https://github.com/getsentry/sentry-unreal/pull/412))
- Fix stack corruption during crash capturing within `on_crash` hook handler on Linux ([#410](https://github.com/getsentry/sentry-unreal/pull/410))
- Move crash properties set as `Extra` to a separate Sentry context ([#413](https://github.com/getsentry/sentry-unreal/pull/413))

### Dependencies

- Bump Native SDK from v0.6.5 to v0.6.6 ([#405](https://github.com/getsentry/sentry-unreal/pull/405))
  - [changelog](https://github.com/getsentry/sentry-native/blob/master/CHANGELOG.md#066)
  - [diff](https://github.com/getsentry/sentry-native/compare/0.6.5...0.6.6)

## 0.11.0

### Fixes

- Fix issue with missing Sentry.framework in iOS app bundle UE 5.3 ([#390](https://github.com/getsentry/sentry-unreal/pull/390))
- Fix dependencies loading for desktop ([#393](https://github.com/getsentry/sentry-unreal/pull/393))
- Fix array/map Json string check to avoid unnecessary error messages in logs ([#394](https://github.com/getsentry/sentry-unreal/pull/394))
- Fix conditional debug symbols uploading ([#399](https://github.com/getsentry/sentry-unreal/pull/399))
- Fix compilation errors in UE 4.27 ([#398](https://github.com/getsentry/sentry-unreal/pull/398))

### Dependencies

- Bump Cocoa SDK (iOS) from v8.11.0 to v8.13.0 ([#386](https://github.com/getsentry/sentry-unreal/pull/386), [#392](https://github.com/getsentry/sentry-unreal/pull/392))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8130)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.11.0...8.13.0)
- Bump CLI from v2.20.7 to v2.21.2 ([#389](https://github.com/getsentry/sentry-unreal/pull/389), [#391](https://github.com/getsentry/sentry-unreal/pull/391), [#400](https://github.com/getsentry/sentry-unreal/pull/400))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2212)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.20.7...2.21.2)
- Bump Java SDK (Android) from v6.29.0 to v6.30.0 ([#396](https://github.com/getsentry/sentry-unreal/pull/396))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6300)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.29.0...6.30.0)

## 0.10.0

### Features

- Add symbol upload scripts downloading tool ([#385](https://github.com/getsentry/sentry-unreal/pull/385))
- Add support of two plugin versions for GitHub/Marketplace([#387](https://github.com/getsentry/sentry-unreal/pull/387))

### Fixes

- Fix errors caused by Cpp20 adoption in UE 5.3 ([#377](https://github.com/getsentry/sentry-unreal/pull/377))

### Dependencies

- Bump Cocoa SDK (iOS) from v8.10.0 to v8.11.0 ([#367](https://github.com/getsentry/sentry-unreal/pull/367))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8110)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.10.0...8.11.0)
- Bump Java SDK (Android) from v6.28.0 to v6.29.0 ([#371](https://github.com/getsentry/sentry-unreal/pull/371))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6290)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.28.0...6.29.0)
- Bump CLI from v2.20.6 to v2.20.7 ([#375](https://github.com/getsentry/sentry-unreal/pull/375))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2207)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.20.6...2.20.7)

## 0.9.0

### Features

- Add HTTP proxy for desktop ([#322](https://github.com/getsentry/sentry-unreal/pull/322))
- Add scope support for Windows/Linux ([#328](https://github.com/getsentry/sentry-unreal/pull/328))
- Add extra crash context for native integration ([#342](https://github.com/getsentry/sentry-unreal/pull/342))
- Add missing plugin settings ([#335](https://github.com/getsentry/sentry-unreal/pull/335))
- Update event context categories for desktop ([#356](https://github.com/getsentry/sentry-unreal/pull/356))
- Add sentry-cli downloading tool ([#362](https://github.com/getsentry/sentry-unreal/pull/362))
- Add breakpad support for Windows ([#363](https://github.com/getsentry/sentry-unreal/pull/363))
- Add Options for enabling platforms & Promoted Builds via the GUI ([#360](https://github.com/getsentry/sentry-unreal/pull/360))

### Fixes

- Fix Linux Compile/Staging Error ([#327](https://github.com/getsentry/sentry-unreal/pull/327))
- Fix UE 5.3 compatibility issues ([#348](https://github.com/getsentry/sentry-unreal/pull/348))
- Fix plugin settings names ([#350](https://github.com/getsentry/sentry-unreal/pull/350))
- Fix plugin build errors when `sentry-native` is disabled on Win/Linux ([#359](https://github.com/getsentry/sentry-unreal/pull/359))

### Dependencies

- Bump Java SDK (Android) from v6.25.0 to v6.28.0 ([#323](https://github.com/getsentry/sentry-unreal/pull/323), [#325](https://github.com/getsentry/sentry-unreal/pull/325), [#330](https://github.com/getsentry/sentry-unreal/pull/330), [#337](https://github.com/getsentry/sentry-unreal/pull/337), [#346](https://github.com/getsentry/sentry-unreal/pull/346))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6280)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.25.0...6.28.0)
- Bump Cocoa SDK (iOS) from v8.8.0 to v8.10.0 ([#324](https://github.com/getsentry/sentry-unreal/pull/324), [#326](https://github.com/getsentry/sentry-unreal/pull/326), [#331](https://github.com/getsentry/sentry-unreal/pull/331), [#334](https://github.com/getsentry/sentry-unreal/pull/334), [#344](https://github.com/getsentry/sentry-unreal/pull/344), [#349](https://github.com/getsentry/sentry-unreal/pull/349), [#351](https://github.com/getsentry/sentry-unreal/pull/351), [#355](https://github.com/getsentry/sentry-unreal/pull/355))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#8100)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.8.0...8.10.0)
- Bump CLI from v2.19.4 to v2.20.6 ([#333](https://github.com/getsentry/sentry-unreal/pull/333), [#338](https://github.com/getsentry/sentry-unreal/pull/338), [#341](https://github.com/getsentry/sentry-unreal/pull/341), [#343](https://github.com/getsentry/sentry-unreal/pull/343), [#347](https://github.com/getsentry/sentry-unreal/pull/347), [#364](https://github.com/getsentry/sentry-unreal/pull/364))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2206)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.19.4...2.20.6)

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
- Bump Cocoa SDK (iOS) from v8.6.0 to v8.7.3 ([#272](https://github.com/getsentry/sentry-unreal/pull/272), [#278](https://github.com/getsentry/sentry-unreal/pull/278), [#288](https://github.com/getsentry/sentry-unreal/pull/288))
  - [changelog](https://github.com/getsentry/sentry-cocoa/blob/main/CHANGELOG.md#873)
  - [diff](https://github.com/getsentry/sentry-cocoa/compare/8.6.0...8.7.2)
- Bump Java SDK (Android) from v6.18.1 to v6.19.1 ([#279](https://github.com/getsentry/sentry-unreal/pull/279), [#283](https://github.com/getsentry/sentry-unreal/pull/283))
  - [changelog](https://github.com/getsentry/sentry-java/blob/main/CHANGELOG.md#6191)
  - [diff](https://github.com/getsentry/sentry-java/compare/6.18.1...6.19.1)
- Bump CLI from v2.17.5 to v2.18.1 ([#284](https://github.com/getsentry/sentry-unreal/pull/284), [#286](https://github.com/getsentry/sentry-unreal/pull/286))
  - [changelog](https://github.com/getsentry/sentry-cli/blob/master/CHANGELOG.md#2181)
  - [diff](https://github.com/getsentry/sentry-cli/compare/2.17.5...2.18.1)

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
