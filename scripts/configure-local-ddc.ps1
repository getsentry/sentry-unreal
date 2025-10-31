#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Configures Unreal Engine to use project-local Derived Data Cache (DDC).

.DESCRIPTION
    This script modifies the project's DefaultEngine.ini to add DDC configuration
    that forces Unreal to store the DDC within the project directory instead of
    the user's home directory. This enables DDC caching in CI environments.

.PARAMETER ProjectPath
    Path to the Unreal project directory containing Config/DefaultEngine.ini

.EXAMPLE
    ./configure-local-ddc.ps1 -ProjectPath "sample"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectPath
)

$ErrorActionPreference = "Stop"

$configPath = Join-Path $ProjectPath "Config/DefaultEngine.ini"

if (-not (Test-Path $configPath)) {
    Write-Error "DefaultEngine.ini not found at: $configPath"
    exit 1
}

Write-Host "Configuring project-local DDC for: $ProjectPath"

$ddcConfig = @"

[InstalledDerivedDataBackendGraph]
MinimumDaysToKeepFile=7
Root=(Type=KeyLength, Length=120, Inner=AsyncPut)
AsyncPut=(Type=AsyncPut, Inner=Hierarchy)
Hierarchy=(Type=Hierarchical, Inner=Boot, Inner=Pak, Inner=EnginePak, Inner=Local, Inner=Shared)
Boot=(Type=Boot, Filename="%GAMEDIR%DerivedDataCache/Boot.ddc", MaxCacheSize=512)
Local=(Type=FileSystem, ReadOnly=false, Clean=false, Flush=false, PurgeTransient=true, DeleteUnused=true, UnusedFileAge=34, FoldersToClean=-1, Path="%GAMEDIR%DerivedDataCache", EnvPathOverride=UE-LocalDataCachePath, EditorOverrideSetting=LocalDerivedDataCache)
Shared=(Type=FileSystem, ReadOnly=false, Clean=false, Flush=false, DeleteUnused=true, UnusedFileAge=10, FoldersToClean=-1, Path="%GAMEDIR%DerivedDataCache/Shared", EnvPathOverride=UE-SharedDataCachePath, EditorOverrideSetting=SharedDerivedDataCache, CommandLineOverride=SharedDataCachePath)
Pak=(Type=ReadPak, Filename="%GAMEDIR%DerivedDataCache/DDC.ddp")
EnginePak=(Type=ReadPak, Filename=%ENGINEDIR%DerivedDataCache/Compressed.ddp)
"@

Add-Content -Path $configPath -Value $ddcConfig

Write-Host "✓ DDC configuration added to $configPath"
Write-Host "  DDC will be stored at: <ProjectDir>/DerivedDataCache/"
