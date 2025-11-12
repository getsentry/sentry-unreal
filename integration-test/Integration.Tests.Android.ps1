# Integration tests for Sentry Unreal SDK on Android
# Requires:
# - Pre-built APK (x64 for emulator)
# - Android emulator or device connected
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN, SENTRY_UNREAL_TEST_APP_PATH

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function script:Get-AndroidDeviceId {
    # Get lines that end with "device" (not "offline" or "unauthorized")
    $lines = adb devices | Select-String "device$"

    if (-not $lines) {
        throw "No Android devices found"
    }

    # Extract device ID from the first matching line
    # Line format: "emulator-5554	device"
    $firstLine = $lines | Select-Object -First 1
    $deviceId = ($firstLine.Line -split '\s+')[0]

    if (-not $deviceId) {
        throw "Could not extract device ID from: $($firstLine.Line)"
    }

    return $deviceId
}

function script:Invoke-AndroidTestApp {
    param(
        [Parameter(Mandatory)]
        [string]$TestName,  # 'crash-capture' or 'message-capture'

        [Parameter()]
        [int]$TimeoutSeconds = 300,

        [Parameter()]
        [switch]$SkipReinstall  # Don't reinstall APK (for preserving crash state)
    )

    $device = Get-AndroidDeviceId
    $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $logFile = "$script:OutputDir/$timestamp-$TestName-logcat.txt"

    Write-Host "Running test: $TestName on device: $device" -ForegroundColor Yellow

    if (-not $SkipReinstall) {
        # 1. Uninstall previous installation (to ensure clean state)
        $packageName = $script:PackageName
        $installed = adb -s $device shell pm list packages | Select-String -Pattern $packageName -SimpleMatch
        if ($installed) {
            Write-Host "Uninstalling previous version..."
            adb -s $device uninstall $packageName | Out-Null
            Start-Sleep -Seconds 1
        }

        # 2. Install APK
        Write-Host "Installing APK..."
        $installOutput = adb -s $device install -r $script:ApkPath 2>&1 | Out-String
        if ($installOutput -notmatch "Success") {
            throw "Failed to install APK: $installOutput"
        }
    } else {
        Write-Host "Skipping reinstall (preserving crash state)..." -ForegroundColor Cyan
    }

    # 3. Clear logcat
    adb -s $device logcat -c

    # 4. Start activity with Intent extras
    Write-Host "Starting activity with Intent extras: -e test $TestName"
    $startOutput = adb -s $device shell am start -n $script:ActivityName -e test $TestName -W 2>&1 | Out-String

    if ($startOutput -match "Error") {
        throw "Failed to start activity: $startOutput"
    }

    # 5. Get process ID (with retries)
    Write-Host "Waiting for app process..."
    Start-Sleep -Seconds 3
    $appPID = $null
    $packageName = $script:PackageName
    for ($i = 0; $i -lt 30; $i++) {
        $pidOutput = adb -s $device shell pidof $packageName 2>&1
        if ($pidOutput) {
            $pidOutput = $pidOutput.ToString().Trim()
            if ($pidOutput -match '^\d+$') {
                $appPID = $pidOutput
                break
            }
        }
        Start-Sleep -Seconds 1
    }

    if (-not $appPID) {
        # App might have already exited (fast message test) - capture logs anyway
        Write-Host "Warning: Could not find process ID (app may have exited quickly)" -ForegroundColor Yellow
        $logCache = adb -s $device logcat -d 2>&1
    } else {
        Write-Host "App PID: $appPID" -ForegroundColor Green

        # 6. Monitor logcat for test completion
        $logCache = @()
        $startTime = Get-Date
        $completed = $false

        while ((Get-Date) - $startTime -lt [TimeSpan]::FromSeconds($TimeoutSeconds)) {
            $newLogs = adb -s $device logcat -d --pid=$appPID 2>&1
            if ($newLogs) {
                $logCache = $newLogs

                # Check for completion markers from SentryPlaygroundGameInstance
                if (($newLogs | Select-String "TEST_RESULT:") -or
                    ($newLogs | Select-String "Requesting app exit")) {
                    $completed = $true
                    Write-Host "Test completion detected" -ForegroundColor Green
                    break
                }
            }

            Start-Sleep -Seconds 2
        }

        if (-not $completed) {
            Write-Host "Warning: Test did not complete within timeout" -ForegroundColor Yellow
        }
    }

    # Save full logcat to file
    $logCache | Out-File $logFile
    Write-Host "Logcat saved to: $logFile"

    # 7. Return structured result
    return @{
        ExitCode = if ($TestName -eq 'crash-capture') { -1 } else { 0 }  # Simulate crash exit code
        Output = $logCache
        Error = @()
    }
}

BeforeAll {
    # Package name is defined in sample/Config/DefaultEngine.ini
    # If this changes, update this constant
    $script:PackageName = "io.sentry.unreal.sample"
    $script:ActivityName = "$script:PackageName/com.epicgames.unreal.GameActivity"

    # Check if configuration file exists
    $configFile = "$PSScriptRoot/TestConfig.local.ps1"
    if (-not (Test-Path $configFile)) {
        throw "Configuration file '$configFile' not found. Run 'cmake -B build -S .' first"
    }

    # Load configuration (provides $global:AppRunnerPath)
    . $configFile

    # Import app-runner modules (SentryApiClient, test utilities)
    . "$global:AppRunnerPath/import-modules.ps1"

    # Validate environment variables
    $script:DSN = $env:SENTRY_UNREAL_TEST_DSN
    $script:AuthToken = $env:SENTRY_AUTH_TOKEN

    if (-not $script:DSN) {
        throw "Environment variable SENTRY_UNREAL_TEST_DSN must be set"
    }

    if (-not $script:AuthToken) {
        throw "Environment variable SENTRY_AUTH_TOKEN must be set"
    }

    # Resolve APK path from environment variable
    $ApkBasePath = $env:SENTRY_UNREAL_TEST_APP_PATH
    if (-not $ApkBasePath) {
        throw "Environment variable SENTRY_UNREAL_TEST_APP_PATH must be set"
    }

    if ($ApkBasePath -match "\.apk$") {
        # Direct path to APK
        $script:ApkPath = $ApkBasePath
    } else {
        # Path to directory containing APKs - pick x64 for emulator
        $script:ApkPath = Join-Path $ApkBasePath "SentryPlayground-x64.apk"
    }

    if (-not (Test-Path $script:ApkPath)) {
        throw "APK not found at: $script:ApkPath"
    }

    Write-Host "APK: $script:ApkPath" -ForegroundColor Cyan

    # Check adb and device
    try {
        $device = Get-AndroidDeviceId
        Write-Host "Found Android device: $device" -ForegroundColor Green
    } catch {
        throw "No Android devices found. Is emulator running?"
    }

    # Connect to Sentry API
    Write-Host "Connecting to Sentry API..." -ForegroundColor Yellow
    Connect-SentryApi -DSN $script:DSN -ApiToken $script:AuthToken

    # Create output directory
    $script:OutputDir = "$PSScriptRoot/output"
    if (-not (Test-Path $script:OutputDir)) {
        New-Item -ItemType Directory -Path $script:OutputDir | Out-Null
    }

    # ==========================================
    # NOTE: Crash test is currently DISABLED due to tag sync issue
    # The test.crash_id tag set before crash is not synced to the captured event on Android
    # TODO: Re-enable once Android SDK tag persistence is fixed
    # ==========================================
    # # RUN 1: Crash test - creates minidump
    # # The crash is captured but NOT uploaded yet (Android behavior)
    # Write-Host "`n=== Running crash-capture test (will crash) ===" -ForegroundColor Yellow
    # $global:AndroidCrashResult = Invoke-AndroidTestApp -TestName 'crash-capture'
    # Write-Host "Crash test exit code: $($global:AndroidCrashResult.ExitCode)" -ForegroundColor Cyan
    #
    # # RUN 2: Message test - uploads crash from Run 1 + captures message
    # # Android Sentry SDK uploads previous crash on next app start
    # # Use -SkipReinstall to preserve the crash state
    # Write-Host "`n=== Running message-capture test (will upload crash from previous run) ===" -ForegroundColor Yellow
    # $global:AndroidMessageResult = Invoke-AndroidTestApp -TestName 'message-capture' -SkipReinstall
    # Write-Host "Message test exit code: $($global:AndroidMessageResult.ExitCode)" -ForegroundColor Cyan

    # ==========================================
    # RUN: Message test only (crash test disabled)
    # ==========================================
    Write-Host "`n=== Running message-capture test ===" -ForegroundColor Yellow
    $global:AndroidMessageResult = Invoke-AndroidTestApp -TestName 'message-capture'
    Write-Host "Message test exit code: $($global:AndroidMessageResult.ExitCode)" -ForegroundColor Cyan
}

Describe "Sentry Unreal Android Integration Tests" {

    # ==========================================
    # NOTE: Crash Capture Tests are DISABLED due to tag sync issue
    # Uncomment when Android SDK tag persistence is fixed
    # ==========================================
    # Context "Crash Capture Tests" {
    #     BeforeAll {
    #         # Crash event is sent during the MESSAGE run (Run 2)
    #         # But the crash_id comes from the CRASH run (Run 1)
    #         $script:CrashResult = $global:AndroidCrashResult
    #         $script:CrashEvent = $null
    #
    #         # Parse crash event ID from crash run output
    #         $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1
    #
    #         if ($eventIds -and $eventIds.Count -gt 0) {
    #             Write-Host "Crash ID captured: $($eventIds[0])" -ForegroundColor Cyan
    #             $crashId = $eventIds[0]
    #
    #             # Fetch crash event using the tag (event was sent during message run)
    #             try {
    #                 $script:CrashEvent = Get-SentryTestEvent -TagName 'test.crash_id' -TagValue "$crashId"
    #                 Write-Host "Crash event fetched from Sentry successfully" -ForegroundColor Green
    #             } catch {
    #                 Write-Host "Failed to fetch crash event from Sentry: $_" -ForegroundColor Red
    #             }
    #         } else {
    #             Write-Host "Warning: No crash event ID found in output" -ForegroundColor Yellow
    #         }
    #     }
    #
    #     It "Should output event ID before crash" {
    #         $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1
    #         $eventIds | Should -Not -BeNullOrEmpty
    #         $eventIds.Count | Should -Be 1
    #     }
    #
    #     It "Should capture crash event in Sentry (uploaded during next run)" {
    #         $script:CrashEvent | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have correct event type and platform" {
    #         $script:CrashEvent.type | Should -Be 'error'
    #         $script:CrashEvent.platform | Should -Be 'native'
    #     }
    #
    #     It "Should have exception information" {
    #         $script:CrashEvent.exception | Should -Not -BeNullOrEmpty
    #         $script:CrashEvent.exception.values | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have stack trace" {
    #         $exception = $script:CrashEvent.exception.values[0]
    #         $exception.stacktrace | Should -Not -BeNullOrEmpty
    #         $exception.stacktrace.frames | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have user context" {
    #         $script:CrashEvent.user | Should -Not -BeNullOrEmpty
    #         $script:CrashEvent.user.username | Should -Be 'TestUser'
    #         $script:CrashEvent.user.email | Should -Be 'user-mail@test.abc'
    #         $script:CrashEvent.user.id | Should -Be '12345'
    #     }
    #
    #     It "Should have test.crash_id tag for correlation" {
    #         $tags = $script:CrashEvent.tags
    #         $crashIdTag = $tags | Where-Object { $_.key -eq 'test.crash_id' }
    #         $crashIdTag | Should -Not -BeNullOrEmpty
    #         $crashIdTag.value | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have integration test tag" {
    #         $tags = $script:CrashEvent.tags
    #         ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
    #     }
    #
    #     It "Should have breadcrumbs from before crash" {
    #         $script:CrashEvent.breadcrumbs | Should -Not -BeNullOrEmpty
    #         $script:CrashEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
    #     }
    # }

    Context "Message Capture Tests" {
        BeforeAll {
            $script:MessageResult = $global:AndroidMessageResult
            $script:MessageEvent = $null

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $script:MessageResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Message event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                # Fetch event from Sentry (with polling)
                try {
                    $script:MessageEvent = Get-SentryTestEvent -EventId $eventIds[0]
                    Write-Host "Message event fetched from Sentry successfully" -ForegroundColor Green
                } catch {
                    Write-Host "Failed to fetch message event from Sentry: $_" -ForegroundColor Red
                }
            } else {
                Write-Host "Warning: No message event ID found in output" -ForegroundColor Yellow
            }
        }

        It "Should output event ID" {
            $eventIds = Get-EventIds -AppOutput $script:MessageResult.Output -ExpectedCount 1
            $eventIds | Should -Not -BeNullOrEmpty
            $eventIds.Count | Should -Be 1
        }

        It "Should output TEST_RESULT with success" {
            $testResultLine = $script:MessageResult.Output | Where-Object { $_ -match 'TEST_RESULT:' }
            $testResultLine | Should -Not -BeNullOrEmpty
            $testResultLine | Should -Match '"success"\s*:\s*true'
        }

        It "Should capture message event in Sentry" {
            $script:MessageEvent | Should -Not -BeNullOrEmpty
        }

        It "Should have correct platform" {
            # Android events are captured from Java layer, so platform is 'java' not 'native'
            $script:MessageEvent.platform | Should -Be 'java'
        }

        It "Should have message content" {
            $script:MessageEvent.message | Should -Not -BeNullOrEmpty
            $script:MessageEvent.message.formatted | Should -Match 'Integration test message'
        }

        It "Should have user context" {
            $script:MessageEvent.user | Should -Not -BeNullOrEmpty
            $script:MessageEvent.user.username | Should -Be 'TestUser'
        }

        It "Should have integration test tag" {
            $tags = $script:MessageEvent.tags
            ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
        }

        It "Should have breadcrumbs" {
            $script:MessageEvent.breadcrumbs | Should -Not -BeNullOrEmpty
            $script:MessageEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
        }
    }
}

AfterAll {
    Write-Host "Disconnecting from Sentry API..." -ForegroundColor Yellow
    Disconnect-SentryApi
    Write-Host "Integration tests complete" -ForegroundColor Green
}