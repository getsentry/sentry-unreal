# Shared utilities for plugin packaging scripts

function Get-PluginSpecPath {
    return "$PSScriptRoot/../../plugin-dev/Sentry.uplugin"
}

function Get-PluginVersion {
    $pluginSpecPath = Get-PluginSpecPath

    if (-not(Test-Path -Path $pluginSpecPath)) {
        throw "Plugin spec file not found at: $pluginSpecPath"
    }

    $pluginSpec = Get-Content $pluginSpecPath
    $pluginVersion = [regex]::Match("$pluginSpec", '"VersionName": "([^"]+)"').Groups[1].Value

    if ([string]::IsNullOrEmpty($pluginVersion)) {
        throw "Failed to extract version from plugin spec"
    }

    return $pluginVersion
}
