param(
    [Parameter(Mandatory=$true)]
    [ValidateSet('Switch', 'PS5', 'XSX', 'XB1')]
    [string]$Platform,

    [Parameter(Mandatory=$true)]
    [string]$ExtensionPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Platform-specific configuration
$platformConfig = @{
    'Switch' = @{
        Preset = 'nx64-unreal'
        BuildDir = 'nx64-unreal'
        PlatformFolder = 'Switch'
        SourceDir = 'Sentry'
    }
    'PS5' = @{
        Preset = 'ps5-unreal'
        BuildDir = 'ps5-unreal'
        PlatformFolder = 'PS5'
        SourceDir = 'Sentry'
    }
    'XSX' = @{
        Preset = 'scarlett-unreal'
        BuildDir = 'scarlett-unreal'
        PlatformFolder = 'XSX'
        SourceDir = 'Sentry_XSX'
    }
    'XB1' = @{
        Preset = 'xboxone-unreal'
        BuildDir = 'xboxone-unreal'
        PlatformFolder = 'XB1'
        SourceDir = 'Sentry_XB1'
    }
}

$config = $platformConfig[$Platform]
$repoRoot = Resolve-Path "$PSScriptRoot/.."

Write-Host "Setting up $Platform console extension..." -ForegroundColor Cyan
Write-Host "Extension path: $ExtensionPath"

# Step 1: Validate extension path
Write-Host "`n[1/5] Validating extension path..." -ForegroundColor Yellow

if (-not (Test-Path $ExtensionPath)) {
    throw "Extension path does not exist: $ExtensionPath"
}
$ExtensionPath = Resolve-Path $ExtensionPath

$cmakeFile = Join-Path $ExtensionPath "CMakeLists.txt"
if (-not (Test-Path $cmakeFile)) {
    throw "CMakeLists.txt not found in extension path: $ExtensionPath"
}

$unrealDir = Join-Path $ExtensionPath "unreal"
if (-not (Test-Path $unrealDir)) {
    throw "unreal directory not found in extension path: $ExtensionPath"
}

$sourceDir = Join-Path $unrealDir $config.SourceDir
if (-not (Test-Path $sourceDir)) {
    throw "Source directory not found: $sourceDir"
}

Write-Host "  ✓ Extension path validated" -ForegroundColor Green

# Step 2: Build extension
Write-Host "`n[2/5] Building extension..." -ForegroundColor Yellow
Write-Host "  Running: cmake --workflow --preset $($config.Preset) --fresh"

Push-Location $ExtensionPath
try {
    $buildOutput = & cmake --workflow --preset $config.Preset --fresh 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host $buildOutput -ForegroundColor Red
        throw "Build failed with exit code $LASTEXITCODE"
    }
    Write-Host "  ✓ Extension built successfully" -ForegroundColor Green
} finally {
    Pop-Location
}

# Verify build output exists
$buildOutputDir = Join-Path $ExtensionPath "build" $config.BuildDir "unreal" "Sentry"
if (-not (Test-Path $buildOutputDir)) {
    throw "Build output directory not found: $buildOutputDir"
}

# Step 3: Create directory structure in sample project
Write-Host "`n[3/5] Creating directory structure..." -ForegroundColor Yellow

$targetPluginRoot = Join-Path $repoRoot "sample" "Platforms" $config.PlatformFolder "Plugins" "Sentry"
$targetSourceRoot = Join-Path $targetPluginRoot "Source"
$targetThirdPartyDir = Join-Path $targetSourceRoot "ThirdParty" $config.PlatformFolder

# Create all necessary directories
$dirsToCreate = @(
    $targetPluginRoot,
    $targetSourceRoot,
    (Join-Path $targetSourceRoot "Sentry" "Private"),
    $targetThirdPartyDir
)

foreach ($dir in $dirsToCreate) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
        Write-Host "  Created: $dir"
    }
}

Write-Host "  ✓ Directory structure created" -ForegroundColor Green

# Step 4: Copy build artifacts (ThirdParty libs/headers)
Write-Host "`n[4/5] Copying build artifacts..." -ForegroundColor Yellow

$buildThirdPartyDir = Join-Path $buildOutputDir "Source" "ThirdParty" $config.PlatformFolder
if (-not (Test-Path $buildThirdPartyDir)) {
    throw "Build ThirdParty directory not found: $buildThirdPartyDir"
}

# Remove existing ThirdParty directory if it exists to ensure clean copy
if (Test-Path $targetThirdPartyDir) {
    Remove-Item -Path $targetThirdPartyDir -Recurse -Force
}

# Copy entire ThirdParty directory
Copy-Item -Path $buildThirdPartyDir -Destination (Join-Path $targetSourceRoot "ThirdParty") -Recurse -Force
Write-Host "  Copied: ThirdParty/$($config.PlatformFolder)/"

Write-Host "  ✓ Build artifacts copied" -ForegroundColor Green

# Step 5: Symlink source files for live editing
Write-Host "`n[5/5] Creating symlinks to source files..." -ForegroundColor Yellow

# Helper function to create symlink/junction
function New-SymbolicLink {
    param(
        [string]$LinkPath,
        [string]$TargetPath
    )

    # Remove existing link if it exists
    if (Test-Path $LinkPath) {
        $item = Get-Item $LinkPath
        if ($item.LinkType -eq "Junction" -or $item.LinkType -eq "SymbolicLink") {
            $item.Delete()
        } else {
            Remove-Item -Path $LinkPath -Recurse -Force
        }
    }

    # Create parent directory if needed
    $parentDir = Split-Path -Parent $LinkPath
    if (-not (Test-Path $parentDir)) {
        New-Item -ItemType Directory -Path $parentDir -Force | Out-Null
    }

    # Create junction (works cross-platform with PowerShell)
    if (Test-Path $TargetPath -PathType Container) {
        New-Item -ItemType Junction -Path $LinkPath -Target $TargetPath | Out-Null
    } else {
        New-Item -ItemType SymbolicLink -Path $LinkPath -Target $TargetPath | Out-Null
    }
}

# Symlink .uplugin file
$upluginFiles = @(Get-ChildItem -Path $sourceDir -Filter "*.uplugin" -ErrorAction SilentlyContinue)
if ($upluginFiles.Count -eq 0) {
    throw "No .uplugin file found in $sourceDir"
}
$upluginTarget = $upluginFiles[0].FullName
$upluginLink = Join-Path $targetPluginRoot $upluginFiles[0].Name
New-SymbolicLink -LinkPath $upluginLink -TargetPath $upluginTarget
Write-Host "  Linked: $($upluginFiles[0].Name)"

# Symlink .Build.cs file
$buildcsFiles = @(Get-ChildItem -Path (Join-Path $sourceDir "Source") -Filter "*.Build.cs" -ErrorAction SilentlyContinue)
if ($buildcsFiles.Count -gt 0) {
    $buildcsTarget = $buildcsFiles[0].FullName
    $buildcsLink = Join-Path $targetSourceRoot $buildcsFiles[0].Name
    New-SymbolicLink -LinkPath $buildcsLink -TargetPath $buildcsTarget
    Write-Host "  Linked: Source/$($buildcsFiles[0].Name)"
}

# Symlink Sentry/Private directory (contains .cpp/.h files)
$privateSourceDir = Join-Path $sourceDir "Source" "Sentry" "Private"
if (Test-Path $privateSourceDir) {
    $privateTargetDir = Join-Path $targetSourceRoot "Sentry" "Private"
    New-SymbolicLink -LinkPath $privateTargetDir -TargetPath $privateSourceDir
    Write-Host "  Linked: Source/Sentry/Private/"
}

Write-Host "  ✓ Source files symlinked" -ForegroundColor Green

Write-Host "`n✓ Console extension setup complete!" -ForegroundColor Green
Write-Host "`nTarget location: $targetPluginRoot" -ForegroundColor Cyan
