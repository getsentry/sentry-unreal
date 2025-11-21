# Integration tests for Sentry Unreal SDK on Android
# Requires:
# - Pre-built APK (x64 for emulator)
# - Android emulator or device connected
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN, SENTRY_UNREAL_TEST_APP_PATH

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function script:Get-AndroidDeviceId {
    $lines = adb devices | Select-String "device$"

    if (-not $lines) {
        throw "No Android devices found. Is emulator running?"
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

function script:Install-AndroidApp {
    param(
        [Parameter(Mandatory)]
        [string]$ApkPath,

        [Parameter(Mandatory)]
        [string]$PackageName,

        [Parameter(Mandatory)]
        [string]$DeviceId
    )

    if (-not (Test-Path $ApkPath)) {
        throw "APK file not found: $ApkPath"
    }

    if ($ApkPath -notlike '*.apk') {
        throw "Package must be an .apk file. Got: $ApkPath"
    }

    # Check for existing installation
    Write-Debug "Checking for existing package: $PackageName"
    $installed = adb -s $DeviceId shell pm list packages | Select-String -Pattern $PackageName -SimpleMatch

    if ($installed) {
        Write-Host "Uninstalling previous version..." -ForegroundColor Yellow
        adb -s $DeviceId uninstall $PackageName | Out-Null
        Start-Sleep -Seconds 1
    }

    # Install APK
    Write-Host "Installing APK to device: $DeviceId" -ForegroundColor Yellow
    $installOutput = adb -s $DeviceId install -r $ApkPath 2>&1 | Out-String

    if ($LASTEXITCODE -ne 0 -or $installOutput -notmatch "Success") {
        throw "Failed to install APK (exit code: $LASTEXITCODE): $installOutput"
    }

    Write-Host "Package installed successfully: $PackageName" -ForegroundColor Green
}

function script:Invoke-AndroidApp {
    param(
        [Parameter(Mandatory)]
        [string]$ExecutablePath,

        [Parameter()]
        [string]$Arguments = ""
    )

    # Extract package name from activity path (format: package.name/activity.name)
    if ($ExecutablePath -notmatch '^([^/]+)/') {
        throw "ExecutablePath must be in format 'package.name/activity.name'. Got: $ExecutablePath"
    }
    $packageName = $matches[1]

    # Use script-level Android configuration
    $deviceId = $script:DeviceId
    $outputDir = $script:OutputDir

    # Android-specific timeout configuration
    $timeoutSeconds = 300
    $initialWaitSeconds = 3
    $pidRetrySeconds = 30
    $logPollIntervalSeconds = 2

    $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $logFile = if ($OutputDir) { "$OutputDir/$timestamp-logcat.txt" } else { $null }

    # Clear logcat before launch
    Write-Debug "Clearing logcat on device: $deviceId"
    adb -s $deviceId logcat -c

    # Launch activity with Intent extras
    Write-Host "Launching: $ExecutablePath" -ForegroundColor Cyan
    if ($Arguments) {
        Write-Host "  Arguments: $Arguments" -ForegroundColor Cyan
    }

    $startOutput = adb -s $deviceId shell am start -n $ExecutablePath $Arguments -W 2>&1 | Out-String

    if ($startOutput -match "Error") {
        throw "Failed to start activity: $startOutput"
    }

    # Get process ID (with retries)
    Write-Debug "Waiting for app process..."
    Start-Sleep -Seconds $initialWaitSeconds

    $appPID = $null
    for ($i = 0; $i -lt $pidRetrySeconds; $i++) {
        $pidOutput = adb -s $deviceId shell pidof $packageName 2>&1
        if ($pidOutput) {
            $pidOutput = $pidOutput.ToString().Trim()
            if ($pidOutput -match '^\d+$') {
                $appPID = $pidOutput
                break
            }
        }
        Start-Sleep -Seconds 1
    }

    # Initialize log cache as array for consistent type handling
    [array]$logCache = @()

    if (-not $appPID) {
        # App might have already exited (fast message test) - capture logs anyway
        Write-Host "Warning: Could not find process ID (app may have exited quickly)" -ForegroundColor Yellow
        $logCache = @(adb -s $deviceId logcat -d 2>&1)
        $exitCode = 0
    } else {
        Write-Host "App PID: $appPID" -ForegroundColor Green

        # Monitor logcat for test completion
        $startTime = Get-Date
        $completed = $false

        while ((Get-Date) - $startTime -lt [TimeSpan]::FromSeconds($timeoutSeconds)) {
            $newLogs = adb -s $deviceId logcat -d --pid=$appPID 2>&1
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

            Start-Sleep -Seconds $logPollIntervalSeconds
        }

        if (-not $completed) {
            Write-Host "Warning: Test did not complete within timeout" -ForegroundColor Yellow
        }

        $exitCode = 0  # Android apps don't report exit codes via adb
    }

    # Save logcat to file if OutputDir specified
    if ($logFile) {
        $logCache | Out-File $logFile
        Write-Host "Logcat saved to: $logFile"
    }

    # Return structured result (matches app-runner pattern)
    return @{
        ExitCode = $exitCode
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

    # Get Android device
    $script:DeviceId = Get-AndroidDeviceId
    Write-Host "Found Android device: $script:DeviceId" -ForegroundColor Green

    # Install APK to device
    Write-Host "Installing APK to Android device..." -ForegroundColor Yellow
    Install-AndroidApp -ApkPath $script:ApkPath -PackageName $script:PackageName -DeviceId $script:DeviceId

    # ==========================================
    # RUN 1: Crash test - creates minidump
    # ==========================================
    # The crash is captured but NOT uploaded yet (Android behavior).
    # TODO: Re-enable once Android SDK tag persistence is fixed (`test.crash_id` tag set before crash is not synced to the captured crash on Android)

    # Write-Host "Running crash-capture test (will crash)..." -ForegroundColor Yellow
    # $cmdlineCrashArgs = "-e cmdline '-crash-capture'"
    # $global:AndroidCrashResult = Invoke-AndroidApp -ExecutablePath $script:ActivityName -Arguments $cmdlineCrashArgs

    # Write-Host "Crash test exit code: $($global:AndroidCrashResult.ExitCode)" -ForegroundColor Cyan

    # ==========================================
    # RUN 2: Message test - uploads crash from Run 1 + captures message
    # ==========================================
    # Currently we need to run again so that Sentry sends the crash event captured during the previous app session.
    # TODO: use -SkipReinstall to preserve the crash state.

    Write-Host "Running message-capture test (will upload crash from previous run)..." -ForegroundColor Yellow
    $cmdlineMessageArgs = "-e cmdline '-message-capture'"
    $global:AndroidMessageResult = Invoke-AndroidApp -ExecutablePath $script:ActivityName -Arguments $cmdlineMessageArgs

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