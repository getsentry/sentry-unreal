# Integration tests for Sentry Unreal SDK on desktop platforms
#
# Usage:
#   Invoke-Pester Integration.Desktop.Tests.ps1
#
# Requires:
# - Pre-built SentryPlayground application
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN, SENTRY_UNREAL_TEST_APP_PATH

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

BeforeDiscovery {
    # Detect current platform and add as test target
    function Get-CurrentDesktopPlatform {
        if ($IsWindows) {
            return 'Windows'
        }
        elseif ($IsMacOS) {
            return 'MacOS'
        }
        elseif ($IsLinux) {
            return 'Linux'
        }
        else {
            return $null
        }
    }

    # Define test targets
    function Get-TestTarget {
        param(
            [string]$Platform,
            [string]$ProviderName
        )

        return @{
            Platform     = $Platform
            ProviderName = $ProviderName
        }
    }

    # Only test the current desktop platform
    $TestTargets = @()
    $currentPlatform = Get-CurrentDesktopPlatform
    $currentPlatform | Should -Not -Be $null
    $TestTargets += Get-TestTarget -Platform $currentPlatform -ProviderName $currentPlatform

    # Define crash types to test
    $TestCrashTypes = @(
        @{ Name = 'NullPointer'; Arg = '-crash-capture' }
        @{ Name = 'StackOverflow'; Arg = '-crash-stack-overflow' }
        @{ Name = 'MemoryCorruption'; Arg = '-crash-memory-corruption' }
    )
}

BeforeAll {
    # Check if configuration file exists
    $configFile = "$PSScriptRoot/TestConfig.local.ps1"
    if (-not (Test-Path $configFile)) {
        throw "Configuration file '$configFile' not found."
    }

    # Load configuration (provides $global:AppRunnerPath)
    . $configFile

    # Import app-runner modules
    . "$global:AppRunnerPath/import-modules.ps1"

    # Validate environment variables
    $script:DSN = $env:SENTRY_UNREAL_TEST_DSN
    $script:AuthToken = $env:SENTRY_AUTH_TOKEN
    $script:AppPath = $env:SENTRY_UNREAL_TEST_APP_PATH

    if (-not $script:DSN) {
        throw "Environment variable SENTRY_UNREAL_TEST_DSN must be set"
    }

    if (-not $script:AuthToken) {
        throw "Environment variable SENTRY_AUTH_TOKEN must be set"
    }

    if (-not $script:AppPath) {
        throw "Environment variable SENTRY_UNREAL_TEST_APP_PATH must be set"
    }

    # Validate app path
    if (-not (Test-Path $script:AppPath)) {
        throw "Application not found at: $script:AppPath"
    }

    # Connect to Sentry API
    Write-Host "Connecting to Sentry API..." -ForegroundColor Yellow
    Connect-SentryApi -DSN $script:DSN -ApiToken $script:AuthToken

    # Create output directory
    $script:OutputDir = "$PSScriptRoot/output"
    if (-not (Test-Path $script:OutputDir)) {
        New-Item -ItemType Directory -Path $script:OutputDir | Out-Null
    }
}

Describe "Sentry Unreal Desktop Integration Tests (<Platform>)" -ForEach $TestTargets {

    BeforeAll {
        # Connect to desktop device (required to create corresponding provider)
        Write-Host "Connecting to $Platform..." -ForegroundColor Yellow
        Connect-Device -Platform $Platform
    }

    AfterAll {
        # Disconnect from desktop device
        Write-Host "Disconnecting from $Platform..." -ForegroundColor Yellow
        Disconnect-Device

        Write-Host "Integration tests complete on $Platform" -ForegroundColor Green
    }

    Context "Crash Capture Tests - <Name>" -ForEach $TestCrashTypes {
        BeforeAll {
            $crashTypeName = $_.Name
            $crashTypeArg = $_.Arg

            $script:CrashResult = $null
            $script:CrashEvent = $null

            Write-Host "Running $crashTypeName crash test..." -ForegroundColor Yellow

            $appArgs = @(
                '-nullrhi',     # Runs without graphics rendering (headless mode)
                '-unattended',  # Disables user prompts and interactive dialogs
                '-stdout',      # Ensures logs are written to stdout on Linux/Unix systems
                '-nosplash'     # Prevents splash screen and dialogs
            )

            # Override default project settings
            $appArgs += "-ini:Engine:[/Script/Sentry.SentrySettings]:Dsn=$script:DSN"               # Prevents double initialization
            $appArgs += "-ini:Engine:[/Script/Sentry.SentrySettings]:EnableOnCrashLogging=True"     # Enables crash logging
            $appArgs += "-ini:Engine:[/Script/Sentry.SentrySettings]:EnableAutoLogAttachment=True"  # Enables log attachment

            # $crashTypeArg triggers specific crash type scenario in the sample app
            $script:CrashResult = Invoke-DeviceApp -ExecutablePath $script:AppPath -Arguments ((@($crashTypeArg) + $appArgs) -join ' ')

            # On macOS, the crash is captured but not uploaded immediately (due to Cocoa's behavior),
            # so we need to run the test app again to send it to Sentry.
            # -init-only allows starting the app to flush captured events and quit right after
            if ($Platform -eq 'MacOS') {
                Invoke-DeviceApp -ExecutablePath $script:AppPath -Arguments ((@('-init-only') + $appArgs) -join ' ')
            }

            Write-Host "$crashTypeName crash test executed. Exit code: $($script:CrashResult.ExitCode)" -ForegroundColor Cyan

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                $crashId = $eventIds[0]

                # Fetch event from Sentry (with polling)
                try {
                    $script:CrashEvent = Get-SentryTestEvent -TagName 'test.crash_id' -TagValue "$crashId"
                    Write-Host "Event fetched from Sentry successfully" -ForegroundColor Green
                }
                catch {
                    Write-Host "Failed to fetch event from Sentry: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No event ID found in output" -ForegroundColor Yellow
            }
        }

        It "Should have non-zero exit code" {
            $script:CrashResult.ExitCode | Should -Not -Be 0
        }

        It "Should output event ID" {
            $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1
            $eventIds | Should -Not -BeNullOrEmpty
            $eventIds.Count | Should -Be 1
        }

        It "Should capture crash event in Sentry" {
            $script:CrashEvent | Should -Not -BeNullOrEmpty
        }

        It "Should have correct event type and platform" {
            $script:CrashEvent.type | Should -Be 'error'
            if ($Platform -eq 'MacOS') {
                $script:CrashEvent.platform | Should -Be 'cocoa'
            }
            else {
                $script:CrashEvent.platform | Should -Be 'native'
            }
        }

        It "Should have exception information" {
            $script:CrashEvent.exception | Should -Not -BeNullOrEmpty
            $script:CrashEvent.exception.values | Should -Not -BeNullOrEmpty
        }

        It "Should have stack trace" {
            $exception = $script:CrashEvent.exception.values[0]
            $exception.stacktrace | Should -Not -BeNullOrEmpty
            $exception.stacktrace.frames | Should -Not -BeNullOrEmpty
        }

        It "Should have user context" {
            $script:CrashEvent.user | Should -Not -BeNullOrEmpty
            $script:CrashEvent.user.username | Should -Be 'TestUser'
            $script:CrashEvent.user.email | Should -Be 'user-mail@test.abc'
            $script:CrashEvent.user.id | Should -Be '12345'
        }

        It "Should have integration test tag" {
            $tags = $script:CrashEvent.tags
            ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
        }

        It "Should have breadcrumbs" {
            $script:CrashEvent.breadcrumbs | Should -Not -BeNullOrEmpty
            $script:CrashEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
        }
    }

    Context "Message Capture Tests" {
        BeforeAll {
            $script:MessageResult = $null
            $script:MessageEvent = $null

            Write-Host "Running message capture test..." -ForegroundColor Yellow

            $appArgs = @(
                '-nullrhi',     # Runs without graphics rendering (headless mode)
                '-unattended',  # Disables user prompts and interactive dialogs
                '-stdout',      # Ensures logs are written to stdout on Linux/Unix systems
                '-nosplash'     # Prevents splash screen and dialogs
            )

            # Override default project settings to avoid double initialization
            $appArgs += "-ini:Engine:[/Script/Sentry.SentrySettings]:Dsn=$script:DSN"

            # -message-capture triggers integration test message scenario in the sample app
            $script:MessageResult = Invoke-DeviceApp -ExecutablePath $script:AppPath -Arguments ((@('-message-capture') + $appArgs) -join ' ')

            Write-Host "Message test executed. Exit code: $($script:MessageResult.ExitCode)" -ForegroundColor Cyan

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $script:MessageResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                # Fetch event from Sentry (with polling) using sanitized helper
                try {
                    $script:MessageEvent = Get-SentryTestEvent -EventId $eventIds[0]
                    Write-Host "Event fetched from Sentry successfully" -ForegroundColor Green
                }
                catch {
                    Write-Host "Failed to fetch event from Sentry: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No event ID found in output" -ForegroundColor Yellow
            }
        }

        It "Should exit cleanly" {
            $script:MessageResult.ExitCode | Should -Be 0
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
            if ($Platform -eq 'MacOS') {
                $script:MessageEvent.platform | Should -Be 'cocoa'
            }
            else {
                $script:MessageEvent.platform | Should -Be 'native'
            }
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

    Context "Structured Logging Tests" {
        BeforeAll {
            $script:LogResult = $null
            $script:CapturedLogs = @()
            $script:TestId = $null

            Write-Host "Running structured logging test..." -ForegroundColor Yellow

            $appArgs = @(
                '-nullrhi',     # Runs without graphics rendering (headless mode)
                '-unattended',  # Disables user prompts and interactive dialogs
                '-stdout',      # Ensures logs are written to stdout on Linux/Unix systems
                '-nosplash'     # Prevents splash screen and dialogs
            )

            # Override default project settings to avoid double initialization
            $appArgs += "-ini:Engine:[/Script/Sentry.SentrySettings]:Dsn=$script:DSN"
            $appArgs += "-ini:Engine:[/Script/Sentry.SentrySettings]:EnableStructuredLogging=True"

            # -log-capture triggers integration test log scenario in the sample app
            $script:LogResult = Invoke-DeviceApp -ExecutablePath $script:AppPath -Arguments ((@('-log-capture') + $appArgs) -join ' ')

            Write-Host "Log test executed. Exit code: $($script:LogResult.ExitCode)" -ForegroundColor Cyan

            # Parse test ID from output (format: LOG_TRIGGERED: <test-id>)
            $logTriggeredLines = @($script:LogResult.Output | Where-Object { $_ -match 'LOG_TRIGGERED: ' })
            if ($logTriggeredLines.Count -gt 0) {
                $script:TestId = ($logTriggeredLines[0] -split 'LOG_TRIGGERED: ')[-1].Trim()
                Write-Host "Captured Test ID: $($script:TestId)" -ForegroundColor Cyan

                # Fetch logs from Sentry with automatic polling
                try {
                    $script:CapturedLogs = Get-SentryTestLog -AttributeName 'test_id' -AttributeValue $script:TestId
                }
                catch {
                    Write-Host "Warning: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No LOG_TRIGGERED line found in output" -ForegroundColor Yellow
            }
        }

        It "Should exit cleanly" {
            $script:LogResult.ExitCode | Should -Be 0
        }

        It "Should output LOG_TRIGGERED with test ID" {
            $script:TestId | Should -Not -BeNullOrEmpty
        }

        It "Should output TEST_RESULT with success" {
            $testResultLine = $script:LogResult.Output | Where-Object { $_ -match 'TEST_RESULT:' }
            $testResultLine | Should -Not -BeNullOrEmpty
            $testResultLine | Should -Match '"success"\s*:\s*true'
        }

        It "Should capture structured log in Sentry" {
            $script:CapturedLogs | Should -Not -BeNullOrEmpty
            $script:CapturedLogs.Count | Should -BeGreaterThan 0
        }

        It "Should have correct log message" {
            $log = $script:CapturedLogs[0]
            $log.message | Should -Match 'Integration test structured log'
        }

        It "Should have correct severity level" {
            $log = $script:CapturedLogs[0]
            $log.severity | Should -Be 'warn'
        }

        It "Should have test_id attribute matching captured ID" {
            $log = $script:CapturedLogs[0]
            $log.test_id | Should -Be $script:TestId
        }
    }
}

AfterAll {
    Write-Host "Disconnecting from Sentry API..." -ForegroundColor Yellow
    Disconnect-SentryApi

    Write-Host "Integration tests complete" -ForegroundColor Green
}
