#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Configures manual iOS code signing for an Unreal project.

.DESCRIPTION
    This script writes a platform-specific config override that points Unreal at a
    provisioning profile and signing team. It exists so that signing settings only
    apply in CI, where fastlane installs the matching certificate and profile - local
    builds keep whatever is configured in the editor.

    The settings live in Config/IOS/IOSEngine.ini rather than DefaultEngine.ini
    because the XcodeProjectSettings section is shared by Mac and iOS: setting a
    signing team there makes macOS builds require a "Mac Development" certificate.

.PARAMETER ProjectPath
    Path to the Unreal project directory.

.PARAMETER TeamId
    Apple Developer Portal team ID to sign with.

.PARAMETER ProvisioningProfile
    Project-relative path to the .mobileprovision file to sign with. Unreal reads this
    setting as an FFilePath, so it must point at a file - a bare profile name fails
    UObject config import and makes the cook commandlet fail.

.EXAMPLE
    ./configure-ios-signing.ps1 -ProjectPath "sample" -TeamId "XXXXXXXXXX"
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath,

    [Parameter(Mandatory = $true)]
    [string]$TeamId,

    [string]$ProvisioningProfile = "/Game/Build/IOS/match.mobileprovision"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $ProjectPath)) {
    Write-Error "Project directory not found at: $ProjectPath"
    exit 1
}

$configDir = Join-Path $ProjectPath "Config/IOS"
$configPath = Join-Path $configDir "IOSEngine.ini"

Write-Host "Configuring iOS code signing for: $ProjectPath"

New-Item -ItemType Directory -Path $configDir -Force | Out-Null

$signingConfig = @"
[/Script/MacTargetPlatform.XcodeProjectSettings]
CodeSigningTeam=$TeamId
bUseAutomaticCodeSigning=False
IOSProvisioningProfile=(FilePath="$ProvisioningProfile")
"@

Set-Content -Path $configPath -Value $signingConfig

Write-Host "✓ Signing configuration written to $configPath"
Write-Host "  Team ID: $TeamId"
Write-Host "  Provisioning profile: $ProvisioningProfile"
