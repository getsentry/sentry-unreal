# Integration tests for Sentry Unreal SDK on Android via SauceLabs Real Device Cloud
# Requires:
# - Pre-built APK
# - SauceLabs account credentials
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN, SENTRY_UNREAL_TEST_APP_PATH
#                          SAUCE_USERNAME, SAUCE_ACCESS_KEY, SAUCE_REGION, SAUCE_DEVICE_NAME
#
# Note: SAUCE_DEVICE_NAME must match a device available in SAUCE_REGION.
#       Example: For SAUCE_REGION=us-west-1, use devices with 'sjc1' suffix (San Jose DC1)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Script-level state for session management
$script:SessionId = $null

function script:Invoke-SauceLabsApi {
    param(
        [Parameter(Mandatory)]
        [string]$Method,

        [Parameter(Mandatory)]
        [string]$Uri,

        [Parameter()]
        [hashtable]$Body = $null,

        [Parameter()]
        [string]$ContentType = 'application/json',

        [Parameter()]
        [switch]$IsMultipart,

        [Parameter()]
        [string]$FilePath
    )

    $username = $env:SAUCE_USERNAME
    $accessKey = $env:SAUCE_ACCESS_KEY

    if (-not $username -or -not $accessKey) {
        throw "SAUCE_USERNAME and SAUCE_ACCESS_KEY environment variables must be set"
    }

    $base64Auth = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes("${username}:${accessKey}"))
    $headers = @{
        'Authorization' = "Basic $base64Auth"
    }

    try {
        if ($IsMultipart) {
            # Use -Form parameter for multipart uploads (PowerShell Core 7+)
            $form = @{
                payload = Get-Item -Path $FilePath
                name = (Split-Path $FilePath -Leaf)
            }
            $webResponse = Invoke-WebRequest -Uri $Uri -Method $Method -Headers $headers -Form $form
        } else {
            $params = @{
                Uri = $Uri
                Method = $Method
                Headers = $headers
            }

            if ($Body) {
                $params['Body'] = ($Body | ConvertTo-Json -Depth 10)
                $params['ContentType'] = $ContentType
            }

            $webResponse = Invoke-WebRequest @params
        }

        # Explicit JSON parsing for better error visibility (Invoke-RestMethod can silently return strings)
        if ($webResponse.Content) {
            return $webResponse.Content | ConvertFrom-Json -AsHashtable
        }
        return $null
    } catch {
        $ErrorMessage = "SauceLabs API request ($Method $Uri) failed: $($_.Exception.Message)"
        if ($_.Exception.Response) {
            $StatusCode = $_.Exception.Response.StatusCode
            $ErrorMessage += " (Status: $StatusCode)"
        }
        throw $ErrorMessage
    }
}

function script:Install-SauceLabsApp {
    param(
        [Parameter(Mandatory)]
        [string]$ApkPath,

        [Parameter(Mandatory)]
        [string]$Region
    )

    if (-not (Test-Path $ApkPath)) {
        throw "APK file not found: $ApkPath"
    }

    if ($ApkPath -notlike '*.apk') {
        throw "Package must be an .apk file. Got: $ApkPath"
    }

    Write-Host "Uploading APK to SauceLabs Storage..." -ForegroundColor Yellow
    $uploadUri = "https://api.${Region}.saucelabs.com/v1/storage/upload"

    $response = Invoke-SauceLabsApi -Method POST -Uri $uploadUri -IsMultipart -FilePath $ApkPath

    if (-not $response.item.id) {
        throw "Failed to upload APK: No storage ID in response"
    }

    $storageId = $response.item.id
    Write-Host "APK uploaded successfully. Storage ID: $storageId" -ForegroundColor Green

    return $storageId
}

function script:Initialize-SauceLabsSession {
    param(
        [Parameter(Mandatory)]
        [string]$StorageId,

        [Parameter(Mandatory)]
        [string]$DeviceName,

        [Parameter(Mandatory)]
        [string]$Region,

        [Parameter(Mandatory)]
        [string]$UnrealVersion
    )

    Write-Host "Creating SauceLabs Appium session..." -ForegroundColor Yellow

    $sessionUri = "https://ondemand.${Region}.saucelabs.com/wd/hub/session"

    $capabilities = @{
        capabilities = @{
            alwaysMatch = @{
                platformName = "Android"
                'appium:app' = "storage:$StorageId"
                'appium:deviceName' = $DeviceName
                'appium:automationName' = "UiAutomator2"
                'appium:noReset' = $true
                'appium:autoLaunch' = $false
                'sauce:options' = @{
                    name = "UE $UnrealVersion Android Integration Test"
                    appiumVersion = "latest"
                }
            }
        }
    }

    $response = Invoke-SauceLabsApi -Method POST -Uri $sessionUri -Body $capabilities

    $sessionId = $response.value.sessionId
    if (-not $sessionId) {
        $sessionId = $response.sessionId
    }

    if (-not $sessionId) {
        throw "Failed to create session: No session ID in response"
    }

    Write-Host "Session created successfully. Session ID: $sessionId" -ForegroundColor Green

    return $sessionId
}

function script:Invoke-SauceLabsApp {
    param(
        [Parameter(Mandatory)]
        [string]$PackageName,

        [Parameter(Mandatory)]
        [string]$ActivityName,

        [Parameter()]
        [string]$Arguments = "",

        [Parameter(Mandatory)]
        [string]$Region
    )

    $sessionId = $script:SessionId
    if (-not $sessionId) {
        throw "No active SauceLabs session. Call Initialize-SauceLabsSession first."
    }

    $baseUri = "https://ondemand.${Region}.saucelabs.com/wd/hub/session/$sessionId"
    $outputDir = $script:OutputDir

    # Configuration
    $timeoutSeconds = 300
    $pollIntervalSeconds = 2

    $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $logFile = if ($OutputDir) { "$OutputDir/$timestamp-logcat.txt" } else { $null }

    # Launch activity with Intent extras
    Write-Host "Launching: $PackageName/$ActivityName" -ForegroundColor Cyan
    if ($Arguments) {
        Write-Host "  Arguments: $Arguments" -ForegroundColor Cyan
    }

    $launchBody = @{
        appPackage = $PackageName
        appActivity = $ActivityName
        appWaitActivity = "*"
        intentAction = "android.intent.action.MAIN"
        intentCategory = "android.intent.category.LAUNCHER"
    }

    if ($Arguments) {
        $launchBody['optionalIntentArguments'] = $Arguments
    }

    try {
        $launchResponse = Invoke-SauceLabsApi -Method POST -Uri "$baseUri/appium/device/start_activity" -Body $launchBody
        Write-Debug "Launch response: $($launchResponse | ConvertTo-Json)"
    } catch {
        throw "Failed to launch activity: $_"
    }

    # Wait a moment for app to start
    Start-Sleep -Seconds 3

    # Poll app state until it exits or completes
    Write-Host "Monitoring app execution..." -ForegroundColor Yellow
    $startTime = Get-Date
    $completed = $false

    while ((Get-Date) - $startTime -lt [TimeSpan]::FromSeconds($timeoutSeconds)) {
        # Query app state
        $stateBody = @{
            script = "mobile: queryAppState"
            args = @(
                @{ appId = $PackageName }
            )
        }

        try {
            $stateResponse = Invoke-SauceLabsApi -Method POST -Uri "$baseUri/execute/sync" -Body $stateBody
            $appState = $stateResponse.value

            Write-Debug "App state: $appState (elapsed: $([int]((Get-Date) - $startTime).TotalSeconds)s)"

            # State 1 = not running, 0 = not installed
            if ($appState -eq 1 -or $appState -eq 0) {
                Write-Host "App finished/crashed (state: $appState)" -ForegroundColor Green
                $completed = $true
                break
            }
        } catch {
            Write-Warning "Failed to query app state: $_"
        }

        Start-Sleep -Seconds $pollIntervalSeconds
    }

    if (-not $completed) {
        Write-Host "Warning: Test did not complete within timeout" -ForegroundColor Yellow
    }

    # Retrieve logs after app completion
    Write-Host "Retrieving logs..." -ForegroundColor Yellow
    $logBody = @{ type = "logcat" }
    $logResponse = Invoke-SauceLabsApi -Method POST -Uri "$baseUri/log" -Body $logBody

    [array]$allLogs = @()
    if ($logResponse.value -and $logResponse.value.Count -gt 0) {
        $allLogs = @($logResponse.value)
        Write-Host "Retrieved $($allLogs.Count) log lines" -ForegroundColor Cyan
    }

    # Convert SauceLabs log format to text (matching adb output)
    $logCache = @()
    if ($allLogs -and $allLogs.Count -gt 0) {
        $logCache = $allLogs | ForEach-Object {
            if ($_) {
                $timestamp = if ($_.timestamp) { $_.timestamp } else { "" }
                $level = if ($_.level) { $_.level } else { "" }
                $message = if ($_.message) { $_.message } else { "" }
                "$timestamp $level $message"
            }
        } | Where-Object { $_ }  # Filter out any nulls
    }

    # Save logs to file if OutputDir specified
    if ($logFile) {
        $logCache | Out-File $logFile
        Write-Host "Logs saved to: $logFile" -ForegroundColor Cyan
    }

    # Return structured result (matches app-runner pattern)
    return @{
        ExitCode = 0  # Android apps don't report exit codes
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
    $script:SauceUsername = $env:SAUCE_USERNAME
    $script:SauceAccessKey = $env:SAUCE_ACCESS_KEY
    $script:SauceRegion = $env:SAUCE_REGION
    $script:SauceDeviceName = $env:SAUCE_DEVICE_NAME

    if (-not $script:DSN) {
        throw "Environment variable SENTRY_UNREAL_TEST_DSN must be set"
    }

    if (-not $script:AuthToken) {
        throw "Environment variable SENTRY_AUTH_TOKEN must be set"
    }

    if (-not $script:ApkPath) {
        throw "Environment variable SENTRY_UNREAL_TEST_APP_PATH must be set"
    }

    if (-not $script:SauceUsername) {
        throw "Environment variable SAUCE_USERNAME must be set"
    }

    if (-not $script:SauceAccessKey) {
        throw "Environment variable SAUCE_ACCESS_KEY must be set"
    }

    if (-not $script:SauceRegion) {
        throw "Environment variable SAUCE_REGION must be set"
    }

    if (-not $script:SauceDeviceName) {
        throw "Environment variable SAUCE_DEVICE_NAME must be set"
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
    $script:ActivityName = "com.epicgames.unreal.GameActivity"
    $script:DeviceName = $script:SauceDeviceName
    $script:UnrealVersion = if ($env:UNREAL_VERSION) { $env:UNREAL_VERSION } else { "5.x" }

    # Upload APK to SauceLabs Storage
    Write-Host "Uploading APK to SauceLabs..." -ForegroundColor Yellow
    $script:StorageId = Install-SauceLabsApp -ApkPath $script:ApkPath -Region $script:SauceRegion

    # Create SauceLabs session (reused for all app launches)
    Write-Host "Creating SauceLabs session..." -ForegroundColor Yellow
    $script:SessionId = Initialize-SauceLabsSession `
        -StorageId $script:StorageId `
        -DeviceName $script:DeviceName `
        -Region $script:SauceRegion `
        -UnrealVersion $script:UnrealVersion

    # ==========================================
    # RUN 1: Crash test - creates minidump
    # ==========================================
    # The crash is captured but NOT uploaded yet (Android behavior).
    # TODO: Re-enable once Android SDK tag persistence is fixed (`test.crash_id` tag set before crash is not synced to the captured crash on Android)

    # Write-Host "Running crash-capture test (will crash)..." -ForegroundColor Yellow
    # $cmdlineCrashArgs = "-e cmdline -crash-capture"
    # $global:SauceLabsCrashResult = Invoke-SauceLabsApp `
    #     -PackageName $script:PackageName `
    #     -ActivityName $script:ActivityName `
    #     -Arguments $cmdlineCrashArgs `
    #     -Region $script:SauceRegion

    # Write-Host "Crash test exit code: $($global:SauceLabsCrashResult.ExitCode)" -ForegroundColor Cyan

    # ==========================================
    # RUN 2: Message test - uploads crash from Run 1 + captures message
    # ==========================================
    # Currently we need to run again so that Sentry sends the crash event captured during the previous app session.

    Write-Host "Running message-capture test (will upload crash from previous run)..." -ForegroundColor Yellow
    $cmdlineMessageArgs = "-e cmdline -message-capture"
    $global:SauceLabsMessageResult = Invoke-SauceLabsApp `
        -PackageName $script:PackageName `
        -ActivityName $script:ActivityName `
        -Arguments $cmdlineMessageArgs `
        -Region $script:SauceRegion

    Write-Host "Message test exit code: $($global:SauceLabsMessageResult.ExitCode)" -ForegroundColor Cyan
}

Describe "Sentry Unreal Android Integration Tests (SauceLabs)" {

    # ==========================================
    # NOTE: Crash Capture Tests are DISABLED due to tag sync issue
    # Uncomment when Android SDK tag persistence is fixed
    # ==========================================
    # Context "Crash Capture Tests" {
    #     BeforeAll {
    #         # Crash event is sent during the MESSAGE run (Run 2)
    #         # But the crash_id comes from the CRASH run (Run 1)
    #         $CrashResult = $global:SauceLabsCrashResult
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
            $MessageResult = $global:SauceLabsMessageResult
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
    # Clean up SauceLabs session
    if ($script:SessionId) {
        Write-Host "Ending SauceLabs session..." -ForegroundColor Yellow
        try {
            $sessionUri = "https://ondemand.$($script:SauceRegion).saucelabs.com/wd/hub/session/$($script:SessionId)"
            Invoke-SauceLabsApi -Method DELETE -Uri $sessionUri
            Write-Host "Session ended successfully" -ForegroundColor Green
        } catch {
            Write-Warning "Failed to end session: $_"
        }
    }

    Write-Host "Disconnecting from Sentry API..." -ForegroundColor Yellow
    Disconnect-SentryApi
    Write-Host "Integration tests complete" -ForegroundColor Green
}
