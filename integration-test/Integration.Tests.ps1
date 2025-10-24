# Integration tests for Sentry Unreal SDK
# Requires:
# - Pre-built SentryPlayground application
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN

param(
    [Parameter(Mandatory=$true)]
    [string]$AppPath,

    [ValidateSet('Win64', 'Linux')]
    [string]$Platform = 'Win64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Convert-HashtableToObject($item) {
    if ($item -is [System.Collections.IDictionary]) {
        $obj = [PSCustomObject]@{}
        foreach ($key in $item.Keys) {
            if ([string]::IsNullOrWhiteSpace($key)) {
                Write-Warning "Removed property with empty name"
                continue
            }
            $obj | Add-Member -NotePropertyName $key -NotePropertyValue (Convert-HashtableToObject $item[$key])
        }
        return $obj
    } elseif ($item -is [System.Collections.IEnumerable] -and -not ($item -is [string])) {
        return @($item | ForEach-Object { Convert-HashtableToObject $_ })
    } else {
        return $item
    }
}

function Get-MyTestEvent {
    [CmdletBinding()]
    param(
        [Parameter()]
        [string]$EventId,

        [Parameter()]
        [string]$TagName,

        [Parameter()]
        [string]$TagValue,

        [Parameter()]
        [int]$TimeoutSeconds = 120
    )

    if ($EventId) {
        Write-Host "Fetching Sentry event by ID: $EventId" -ForegroundColor Yellow
        $progressActivity = "Waiting for Sentry event $EventId"
    } elseif ($TagName -and $TagValue) {
        Write-Host "Fetching Sentry event by tag: $TagName=$TagValue" -ForegroundColor Yellow
        $progressActivity = "Waiting for Sentry event with tag $TagName=$TagValue"
    } else {
        throw 'Must specify either EventId or both TagName and TagValue'
    }

    $startTime = Get-Date
    $endTime = $startTime.AddSeconds($TimeoutSeconds)
    $lastError = $null
    $elapsedSeconds = 0

    try {
        do {
            $sentryEvent = $null
            $elapsedSeconds = [int]((Get-Date) - $startTime).TotalSeconds
            $percentComplete = [math]::Min(100, ($elapsedSeconds / $TimeoutSeconds) * 100)

            Write-Progress -Activity $progressActivity -Status "Elapsed: $elapsedSeconds/$TimeoutSeconds seconds" -PercentComplete $percentComplete

            try {
                if ($EventId) {
                    # Find by event ID
                    $sentryEvent = Get-SentryEvent -EventId $EventId
                } else {
                    # Find by tag
                    $result = Find-SentryEventByTag -TagName $TagName -TagValue $TagValue
                    $result.Count | Should -Be 1
                    $sentryEvent = $result[0]
                }
            } catch {
                $lastError = $_.Exception.Message
                # Event not found yet, continue waiting
                if ($EventId) {
                    Write-Debug "Event $EventId not found yet: $lastError"
                } else {
                    Write-Debug "Event with tag $TagName=$TagValue not found yet: $lastError"
                }
            }

            if ($sentryEvent) {
               # Convert from JSON if it's a raw string
                if ($sentryEvent -is [string]) {
                    try {
                        $raw = $sentryEvent | ConvertFrom-Json -Depth 50 -AsHashTable
                        # Recursively convert hashtables to PSCustomObjects
                        $sentryEvent = Convert-HashtableToObject $raw
                    } catch {
                        Write-Host "Failed to parse JSON from Sentry event: $($_.Exception.Message)" -ForegroundColor Red
                        return
                    }
                }

                Write-Host "Event $($sentryEvent.id) fetched from Sentry" -ForegroundColor Green
                $entries = $sentryEvent.entries
                $sentryEvent = $sentryEvent | Select-Object -ExcludeProperty 'entries'
                foreach ($entry in $entries) {
                    $sentryEvent | Add-Member -MemberType NoteProperty -Name $entry.type -Value $entry.data -Force
                }
                $sentryEvent | ConvertTo-Json -Depth 10 | Out-File -FilePath (Get-OutputFilePath "event-$($sentryEvent.id).json")
                return $sentryEvent
            }

            Start-Sleep -Milliseconds 500
            $currentTime = Get-Date
        } while ($currentTime -lt $endTime)
    } finally {
        Write-Progress -Activity $progressActivity -Completed
    }

    if ($EventId) {
        throw "Event $EventId not found in Sentry within $TimeoutSeconds seconds: $lastError"
    } else {
        throw "Event with tag $TagName=$TagValue not found in Sentry within $TimeoutSeconds seconds: $lastError"
    }
}

BeforeAll {
    # Check if environment is configured
    $configFile = "$PSScriptRoot/TestConfig.local.ps1"
    if (-not (Test-Path $configFile)) {
        throw @"
Integration test environment not configured.

Please run:
  cd integration-test
  cmake -B build -S .

With environment variables:
  SENTRY_UNREAL_TEST_DSN=https://...
  SENTRY_AUTH_TOKEN=sntrys_...
"@
    }

    # Load configuration (provides $global:AppRunnerPath)
    . $configFile

    # Import app-runner modules
    . "$global:AppRunnerPath/import-modules.ps1"

    # Validate environment variables
    $script:DSN = $env:SENTRY_UNREAL_TEST_DSN
    $script:AuthToken = $env:SENTRY_AUTH_TOKEN

    if (-not $script:DSN -or -not $script:AuthToken) {
        throw "Environment variables SENTRY_UNREAL_TEST_DSN and SENTRY_AUTH_TOKEN must be set"
    }

    # Connect to Sentry API
    Write-Host "Connecting to Sentry API..." -ForegroundColor Yellow
    Connect-SentryApi -DSN $script:DSN -ApiToken $script:AuthToken

    # Validate app path
    if (-not (Test-Path $AppPath)) {
        throw "Application not found at: $AppPath"
    }

    # Create output directory
    $script:OutputDir = "$PSScriptRoot/output"
    if (-not (Test-Path $script:OutputDir)) {
        New-Item -ItemType Directory -Path $script:OutputDir | Out-Null
    }

    Write-Host "Test environment ready" -ForegroundColor Green
    Write-Host "  Platform: $Platform" -ForegroundColor Cyan
    Write-Host "  App: $AppPath" -ForegroundColor Cyan
    Write-Host "  Output: $script:OutputDir" -ForegroundColor Cyan
}

Describe "Sentry Unreal Integration Tests ($Platform)" {

    Context "Crash Capture Tests" {
        BeforeAll {
            $script:CrashResult = $null
            $script:CrashEvent = $null

            Write-Host "Running crash capture test..." -ForegroundColor Yellow

            # Prepare output files
            $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
            $stdoutFile = "$script:OutputDir/$timestamp-crash-stdout.log"
            $stderrFile = "$script:OutputDir/$timestamp-crash-stderr.log"

            # Build arguments
            $args = @('-crash-capture', '-NullRHI', '-unattended')

            # Execute application based on platform
            if ($Platform -eq 'Win64') {
                $process = Start-Process -FilePath $AppPath -ArgumentList $args `
                    -Wait -PassThru -NoNewWindow `
                    -RedirectStandardOutput $stdoutFile `
                    -RedirectStandardError $stderrFile

                $script:CrashResult = @{
                    ExitCode = $process.ExitCode
                    Output = Get-Content $stdoutFile
                    Error = Get-Content $stderrFile
                }
            } else {
                # Linux
                chmod +x $AppPath 2>&1 | Out-Null
                $output = & $AppPath @args 2>&1
                $output | Out-File $stdoutFile

                $script:CrashResult = @{
                    ExitCode = $LASTEXITCODE
                    Output = $output
                    Error = @()
                }
            }

            # Save full output
            $script:CrashResult | ConvertTo-Json -Depth 5 | Out-File "$script:OutputDir/$timestamp-crash-result.json"

            Write-Host "Crash test executed. Exit code: $($script:CrashResult.ExitCode)" -ForegroundColor Cyan

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                $crashId = $eventIds[0]

                # Fetch event from Sentry (with polling)
                try {
                    $script:CrashEvent = Get-SentryTestEvent -TagName 'test.crash_id' -TagValue "$crashId"
                    Write-Host "Event fetched from Sentry successfully" -ForegroundColor Green
                } catch {
                    Write-Host "Failed to fetch event from Sentry: $_" -ForegroundColor Red
                }
            } else {
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
            $script:CrashEvent.platform | Should -Be 'native'
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

            # Prepare output files
            $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
            $stdoutFile = "$script:OutputDir/$timestamp-message-stdout.log"
            $stderrFile = "$script:OutputDir/$timestamp-message-stderr.log"

            # Build arguments
            $args = @('-message-capture', '-NullRHI', '-unattended')

            # Execute application based on platform
            if ($Platform -eq 'Win64') {
                $process = Start-Process -FilePath $AppPath -ArgumentList $args `
                    -Wait -PassThru -NoNewWindow `
                    -RedirectStandardOutput $stdoutFile `
                    -RedirectStandardError $stderrFile

                $script:MessageResult = @{
                    ExitCode = $process.ExitCode
                    Output = Get-Content $stdoutFile
                    Error = Get-Content $stderrFile
                }
            } else {
                # Linux
                $output = & $AppPath @args 2>&1
                $output | Out-File $stdoutFile

                $script:MessageResult = @{
                    ExitCode = $LASTEXITCODE
                    Output = $output
                    Error = @()
                }
            }

            # Save full output
            $script:MessageResult | ConvertTo-Json -Depth 5 | Out-File "$script:OutputDir/$timestamp-message-result.json"

            Write-Host "Message test executed. Exit code: $($script:MessageResult.ExitCode)" -ForegroundColor Cyan

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $script:MessageResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                # Fetch event from Sentry (with polling) using sanitized helper
                try {
                    $script:MessageEvent = Get-MyTestEvent -EventId $eventIds[0] -TimeoutSeconds 300
                    Write-Host "Event fetched from Sentry successfully" -ForegroundColor Green
                } catch {
                    Write-Host "Failed to fetch event from Sentry: $_" -ForegroundColor Red
                }
            } else {
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
            $script:MessageEvent.platform | Should -Be 'native'
        }

        It "Should have message content" {
            $script:MessageEvent.message | Should -Not -BeNullOrEmpty
            $script:MessageEvent.message | Should -Match 'Integration test message'
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
