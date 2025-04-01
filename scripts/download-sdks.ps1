Set-StrictMode -Version latest

Write-Host "Downloading native SDKs from the latest CI pipeline"

function findCiRun([string] $branch)
{
    Write-Host "Looking for the latest successful CI run on branch '$branch'"
    $jsonArray = gh run list --branch $branch --workflow package-plugin-workflow --json 'conclusion,databaseId' | ConvertFrom-Json
    $id = $jsonArray | Where-Object 'conclusion' -EQ 'success' | Select-Object -First 1 -ExpandProperty 'databaseId'
    if ( "$id" -eq "" )
    {
        Write-Warning "  ... no successful CI run found on $branch"
    }
    else
    {
        Write-Host "  ... found CI run ID: $id"
        "$id"
    }
}

$runId = findCiRun("$(git rev-parse --abbrev-ref HEAD)")
if ( "$runId" -eq "" )
{
    $runId = findCiRun("main")
    if ( "$runId" -eq "" )
    {
        exit 1
    }
}

$outDir = "$(Resolve-Path "$PSScriptRoot/../plugin-dev/Source")/ThirdParty"
if (-not (Test-Path $outDir))
{
    New-Item $outDir -ItemType Directory > $null
}

$sdks = @("Android", "IOS", "Linux", "LinuxArm64", "Mac", "Win64-Crashpad", "Win64-Breakpad")
foreach ($sdk in $sdks)
{
    $sdkDir = "$outDir/$sdk"

    if ($sdk.StartsWith('Win64'))
    {
        $winSdk, $crashBackend = $sdk.Split("-")
        $sdkDir = "$outDir/$winSdk/$crashBackend"
    }

    Write-Host "Downloading $sdk SDK to $sdkDir ..."
    if (Test-Path $sdkDir)
    {
        Remove-Item "$sdkDir" -Recurse
    }

    gh run download $runId -n "$sdk-sdk" -D $sdkDir
}
