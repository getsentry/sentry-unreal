param(
    [switch]$All,
    [switch]$Switch,
    [switch]$PS5,
    [switch]$XSX,
    [switch]$XB1
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Write-Host "Console Extension Setup" -ForegroundColor Cyan
Write-Host "=======================" -ForegroundColor Cyan

# If -All is specified, enable all platforms
if ($All) {
    $Switch = $true
    $PS5 = $true
    $XSX = $true
    $XB1 = $true
}

# Check if at least one platform is selected
if (-not ($Switch -or $PS5 -or $XSX -or $XB1)) {
    Write-Host "Error: No platform specified. Use -All or specify individual platforms (-Switch, -PS5, -XSX, -XB1)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Gray
    Write-Host "  ./scripts/init-consoles.ps1 -All" -ForegroundColor Gray
    Write-Host "  ./scripts/init-consoles.ps1 -Switch -PS5" -ForegroundColor Gray
    Write-Host "  ./scripts/init-consoles.ps1 -XSX -XB1" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Environment variables required:" -ForegroundColor Gray
    Write-Host "  SENTRY_SWITCH_PATH       - Path to sentry-switch repo" -ForegroundColor Gray
    Write-Host "  SENTRY_PLAYSTATION_PATH  - Path to sentry-playstation repo" -ForegroundColor Gray
    Write-Host "  SENTRY_XBOX_PATH         - Path to sentry-xbox repo" -ForegroundColor Gray
    exit 1
}

# Platform configuration with env var mapping
$platformConfigs = @{
    'Switch' = @{
        EnvVar = 'SENTRY_SWITCH_PATH'
        Enabled = $Switch
    }
    'PS5' = @{
        EnvVar = 'SENTRY_PLAYSTATION_PATH'
        Enabled = $PS5
    }
    'XSX' = @{
        EnvVar = 'SENTRY_XBOX_PATH'
        Enabled = $XSX
    }
    'XB1' = @{
        EnvVar = 'SENTRY_XBOX_PATH'
        Enabled = $XB1
    }
}

# Track results
$results = @()

# Process each enabled platform
foreach ($platform in $platformConfigs.Keys | Sort-Object) {
    $config = $platformConfigs[$platform]

    if (-not $config.Enabled) {
        continue
    }

    Write-Host "`n[$platform]" -ForegroundColor Cyan

    # Get extension path from environment variable
    $envVarName = $config.EnvVar
    $extensionPath = [Environment]::GetEnvironmentVariable($envVarName)

    if (-not $extensionPath) {
        Write-Host "ERROR: Environment variable '$envVarName' is not set" -ForegroundColor Red
        $results += @{
            Platform = $platform
            Status = "Failed"
            Message = "Environment variable not set: $envVarName"
        }
        continue
    }

    # Call init-console-ext.ps1 for this platform
    try {
        $scriptPath = Join-Path $PSScriptRoot "init-console-ext.ps1"
        & $scriptPath -Platform $platform -ExtensionPath $extensionPath

        if ($LASTEXITCODE -ne 0) {
            throw "Script failed with exit code $LASTEXITCODE"
        }

        $results += @{
            Platform = $platform
            Status = "Success"
            Message = "Setup completed"
        }
    }
    catch {
        Write-Host "ERROR: Failed to setup $platform - $_" -ForegroundColor Red
        $results += @{
            Platform = $platform
            Status = "Failed"
            Message = $_.Exception.Message
        }
    }
}

# Summary report
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$successCount = 0
$failCount = 0

foreach ($result in $results) {
    $statusColor = if ($result.Status -eq "Success") { "Green" } else { "Red" }
    $statusSymbol = if ($result.Status -eq "Success") { "✓" } else { "✗" }

    Write-Host "$statusSymbol $($result.Platform): " -NoNewline -ForegroundColor $statusColor
    Write-Host $result.Message -ForegroundColor Gray

    if ($result.Status -eq "Success") {
        $successCount++
    } else {
        $failCount++
    }
}

Write-Host ""
Write-Host "Total: $($results.Count) | Success: $successCount | Failed: $failCount" -ForegroundColor $(if ($failCount -eq 0) { "Green" } else { "Yellow" })

if ($failCount -gt 0) {
    exit 1
}
