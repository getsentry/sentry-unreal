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
        [switch]$SkipReinstall,  # Don't reinstall APK (for preserving crash state)

        [Parameter()]
        [int]$InitialWaitSeconds = 3,  # Wait time before checking for PID

        [Parameter()]
        [int]$PidRetrySeconds = 30,  # Timeout for PID detection (in seconds)

        [Parameter()]
        [int]$LogPollIntervalSeconds = 2  # Logcat polling interval
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
        if ($LASTEXITCODE -ne 0 -or $installOutput -notmatch "Success") {
            throw "Failed to install APK (exit code: $LASTEXITCODE): $installOutput"
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
    Start-Sleep -Seconds $InitialWaitSeconds
    $appPID = $null
    $packageName = $script:PackageName
    for ($i = 0; $i -lt $PidRetrySeconds; $i++) {
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

    # Initialize logCache as array for consistent type handling
    [array]$logCache = @()

    if (-not $appPID) {
        # App might have already exited (fast message test) - capture logs anyway
        Write-Host "Warning: Could not find process ID (app may have exited quickly)" -ForegroundColor Yellow
        $logCache = @(adb -s $device logcat -d 2>&1)
    } else {
        Write-Host "App PID: $appPID" -ForegroundColor Green

        # 6. Monitor logcat for test completion
        $startTime = Get-Date
        $completed = $false

        while ((Get-Date) - $startTime -lt [TimeSpan]::FromSeconds($TimeoutSeconds)) {
            $newLogs = adb -s $device logcat -d --pid=$appPID 2>&1
            if ($newLogs) {
                $logCache = @($newLogs)

                # Check for completion markers from SentryPlaygroundGameInstance
                if (($newLogs | Select-String "TEST_RESULT:") -or
                    ($newLogs | Select-String "Requesting app exit")) {
                    $completed = $true
                    Write-Host "Test completion detected" -ForegroundColor Green
                    break
                }
            }

            Start-Sleep -Seconds $LogPollIntervalSeconds
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

    # Connect to Sentry API
    Write-Host "Connecting to Sentry API..." -ForegroundColor Yellow
    Connect-SentryApi -DSN $script:DSN -ApiToken $script:AuthToken

    # Validate app path
    if (-not (Test-Path $script:ApkPath)) {
        throw "Application not found at: $script:ApkPath"
    }

    # Create output directory
    $script:OutputDir = "$PSScriptRoot/output"
    if (-not (Test-Path $script:OutputDir)) {
        New-Item -ItemType Directory -Path $script:OutputDir | Out-Null
    }

    $script:PackageName = "io.sentry.unreal.sample"
    $script:ActivityName = "$script:PackageName/com.epicgames.unreal.GameActivity"

    # ==========================================
    # RUN 1: Crash test - creates minidump
    # ==========================================
    # The crash is captured but NOT uploaded yet (Android behavior).
    # TODO: Re-enable once Android SDK tag persistence is fixed (`test.crash_id` tag set before crash is not synced to the captured crash on Android)

    # Write-Host "Running crash-capture test (will crash)..." -ForegroundColor Yellow
    # $global:AndroidCrashResult = Invoke-AndroidTestApp -TestName 'crash-capture'

    # Write-Host "Crash test exit code: $($global:AndroidCrashResult.ExitCode)" -ForegroundColor Cyan

    # ==========================================
    # RUN 2: Message test - uploads crash from Run 1 + captures message
    # ==========================================
    # Currently we need to run again so that Sentry sends the crash event captured during the previous app session.
    # TODO: use -SkipReinstall to preserve the crash state.

    Write-Host "Running message-capture test (will upload crash from previous run)..." -ForegroundColor Yellow
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
    #         $CrashResult = $global:AndroidCrashResult
    #         $CrashEvent = $null
    #
    #         # Parse crash event ID from crash run output
    #         $eventIds = Get-EventIds -AppOutput $CrashResult.Output -ExpectedCount 1
    #
    #         if ($eventIds -and $eventIds.Count -gt 0) {
    #             Write-Host "Crash ID captured: $($eventIds[0])" -ForegroundColor Cyan
    #             $crashId = $eventIds[0]
    #
    #             # Fetch crash event using the tag (event was sent during message run)
    #             try {
    #                 $CrashEvent = Get-SentryTestEvent -TagName 'test.crash_id' -TagValue "$crashId"
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
    #         $eventIds = Get-EventIds -AppOutput $CrashResult.Output -ExpectedCount 1
    #         $eventIds | Should -Not -BeNullOrEmpty
    #         $eventIds.Count | Should -Be 1
    #     }
    #
    #     It "Should capture crash event in Sentry (uploaded during next run)" {
    #         $CrashEvent | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have correct event type and platform" {
    #         $CrashEvent.type | Should -Be 'error'
    #         $CrashEvent.platform | Should -Be 'native'
    #     }
    #
    #     It "Should have exception information" {
    #         $CrashEvent.exception | Should -Not -BeNullOrEmpty
    #         $CrashEvent.exception.values | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have stack trace" {
    #         $exception = $CrashEvent.exception.values[0]
    #         $exception.stacktrace | Should -Not -BeNullOrEmpty
    #         $exception.stacktrace.frames | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have user context" {
    #         $CrashEvent.user | Should -Not -BeNullOrEmpty
    #         $CrashEvent.user.username | Should -Be 'TestUser'
    #         $CrashEvent.user.email | Should -Be 'user-mail@test.abc'
    #         $CrashEvent.user.id | Should -Be '12345'
    #     }
    #
    #     It "Should have test.crash_id tag for correlation" {
    #         $tags = $CrashEvent.tags
    #         $crashIdTag = $tags | Where-Object { $_.key -eq 'test.crash_id' }
    #         $crashIdTag | Should -Not -BeNullOrEmpty
    #         $crashIdTag.value | Should -Not -BeNullOrEmpty
    #     }
    #
    #     It "Should have integration test tag" {
    #         $tags = $CrashEvent.tags
    #         ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
    #     }
    #
    #     It "Should have breadcrumbs from before crash" {
    #         $CrashEvent.breadcrumbs | Should -Not -BeNullOrEmpty
    #         $CrashEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
    #     }
    # }

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
                } catch {
                    Write-Host "Failed to fetch message event from Sentry: $_" -ForegroundColor Red
                }
            } else {
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
    Write-Host "Disconnecting from Sentry API..." -ForegroundColor Yellow
    Disconnect-SentryApi
    Write-Host "Integration tests complete" -ForegroundColor Green
}