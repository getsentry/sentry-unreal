# Integration tests for Sentry Unreal SDK on Android
# Supports both ADB (local devices/emulators) and SauceLabs (cloud devices)
#
# Usage:
#   Invoke-Pester Integration.Android.Tests.ps1
#
# Requires:
# - Pre-built APK
# - Environment variables: SENTRY_UNREAL_TEST_DSN, SENTRY_AUTH_TOKEN, SENTRY_UNREAL_TEST_APP_PATH
#
# For ADB:
#   - Android emulator or device connected via ADB
#
# For SauceLabs:
#   - SAUCE_USERNAME, SAUCE_ACCESS_KEY, SAUCE_REGION, SAUCE_DEVICE_NAME, SAUCE_SESSION_NAME

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

BeforeDiscovery {
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

    $TestTargets = @()

    # Detect if running in CI environment
    # In CI, running tests using SauceLabs is mandatory while adb tests are skipped due to emulator limitations
    $isCI = $env:CI -eq 'true'

    # Check adb test configuration
    if (Get-Command 'adb' -ErrorAction SilentlyContinue) {
        # Check if any devices are connected
        $adbDevices = adb devices
        if ($adbDevices -match '\tdevice$') {
            $TestTargets += Get-TestTarget -Platform 'Adb' -ProviderName 'Adb'
        }
        else {
            Write-Host "No devices connected via adb. Adb tests will be skipped."
        }
    }
    else {
        Write-Host "adb not found in PATH. Adb tests will be skipped."
    }

    # Check SauceLabs test configuration
    if ($env:SAUCE_USERNAME -and $env:SAUCE_ACCESS_KEY -and $env:SAUCE_REGION -and $env:SAUCE_DEVICE_NAME -and $env:SAUCE_SESSION_NAME) {
        $TestTargets += Get-TestTarget -Platform 'SauceLabs' -ProviderName 'AndroidSauceLabs'
    }
    else {
        $message = "SauceLabs credentials not found"
        if ($isCI) {
            throw "$message. These are required in CI."
        }
        else {
            Write-Host "$message. SauceLabs tests will be skipped."
        }
    }

    # Inform user if no test targets are available
    if ($TestTargets.Count -eq 0) {
        Write-Warning "No Android test targets detected. Integration tests will be skipped."
        Write-Warning "To run Android integration tests, configure at least one test target:"
        Write-Warning "  - Adb: ADB must be in PATH and at least one Android device must be connected (physical or emulator)"
        Write-Warning "  - SauceLabs: Environment variables SAUCE_USERNAME, SAUCE_ACCESS_KEY, SAUCE_REGION, SAUCE_DEVICE_NAME, SAUCE_SESSION_NAME must be set"
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
}

Describe 'Sentry Unreal Android Integration Tests (<Platform>)' -ForEach $TestTargets {

    BeforeAll {
        # Connect to Android device (provider validates its own env vars)
        Write-Host "Connecting to Android via $Platform..." -ForegroundColor Yellow
        Connect-Device -Platform $ProviderName

        # Install APK
        Write-Host "Installing APK via $Platform..." -ForegroundColor Yellow
        Install-DeviceApp -Path $script:ApkPath

        # ==========================================
        # RUN 1: Crash test - creates minidump
        # ==========================================
        # The crash is captured but NOT uploaded yet (Android behavior).

        Write-Host "Running crash-capture test (will crash) on $Platform..." -ForegroundColor Yellow
        $crashIntentArgs = "-e cmdline -crash-capture"
        $global:AndroidCrashResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $crashIntentArgs

        Write-Host "Crash test exit code: $($global:AndroidCrashResult.ExitCode)" -ForegroundColor Cyan

        # ==========================================
        # RUN 2: Init-only - flushes crash event from Run 1
        # ==========================================
        # The crash is captured but NOT uploaded immediately on Android,
        # so we need to run the app again to send it to Sentry.
        # -init-only allows starting the app to flush captured events and quit right after.

        Write-Host "Running init-only to flush crash event on $Platform..." -ForegroundColor Yellow
        $initOnlyIntentArgs = "-e cmdline -init-only"
        $global:AndroidInitOnlyResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $initOnlyIntentArgs

        Write-Host "Init-only exit code: $($global:AndroidInitOnlyResult.ExitCode)" -ForegroundColor Cyan

        # ==========================================
        # RUN 3: Message test - captures message
        # ==========================================

        Write-Host "Running message-capture test on $Platform..." -ForegroundColor Yellow
        $messageIntentArgs = "-e cmdline -message-capture\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:BeforeSendHandler=/Script/SentryPlayground.CppBeforeSendHandler"
        $global:AndroidMessageResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $messageIntentArgs

        Write-Host "Message test exit code: $($global:AndroidMessageResult.ExitCode)" -ForegroundColor Cyan

        # ==========================================
        # RUN 4: Log test - captures structured log
        # ==========================================

        Write-Host "Running log-capture test on $Platform..." -ForegroundColor Yellow
        $logIntentArgs = "-e cmdline -log-capture\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:EnableStructuredLogging=True\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:BeforeLogHandler=/Script/SentryPlayground.CppBeforeLogHandler"
        $global:AndroidLogResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $logIntentArgs

        Write-Host "Log test exit code: $($global:AndroidLogResult.ExitCode)" -ForegroundColor Cyan

        # ==========================================
        # RUN 4: Metric test - captures custom metric
        # ==========================================

        Write-Host "Running metric-capture test on $Platform..." -ForegroundColor Yellow
        $metricIntentArgs = "-e cmdline -metric-capture\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:EnableMetrics=True\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:BeforeMetricHandler=/Script/SentryPlayground.CppBeforeMetricHandler"
        $global:AndroidMetricResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $metricIntentArgs

        Write-Host "Metric test exit code: $($global:AndroidMetricResult.ExitCode)" -ForegroundColor Cyan

        # ==========================================
        # RUN 5: Tracing test - captures transaction
        # ==========================================

        Write-Host "Running tracing-capture test on $Platform..." -ForegroundColor Yellow
        $tracingIntentArgs = "-e cmdline -tracing-capture\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:EnableTracing=True\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:SamplingType=TracesSampler\ -ini:Engine:\[/Script/Sentry.SentrySettings\]:TracesSampler=/Script/SentryPlayground.CppTraceSampler"
        $global:AndroidTracingResult = Invoke-DeviceApp -ExecutablePath $script:ActivityName -Arguments $tracingIntentArgs

        Write-Host "Tracing test exit code: $($global:AndroidTracingResult.ExitCode)" -ForegroundColor Cyan
    }

    AfterAll {
        # Disconnect from Android device
        Write-Host "Disconnecting from $Platform..." -ForegroundColor Yellow
        Disconnect-Device

        Write-Host "Integration tests complete on $Platform" -ForegroundColor Green
    }

    Context "Crash Capture Tests" {
        BeforeAll {
            # Crash event is sent during the INIT-ONLY run (Run 2)
            # But the crash_id comes from the CRASH run (Run 1)
            $script:CrashResult = $global:AndroidCrashResult
            $script:CrashEvent = $null
    
            # Parse crash event ID from crash run output
            $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1
    
            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Crash ID captured: $($eventIds[0])" -ForegroundColor Cyan
                $crashId = $eventIds[0]
    
                # Fetch crash event using the tag (event was sent during init-only run)
                try {
                    $script:CrashEvent = Get-SentryTestEvent -TagName 'test.crash_id' -TagValue "$crashId"
                    Write-Host "Crash event fetched from Sentry successfully" -ForegroundColor Green
                }
                catch {
                    Write-Host "Failed to fetch crash event from Sentry: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No crash event ID found in output" -ForegroundColor Yellow
            }
        }
    
        It "Should output event ID before crash" {
            $eventIds = Get-EventIds -AppOutput $script:CrashResult.Output -ExpectedCount 1
            $eventIds | Should -Not -BeNullOrEmpty
            $eventIds.Count | Should -Be 1
        }
    
        It "Should capture crash event in Sentry (uploaded during init-only run)" {
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
    
        It "Should have test.crash_id tag for correlation" {
            $tags = $script:CrashEvent.tags
            $crashIdTag = $tags | Where-Object { $_.key -eq 'test.crash_id' }
            $crashIdTag | Should -Not -BeNullOrEmpty
            $crashIdTag.value | Should -Not -BeNullOrEmpty
        }
    
        It "Should have integration test tag" {
            $tags = $script:CrashEvent.tags
            ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
        }
    
        It "Should have breadcrumbs from before crash" {
            $script:CrashEvent.breadcrumbs | Should -Not -BeNullOrEmpty
            $script:CrashEvent.breadcrumbs.values | Should -Not -BeNullOrEmpty
        }
    }

    Context "Message Capture Tests" {
        BeforeAll {
            $script:MessageResult = $global:AndroidMessageResult
            $script:MessageEvent = $null

            # Parse event ID from output
            $eventIds = Get-EventIds -AppOutput $MessageResult.Output -ExpectedCount 1

            if ($eventIds -and $eventIds.Count -gt 0) {
                Write-Host "Message event ID captured: $($eventIds[0])" -ForegroundColor Cyan

                # Fetch event from Sentry (with polling)
                try {
                    $script:MessageEvent = Get-SentryTestEvent -EventId $eventIds[0]
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

        It "Should have global breadcrumbs" {
            $breadcrumbs = $script:MessageEvent.breadcrumbs.values
            $breadcrumbs | Should -Not -BeNullOrEmpty
            $breadcrumbs | Where-Object { $_.message -eq 'Integration test started' -and $_.category -eq 'Test' } | Should -Not -BeNullOrEmpty
            $breadcrumbs | Where-Object { $_.message -eq 'Context configuration finished' -and $_.category -eq 'Test' } | Should -Not -BeNullOrEmpty
        }

        # BeforeSendHandler assertions
        It "Should have tag added by BeforeSendHandler" {
            $tags = $script:MessageEvent.tags
            ($tags | Where-Object { $_.key -eq 'before_send.handled' }).value | Should -Be 'true'
        }

        It "Should not have tag removed by BeforeSendHandler" {
            $tags = $script:MessageEvent.tags
            $tags | Where-Object { $_.key -eq 'tag_to_be_removed' } | Should -BeNullOrEmpty
        }

        It "Should have extra added by BeforeSendHandler" {
            $script:MessageEvent.context.handler_added | Should -Be 'added_value'
        }

        It "Should not have extra removed by BeforeSendHandler" {
            $script:MessageEvent.context.extra_to_be_removed | Should -BeNullOrEmpty
        }

        It "Should not have context removed by BeforeSendHandler" {
            $script:MessageEvent.contexts.context_removed_by_handler | Should -BeNullOrEmpty
        }

        # Global scope context assertions
        It "Should have custom context from global scope" {
            $script:MessageEvent.contexts.test_context | Should -Not -BeNullOrEmpty
            $script:MessageEvent.contexts.test_context.context_key | Should -Be 'context_value'
        }

        # Local scope enrichment assertions
        It "Should have local scope tag" {
            $tags = $script:MessageEvent.tags
            ($tags | Where-Object { $_.key -eq 'scope.locality' }).value | Should -Be 'local'
        }

        It "Should have local scope extra" {
            $script:MessageEvent.context.local_extra | Should -Be 'local_extra_value'
        }

        It "Should have local scope context" {
            $script:MessageEvent.contexts.local_context | Should -Not -BeNullOrEmpty
            $script:MessageEvent.contexts.local_context.local_key | Should -Be 'local_value'
        }

        It "Should have local scope breadcrumb" {
            $breadcrumbs = $script:MessageEvent.breadcrumbs.values
            $breadcrumbs | Where-Object { $_.message -eq 'Local scope breadcrumb' -and $_.category -eq 'test' } | Should -Not -BeNullOrEmpty
        }
    }

    Context "Structured Logging Tests" {
        BeforeAll {
            $script:LogResult = $global:AndroidLogResult
            $script:CapturedLogs = @()
            $script:TestId = $null

            # Parse test ID from output (format: LOG_TRIGGERED: <test-id>)
            $logTriggeredLines = @($script:LogResult.Output | Where-Object { $_ -match 'LOG_TRIGGERED: ' })
            if ($logTriggeredLines.Count -gt 0) {
                $script:TestId = ($logTriggeredLines[0] -split 'LOG_TRIGGERED: ')[-1].Trim()
                Write-Host "Captured Test ID: $($script:TestId)" -ForegroundColor Cyan

                # Fetch logs from Sentry with automatic polling
                try {
                    $script:CapturedLogs = Get-SentryTestLog -AttributeName 'test_id' -AttributeValue $script:TestId -Fields @('handler_added', 'to_be_removed')
                }
                catch {
                    Write-Host "Warning: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No LOG_TRIGGERED line found in output" -ForegroundColor Yellow
            }
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
            $log.'test_id' | Should -Be $script:TestId
        }

        It "Should have attribute added by BeforeLogHandler" {
            $log = $script:CapturedLogs[0]
            $log.'handler_added' | Should -Be 'added_value'
        }

        It "Should not have attribute removed by BeforeLogHandler" {
            $log = $script:CapturedLogs[0]
            $log.'to_be_removed' | Should -BeNullOrEmpty
        }

        # Note: Global log attributes (SetAttribute/RemoveAttribute on subsystem) are not supported
        # on Android (sentry-java) - the implementation is a no-op. These are tested in desktop tests only.
    }

    Context "Metrics Capture Tests" {
        BeforeAll {
            $script:MetricResult = $global:AndroidMetricResult
            $script:CapturedCounterMetrics = @()
            $script:CapturedDistributionMetrics = @()
            $script:CapturedGaugeMetrics = @()
            $script:TestId = $null

            # Parse test ID from output (format: METRIC_TRIGGERED: <test-id>)
            $metricTriggeredLines = @($script:MetricResult.Output | Where-Object { $_ -match 'METRIC_TRIGGERED: ' })
            if ($metricTriggeredLines.Count -gt 0) {
                $script:TestId = ($metricTriggeredLines[0] -split 'METRIC_TRIGGERED: ')[-1].Trim()
                Write-Host "Captured Test ID: $($script:TestId)" -ForegroundColor Cyan

                # Fetch all three metric types from Sentry with automatic polling
                $metricFields = @('handler_added', 'to_be_removed')

                try {
                    $script:CapturedCounterMetrics = Get-SentryTestMetric -MetricName 'test.integration.counter' -AttributeName 'test_id' -AttributeValue $script:TestId -Fields $metricFields
                }
                catch {
                    Write-Host "Warning (counter): $_" -ForegroundColor Red
                }

                try {
                    $script:CapturedDistributionMetrics = Get-SentryTestMetric -MetricName 'test.integration.distribution' -AttributeName 'test_id' -AttributeValue $script:TestId -Fields $metricFields
                }
                catch {
                    Write-Host "Warning (distribution): $_" -ForegroundColor Red
                }

                try {
                    $script:CapturedGaugeMetrics = Get-SentryTestMetric -MetricName 'test.integration.gauge' -AttributeName 'test_id' -AttributeValue $script:TestId -Fields $metricFields
                }
                catch {
                    Write-Host "Warning (gauge): $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No METRIC_TRIGGERED line found in output" -ForegroundColor Yellow
            }
        }

        It "Should output METRIC_TRIGGERED with test ID" {
            $script:TestId | Should -Not -BeNullOrEmpty
        }

        It "Should output TEST_RESULT with success" {
            $testResultLine = $script:MetricResult.Output | Where-Object { $_ -match 'TEST_RESULT:' }
            $testResultLine | Should -Not -BeNullOrEmpty
            $testResultLine | Should -Match '"success"\s*:\s*true'
        }

        # Counter metric assertions
        It "Should capture counter metric in Sentry" {
            $script:CapturedCounterMetrics | Should -Not -BeNullOrEmpty
        }

        It "Should have correct counter metric name and type" {
            $metric = $script:CapturedCounterMetrics[0]
            $metric.'metric.name' | Should -Be 'test.integration.counter'
            $metric.'metric.type' | Should -Be 'counter'
        }

        It "Should have correct counter metric value" {
            $metric = $script:CapturedCounterMetrics[0]
            $metric.value | Should -Be 1.0
        }

        # Distribution metric assertions
        It "Should capture distribution metric in Sentry" {
            $script:CapturedDistributionMetrics | Should -Not -BeNullOrEmpty
        }

        It "Should have correct distribution metric name and type" {
            $metric = $script:CapturedDistributionMetrics[0]
            $metric.'metric.name' | Should -Be 'test.integration.distribution'
            $metric.'metric.type' | Should -Be 'distribution'
        }

        It "Should have correct distribution metric value" {
            $metric = $script:CapturedDistributionMetrics[0]
            $metric.value | Should -Be 42.5
        }

        # Gauge metric assertions
        It "Should capture gauge metric in Sentry" {
            $script:CapturedGaugeMetrics | Should -Not -BeNullOrEmpty
        }

        It "Should have correct gauge metric name and type" {
            $metric = $script:CapturedGaugeMetrics[0]
            $metric.'metric.name' | Should -Be 'test.integration.gauge'
            $metric.'metric.type' | Should -Be 'gauge'
        }

        It "Should have correct gauge metric value" {
            $metric = $script:CapturedGaugeMetrics[0]
            $metric.value | Should -Be 15.0
        }

        # BeforeMetricHandler attribute assertions (verified on counter, applies to all)
        It "Should have attribute added by BeforeMetricHandler" {
            $metric = $script:CapturedCounterMetrics[0]
            $metric.'handler_added' | Should -Be 'added_value'
        }

        It "Should not have attribute removed by BeforeMetricHandler" {
            $metric = $script:CapturedCounterMetrics[0]
            $metric.'to_be_removed' | Should -BeNullOrEmpty
        }

        It "Should have test_id attribute matching captured ID" {
            $metric = $script:CapturedCounterMetrics[0]
            $metric.test_id | Should -Be $script:TestId
        }
    }

    Context "Tracing Capture Tests" {
        BeforeAll {
            $script:TracingResult = $global:AndroidTracingResult
            $script:TransactionEvent = $null
            $script:TraceId = $null

            # Parse trace ID from output (format: TRACE_CAPTURED: <trace-id>)
            $traceLines = @($script:TracingResult.Output | Where-Object { $_ -match 'TRACE_CAPTURED: ' })
            if ($traceLines.Count -gt 0) {
                $script:TraceId = ($traceLines[0] -split 'TRACE_CAPTURED: ')[-1].Trim()
                Write-Host "Captured Trace ID: $($script:TraceId)" -ForegroundColor Cyan

                # Fetch transaction from Sentry using Get-SentryTestTransaction
                try {
                    $script:TransactionEvent = Get-SentryTestTransaction -TraceId $script:TraceId
                    Write-Host "Transaction fetched from Sentry successfully" -ForegroundColor Green
                }
                catch {
                    Write-Host "Failed to fetch transaction from Sentry: $_" -ForegroundColor Red
                }
            }
            else {
                Write-Host "Warning: No TRACE_CAPTURED line found in output" -ForegroundColor Yellow
            }
        }

        It "Should output TRACE_CAPTURED with trace ID" {
            $script:TraceId | Should -Not -BeNullOrEmpty
        }

        It "Should output TEST_RESULT with success" {
            $testResultLine = $script:TracingResult.Output | Where-Object { $_ -match 'TEST_RESULT:' }
            $testResultLine | Should -Not -BeNullOrEmpty
            $testResultLine | Should -Match '"success"\s*:\s*true'
        }

        It "Should capture transaction in Sentry" {
            $script:TransactionEvent | Should -Not -BeNullOrEmpty
        }

        It "Should have correct transaction name" {
            $script:TransactionEvent.title | Should -Be 'integration.tracing.test'
        }

        It "Should have correct transaction operation" {
            $script:TransactionEvent.contexts.trace.op | Should -Be 'e2e.test'
        }

        It "Should have test tags on transaction" {
            $tags = $script:TransactionEvent.tags
            ($tags | Where-Object { $_.key -eq 'test.type' }).value | Should -Be 'tracing'
            ($tags | Where-Object { $_.key -eq 'test.suite' }).value | Should -Be 'integration'
        }

        It "Should not have tag removed from transaction" {
            $tags = $script:TransactionEvent.tags
            $tags | Where-Object { $_.key -eq 'tracing.to_be_removed' } | Should -BeNullOrEmpty
        }

        It "Should have transaction data" {
            $script:TransactionEvent.contexts.trace.data.test_data | Should -Not -BeNullOrEmpty
            $script:TransactionEvent.contexts.trace.data.test_data.data_key | Should -Be 'data_value'
        }

        It "Should not have data removed from transaction" {
            $script:TransactionEvent.contexts.trace.data.data_to_be_removed | Should -BeNullOrEmpty
        }

        It "Should have child spans" {
            $script:TransactionEvent.spans | Should -Not -BeNullOrEmpty
            $script:TransactionEvent.spans.Count | Should -BeGreaterOrEqual 2
        }

        It "Should have child span with correct operation and description" {
            $childSpan = $script:TransactionEvent.spans | Where-Object { $_.op -eq 'e2e.child' }
            $childSpan | Should -Not -BeNullOrEmpty
            $childSpan.description | Should -Be 'Child span description'
        }

        It "Should have data on child span" {
            $childSpan = $script:TransactionEvent.spans | Where-Object { $_.op -eq 'e2e.child' }
            $childSpan.data.span_data | Should -Not -BeNullOrEmpty
            $childSpan.data.span_data.span_key | Should -Be 'span_value'
        }

        It "Should have grandchild span with correct operation and description" {
            $grandchildSpan = $script:TransactionEvent.spans | Where-Object { $_.op -eq 'e2e.grandchild' }
            $grandchildSpan | Should -Not -BeNullOrEmpty
            $grandchildSpan.description | Should -Be 'Grandchild span description'
        }

        It "Should have correct span hierarchy" {
            $childSpan = $script:TransactionEvent.spans | Where-Object { $_.op -eq 'e2e.child' }
            $grandchildSpan = $script:TransactionEvent.spans | Where-Object { $_.op -eq 'e2e.grandchild' }
            $grandchildSpan.parent_span_id | Should -Be $childSpan.span_id
        }
    }
}

AfterAll {
    # Disconnect from Sentry API
    Write-Host "Disconnecting from Sentry API..." -ForegroundColor Yellow
    Disconnect-SentryApi

    Write-Host "Integration tests complete" -ForegroundColor Green
}
