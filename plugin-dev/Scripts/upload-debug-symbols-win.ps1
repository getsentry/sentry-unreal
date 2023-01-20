param([string] $TargetPlatform, [string] $TargetName, [string] $TargetType, [string] $ProjectPath, [string] $PluginPath)

$ProjectBinariesPath = "$ProjectPath\Binaries\$TargetPlatform"
$PluginBinariesPath = "$PluginPath\Source\ThirdParty\$TargetPlatform"
$ConfigPath = "$ProjectPath\Config"

Write-Host "Sentry: Start debug symbols upload"

If ($TargetType -eq "Editor")
{
    Write-Host "Sentry: Automatic symbols upload is not required for Editor target. Skipping..."
    Exit
}

If ($TargetPlatform -eq "Win64" -or $TargetPlatform -eq "Linux")
{
    $CliExec = "$PluginPath\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe"
}
Elseif ($TargetPlatform -eq "Android")
{
    Write-Warning "Sentry: Debug symbols upload for Android is handled by Sentry's gradle plugin if enabled"
    Exit
}
Else
{
    Write-Warning "Sentry: Unexpected platform $TargetPlatform. Skipping..."
    Exit
}

function ParseIniFile
{
    param([parameter(Mandatory = $true)] [string] $filePath)

    $anonymous = "NoSection"

    $ini = @{}
    switch -regex -file $filePath
    {
        "^\[(.+)\]$" # Section
        {
            $section = $matches[1]
            $ini[$section] = @{}
            $CommentCount = 0
        }

        "^(;.*)$" # Comment
        {
            if (!($section))
            {
                $section = $anonymous
                $ini[$section] = @{}
            }
            $value = $matches[1]
            $CommentCount = $CommentCount + 1
            $name = "Comment" + $CommentCount
            $ini[$section][$name] = $value
        }

        "(.+?)\s*=\s*(.*)" # Key
        {
            if (!($section))
            {
                $section = $anonymous
                $ini[$section] = @{}
            }
            $name, $value = $matches[1..2]
            $ini[$section][$name] = $value
        }
    }

    return $ini
}

$ConfigIni = ParseIniFile "$ConfigPath\DefaultEngine.ini"
$SentrySettingsSection = "/Script/Sentry.SentrySettings"

$UploadSymbols = $ConfigIni.$SentrySettingsSection.UploadSymbolsAutomatically

If ("$UploadSymbols".ToLower() -ne "true")
{
    Write-Host "Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."
    Exit
}

$PropertiesFile = "$ProjectPath/sentry.properties"

If (-not (Test-Path -Path $PropertiesFile -PathType Leaf))
{
    Write-Error "Sentry: Properties file is missing: '$PropertiesFile'"
    Exit 1
}

Write-Host "Sentry: Upload started using PropertiesFile '$PropertiesFile'"

$env:SENTRY_PROPERTIES = $PropertiesFile
& $CliExec upload-dif --include-sources --log-level info $ProjectBinariesPath $PluginBinariesPath

Write-Host "Sentry: Upload finished"