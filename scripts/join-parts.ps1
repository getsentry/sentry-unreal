#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Reassembles an archive that was published to a registry in fixed-size parts.

.DESCRIPTION
    `split -b 4G <archive> <archive>.part-` produces .part-aa, .part-ab, ... which
    concatenate back in ascending name order. Streams them so multi-GB archives do
    not have to fit in memory - Get-Content would both buffer and corrupt binary data.

.PARAMETER PartsDir
    Directory holding the parts.

.PARAMETER Destination
    Path of the archive to write.
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$PartsDir,

    [Parameter(Mandatory = $true)]
    [string]$Destination
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$parts = Get-ChildItem -Path $PartsDir -File -Recurse | Sort-Object Name
if ($parts.Count -eq 0) {
    throw "No parts found under $PartsDir"
}

Write-Host "Joining $($parts.Count) part(s) into $Destination"
$out = [System.IO.File]::Create($Destination)
try {
    foreach ($part in $parts) {
        Write-Host "  $($part.Name) ($([math]::Round($part.Length / 1GB, 2)) GB)"
        $in = [System.IO.File]::OpenRead($part.FullName)
        try { $in.CopyTo($out) } finally { $in.Dispose() }
    }
}
finally {
    $out.Dispose()
}

$size = [math]::Round((Get-Item $Destination).Length / 1GB, 2)
Write-Host "Wrote $Destination ($size GB)"
