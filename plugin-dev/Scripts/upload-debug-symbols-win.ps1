param([string] $TargetPlatform,[string] $TargetName,[string] $TargetType,[string] $ProjectPath,[string] $PluginPath)

$ProjectBinariesPath = "$ProjectPath\Binaries\$TargetPlatform"
$PluginBinariesPath = "$PluginPath\Source\ThirdParty\$TargetPlatform"
$ConfigPath = "$ProjectPath\Config"

Write-Host "Sentry: Start debug symbols upload"

If ($TargetType -eq "Editor")
{
    Write-Host "Sentry: Automatic symbols upload is not required for Editor target. Terminating..."
    Exit
}

If ($TargetPlatform -eq "Win64")
{
    Set-Variable -Name "CliExec" -Value "$PluginPath\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe"
}
Else
{
    Write-Host "Sentry: Unexpected platform $TargetPlatform. Terminating..."
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
            $name,$value = $matches[1..2]  
            $ini[$section][$name] = $value  
        }  
    }  

    return $ini  
}

$ConfigIni = ParseIniFile "$ConfigPath\DefaultEngine.ini"
$SentrySettingsSection = "/Script/Sentry.SentrySettings"

$UploadSymbols = $ConfigIni.$SentrySettingsSection.UploadSymbolsAutomatically

If (("$UploadSymbols" -eq "") -or ("$UploadSymbols" -eq "False"))
{
    Write-Host "Sentry: Automatic symbols upload is disabled in plugin settings. Terminating..."
    Exit
}

Write-Host "Sentry: Parse project settings"

$SentryProjectName = $ConfigIni.$SentrySettingsSection.ProjectName
$SentryProjectOrg = $ConfigIni.$SentrySettingsSection.OrganisationName
$SentryAuthToken = $ConfigIni.$SentrySettingsSection.AuthToken

Write-Host "Sentry: Copy user credentials config file template to home directory"

Copy-Item "$PluginPath\Resources\sentry.properties" -Destination "$ProjectBinariesPath\sentry.properties"

(Get-Content "$ProjectBinariesPath\sentry.properties") -replace "your-project", "$SentryProjectName" | Out-File "$ProjectBinariesPath\sentry.properties"
(Get-Content "$ProjectBinariesPath\sentry.properties") -replace "your-org", "$SentryProjectOrg" | Out-File "$ProjectBinariesPath\sentry.properties"
(Get-Content "$ProjectBinariesPath\sentry.properties") -replace "YOUR_AUTH_TOKEN", "$SentryAuthToken" | Out-File "$ProjectBinariesPath\sentry.properties"


$Env:SENTRY_PROPERTIES = "$ProjectBinariesPath\sentry.properties"
Write-Host "Sentry: $env:SENTRY_PROPERTIES"



Write-Host "Sentry: Upload started"

Push-Location  "$PluginPath\Source\ThirdParty\CLI"

.\sentry-cli-Windows-x86_64.exe upload-dif --include-sources --log-level info $ProjectBinariesPath $PluginBinariesPath

Pop-Location

Write-Host "Sentry: Upload finished"