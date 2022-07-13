param(
    # Path to the dependency, which can be either of the following:
    #  - a submodule
    #  - a [.properties](https://en.wikipedia.org/wiki/.properties) file with `version` (e.g. 1.0.0) and `repo` (e.g. https://github.com/getsentry/dependency)
    #  - a script (.sh, .ps1) that takes the executes a given action based on a given argument:
    #    * `get-version` - return the currently specified dependency version
    #    * `get-repo` - return the repository url (e.g.  https://github.com/getsentry/dependency)
    #    * `set-version` - update the dependency version (passed as another string argument after this one)
    [Parameter(Mandatory = $true)][string] $Path,
    # RegEx pattern that will be matched against available versions when picking the latest one
    [string] $Pattern = '',
    # Specific version - if passed, no discovery is performed and the version is set directly
    [string] $Tag = ''
)

Set-StrictMode -Version latest

if (-not (Test-Path $Path ))
{
    throw "Dependency $Path doesn't exit";
}

# If it's a directory, we consider it a submodule dependendency. Otherwise, it must a properties-style file
$isSubmodule = (Test-Path $Path -PathType Container)

if (-not $isSubmodule)
{
    $isScript = $Path -match '\.(ps1|sh)$'
    function DependencyConfig ([Parameter(Mandatory = $true)][string] $action, [string] $value = $null)
    {
        if ($isScript)
        {
            if (Get-Command 'chmod' -ErrorAction SilentlyContinue)
            {
                chmod +x $Path
            }
            $result = & $Path $action $value
            if (-not $?)
            {
                throw "Script execution failed: $Path $action $value | output: $result"
            }
            return $result
        }
        else
        {
            switch ($action)
            {
                "get-version"
                {
                    return (Get-Content $Path -Raw | ConvertFrom-StringData).version
                }
                "get-repo"
                {
                    return (Get-Content $Path -Raw | ConvertFrom-StringData).repo
                }
                "set-version"
                {
                    $content = Get-Content $Path
                    $content = $content -replace "^(?<prop>version *= *).*$", "`${prop}$value"
                    $content | Out-File $Path

                    $readVersion = (Get-Content $Path -Raw | ConvertFrom-StringData).version

                    if ("$readVersion" -ne "$value")
                    {
                        throw "Update failed - read-after-write yielded '$readVersion' instead of expected '$value'"
                    }
                }
                Default
                {
                    throw "Unknown action $action"
                }
            }
        }
    }
}

if ("$Tag" -eq "")
{
    if ($isSubmodule)
    {
        git submodule update --init --no-fetch --single-branch $Path
        Push-Location $Path
        try
        {
            $originalTag = $(git describe --tags)
            git fetch --tags
            [string[]]$tags = $(git tag --list)
            $url = $(git remote get-url origin)
            $mainBranch = $(git remote show origin | Select-String "HEAD branch: (.*)").Matches[0].Groups[1].Value
        }
        finally
        {
            Pop-Location
        }
    }
    else
    {
        $originalTag = DependencyConfig 'get-version'
        $url = DependencyConfig 'get-repo'

        # Get tags for a repo without cloning.
        [string[]]$tags = $(git ls-remote --refs --tags $url)
        $tags = $tags | ForEach-Object { ($_ -split "\s+")[1] -replace '^refs/tags/', '' }

        $headRef = ($(git ls-remote $url HEAD) -split "\s+")[0]
        if ("$headRef" -eq '') {
            throw "Couldn't determine repository head (no ref returned by ls-remote HEAD"
        }
        $mainBranch = (git ls-remote --heads $url | Where-Object { $_.StartsWith($headRef) }) -replace '.*\srefs/heads/', ''
    }

    $url = $url -replace '\.git$', ''

    if ("$Pattern" -eq '')
    {
        # Use a default pattern that excludes pre-releases
        $Pattern = '^v?([0-9.]+)$'
    }

    Write-Host "Filtering tags with pattern '$Pattern'"
    $tags = $tags -match $Pattern

    if ($tags.Length -le 0)
    {
        throw "Found no tags matching pattern '$Pattern'"
    }

    $tags = & "$PSScriptRoot/sort-versions.ps1" $tags

    Write-Host "Sorted tags: $tags"
    $latestTag = $tags[-1]
    $latestTagNice = ($latestTag -match "^[0-9]") ? "v$latestTag" : $latestTag

    Write-Host '::echo::on'
    Write-Host "::set-output name=originalTag::$originalTag"
    Write-Host "::set-output name=latestTag::$latestTag"
    Write-Host "::set-output name=latestTagNice::$latestTagNice"
    Write-Host "::set-output name=url::$url"
    Write-Host "::set-output name=mainBranch::$mainBranch"

    if ("$originalTag" -eq "$latestTag") {
        return
    }
    $Tag = $latestTag
}

if ($isSubmodule)
{
    Write-Host "Updating submodule $Path to $Tag"
    Push-Location $Path
    git checkout $Tag
    Pop-Location
}
else
{
    Write-Host "Updating 'version' in $Path to $Tag"
    DependencyConfig 'set-version' $tag
}