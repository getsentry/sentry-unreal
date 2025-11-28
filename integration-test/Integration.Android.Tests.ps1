# Integration tests for Sentry Unreal SDK on Android
# Supports both ADB (local devices/emulators) and SauceLabs (cloud devices)
#
# Usage:
#   # Default (uses ADB)
#   Invoke-Pester Integration.Android.Tests.ps1
#
#   # Explicit platform selection with Pester containers
#   $Container = New-PesterContainer -Path 'Integration.Android.Tests.ps1' -Data @{ Platform = 'Adb' }
#   Invoke-Pester -Container $Container
#
#   $Container = New-PesterContainer -Path 'Integration.Android.Tests.ps1' -Data @{ Platform = 'SauceLabs' }
#   Invoke-Pester -Container $Container
#
# Requires:
# - Pre-built APK
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN, SENTRY_UNREAL_TEST_APP_PATH
#
# For ADB:
#   - Android emulator or device connected via ADB
#
# For SauceLabs:
#   - SAUCE_USERNAME, SAUCE_ACCESS_KEY, SAUCE_REGION, SAUCE_DEVICE_NAME

param(
    [Parameter(Mandatory = $false)]
    [ValidateSet('Adb', 'SauceLabs')]
    [string]$Platform = 'Adb'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

BeforeAll {
    # Map friendly platform name to app-runner platform name
    $appRunnerPlatform = if ($Platform -eq 'Adb') { 'AndroidAdb' } else { 'AndroidSauceLabs' }

    Write-Host "Running Android tests with platform: $Platform (app-runner: $appRunnerPlatform)" -ForegroundColor Cyan

    # Check if configuration file exists
    $configFile = "$PSScriptRoot/TestConfig.local.ps1"
    if (-not (Test-Path $configFile)) {
        throw "Configuration file '$configFile' not found. Run 'cmake -B build -S .' first"
    }

    # Load configuration (provides $global:AppRunnerPath)
    . $configFile

    # Import app-runner modules (SentryApiClient, test utilities)
    . "$global:AppRunnerPath/import-modules.ps1"

    # Validate environment variables (test-specific only, not provider-specific)
    $script:DSN = $env:SENTRY_UNREAL_TEST_DSN
    $script:AuthToken = $env:SENTRY_AUTH_TOKEN
    $script:ApkPath = $env:SENTRY_UNREAL_TEST_APP_PATH

    if (-not $script:DSN) {
        throw "Environment variable SENTRY_UNREAL_TEST_DSN must be set"
    }

    if (-not $script:AuthToken) {
        throw "Environment variable SENTRY_AUTH_TOKEN must be set"
    }

    if (-not $script:ApkPath) {
        throw "Environment variable SENTRY_UNREAL_TEST_APP_PATH must be set"
    }

    # Validate app path
    if (-not (Test-Path $script:ApkPath)) {
        throw "Application not found at: $script:ApkPath"
    }

    # Connect to Sentry API
    Write-Host "Connecting to Sentry API..." -ForegroundColor Yellow
    Connect-SentryApi -DSN $script:DSN -ApiToken $script:AuthToken

    # Create output directory
    $script:OutputDir = "$PSScriptRoot/output"
    if (-not (Test-Path $script:OutputDir)) {
        New-Item -ItemType Directory -Path $script:OutputDir | Out-Null
    }

    $script:PackageName = "io.sentry.unreal.sample"
    $script:ActivityName = "$script:PackageName/com.epicgames.unreal.GameActivity"

    # Connect to Android device (provider validates its own env vars)
    Write-Host "Connecting to Android via $Platform..." -ForegroundColor Yellow
    Connect-Device -Platform $appRunnerPlatform

    # Install APK
    Write-Host "Installing APK via $Platform..." -ForegroundColor Yellow
    Install-DeviceApp -Path $script:ApkPath

    # ==========================================
    # RUN 1: Crash test - creates minidump
    # ==========================================
    # The crash is captured but NOT uploaded yet (Android behavior).
    # TODO: Re-enable once Android SDK tag persistence is fixed (`test.crash_id` tag set before crash is not synced to the captured crash on Android)

    Write-Host "Running crash-capture test (will crash)..." -ForegroundColor Yellow
    $cmdlineCrashArgs = "-e cmdline -crash-capture"
    $global:AndroidCrashResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $cmdlineCrashArgs

    Write-Host "Crash test exit code: $($global:AndroidCrashResult.ExitCode)" -ForegroundColor Cyan

    # ==========================================
    # RUN 2: Message test - uploads crash from Run 1 + captures message
    # ==========================================
    # Currently we need to run again so that Sentry sends the crash event captured during the previous app session.

    Write-Host "Running message-capture test on $Platform..." -ForegroundColor Yellow
    $cmdlineMessageArgs = "-e cmdline -message-capture"
    $global:AndroidMessageResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $cmdlineMessageArgs

    Write-Host "Message test exit code: $($global:AndroidMessageResult.ExitCode)" -ForegroundColor Cyan
}

Describe "Sentry Unreal Android Integration Tests ($Platform)" {

    # ==========================================
    # NOTE: Crash Capture Tests are DISABLED due to tag sync issue
    # Uncomment when Android SDK tag persistence is fixed
    # ==========================================
    Context "Crash Capture Tests" {
        BeforeAll {
            # Crash event is sent during the MESSAGE run (Run 2)
            # But the crash_id comes from the CRASH run (Run 1)
            $CrashResult = $global:AndroidCrashResult
            $CrashEvent = $null
    
            # Parse crash event ID from crash run output
            $eventIds = Get-EventIds -AppOutput $CrashResult.Output -ExpectedCount 1
    
            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Crash ID captured: $($eventIds[0])" -ForegroundColor Cyan
                $crashId = $eventIds[0]
    
                # Fetch crash event using the tag (event was sent during message run)
                try {
                    $CrashEvent = Get-SentryTestEvent -TagName 'test.crash_id' -TagValue "$crashId"
                    Write-Host "Crash event fetched from Sentry successfully" -ForegroundColor Green
                } catch {
                    Write-Host "Failed to fetch crash event from Sentry: $_" -ForegroundColor Red
                }
            } else {
                Write-Host "Warning: No crash event ID found in output" -ForegroundColor Yellow
            }
        }
    
        It "Should output event ID before crash" {
            $eventIds = Get-EventIds -AppOutput $CrashResult.Output -ExpectedCount 1
            $eventIds | Should -Not -BeNullOrEmpty
            $eventIds.Count | Should -Be 1
        }
    
        It "Should capture crash event in Sentry (uploaded during next run)" {
            $CrashEvent | Should -Not -BeNullOrEmpty
        }
    
        It "Should have correct event type and platform" {
            $CrashEvent.type | Should -Be 'error'
            $CrashEvent.platform | Should -Be 'native'
        }
    
        It "Should have exception information" {
            $CrashEvent.exception | Should -Not -BeNullOrEmpty
            $CrashEvent.exception.values | Should -Not -BeNullOrEmpty
        }
    
        It "Should have stack trace" {
            $exception = $CrashEvent.exception.values[0]
            $exception.stacktrace | Should -Not -BeNullOrEmpty
            $exception.stacktrace.frames | Should -Not -BeNullOrEmpty
        }
    
        It "Should have user context" {
            $CrashEvent.user | Should -Not -BeNullOrEmpty
            $CrashEvent.user.username | Should -Be 'TestUser'
            $CrashEvent.user.email | Should -Be 'user-mail@test.abc'
            $CrashEvent.user.id | Should -Be '12345'
        }
    
        It "Should have test.crash_id tag for correlation" {
            $tags = $CrashEvent.tags
            $crashIdTag = $tags | Where-Object { $_.key -eq 'test.crash_id' }
            $crashIdTag | Should -Not -BeNullOrEmpty
            $crashIdTag.value | Should -Not -BeNullOrEmpty
        }
    
        It "Should have integration test tag" {
            $tags = $CrashEvent.tags
            ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
        }
    
        It "Should have breadcrumbs from before crash" {
            $CrashEvent.breadcrumbs | Should -Not -BeNullOrEmpty
            $CrashEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
        }
    }

    Context "Message Capture Tests" {
        BeforeAll {
            $MessageResult = $global:AndroidMessageResult
            $MessageEvent = $null

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $MessageResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Message event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                # Fetch event from Sentry (with polling)
                try {
                    $MessageEvent = Get-SentryTestEvent -EventId $eventIds[0]
                    Write-Host "Message event fetched from Sentry successfully" -ForegroundColor Green
                }
                catch {
                    Write-Host "Failed to fetch message event from Sentry: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No message event ID found in output" -ForegroundColor Yellow
            }
        }

        It "Should output event ID" {
            $eventIds = Get-EventIds -AppOutput $MessageResult.Output -ExpectedCount 1
            $eventIds | Should -Not -BeNullOrEmpty
            $eventIds.Count | Should -Be 1
        }

        It "Should output TEST_RESULT with success" {
            $testResultLine = $MessageResult.Output | Where-Object { $_ -match 'TEST_RESULT:' }
            $testResultLine | Should -Not -BeNullOrEmpty
            $testResultLine | Should -Match '"success"\s*:\s*true'
        }

        It "Should capture message event in Sentry" {
            $MessageEvent | Should -Not -BeNullOrEmpty
        }

        It "Should have correct platform" {
            # Android events are captured from Java layer, so platform is 'java' not 'native'
            $MessageEvent.platform | Should -Be 'java'
        }

        It "Should have message content" {
            $MessageEvent.message | Should -Not -BeNullOrEmpty
            $MessageEvent.message.formatted | Should -Match 'Integration test message'
        }

        It "Should have user context" {
            $MessageEvent.user | Should -Not -BeNullOrEmpty
            $MessageEvent.user.username | Should -Be 'TestUser'
        }

        It "Should have integration test tag" {
            $tags = $MessageEvent.tags
            ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
        }

        It "Should have breadcrumbs" {
            $MessageEvent.breadcrumbs | Should -Not -BeNullOrEmpty
            $MessageEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
        }
    }
}

AfterAll {
    # Disconnect from Android device
    Write-Host "Disconnecting from $Platform..." -ForegroundColor Yellow
    Disconnect-Device

    # Disconnect from Sentry API
    Write-Host "Disconnecting from Sentry API..." -ForegroundColor Yellow
    Disconnect-SentryApi

    Write-Host "Integration tests complete" -ForegroundColor Green
}
