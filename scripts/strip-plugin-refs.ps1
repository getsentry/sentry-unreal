#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Removes specified plugin references from a .uproject's Plugins array.

.DESCRIPTION
    Strips named plugin references from a Unreal project file in place. Used in CI
    to handle engine-version skew where a plugin enabled in the
    uproject doesn't exist in an older engine (e.g., AVCodecsCore / NVCodecs are
    UE 5.0+ and absent in UE 4.27 causing "Unable to find plugin" build failures).

.PARAMETER ProjectPath
    Path to the .uproject file to modify in place.

.PARAMETER PluginNames
    Names of plugins to remove from the Plugins array (case-sensitive match).
    Pass one or more names, comma-separated.

.EXAMPLE
    ./strip-plugin-refs.ps1 -ProjectPath sample/SentryPlayground.uproject -PluginNames AVCodecsCore,NVCodecs
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath,
    [Parameter(Mandatory = $true)]
    [string[]]$PluginNames
)

if (-not (Test-Path $ProjectPath)) {
    Write-Error "uproject not found: $ProjectPath"
    exit 1
}

$json = Get-Content $ProjectPath -Raw | ConvertFrom-Json
$before = $json.Plugins.Count
$json.Plugins = @($json.Plugins | Where-Object { $_.Name -notin $PluginNames })
$removed = $before - $json.Plugins.Count

$json | ConvertTo-Json -Depth 10 | Set-Content -Path $ProjectPath -Encoding utf8NoBOM

Write-Host "Stripped $removed plugin reference(s) ($($PluginNames -join ', ')) from $ProjectPath"
