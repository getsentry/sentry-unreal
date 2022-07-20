param([string] $Action,[string] $Tag)

$submodulePath="Modules/sentry-native"
$targetPlatform="Win64"

function get-version ()
{
    git submodule update --init --no-fetch --single-branch $submodulePath | out-null
    $version = $(git -C $submodulePath describe --tags)
    return $version
}

function get-repo ()
{
    $repo = "https://github.com/getsentry/sentry-native.git"
    return $repo
}

function set-version ([string] $version)
{
    git -C $submodulePath checkout $version

    git submodule update --init --recursive

    & ./Scripts/build-native-windows.ps1 $submodulePath Source/ThirdParty/$targetPlatform
}

switch ($Action)
{
    "get-version"
    {
        return get-version
    }
    "get-repo"
    {
        return get-repo
    }
    "set-version"
    {
        set-version $Tag
    }
    Default
    {
        throw "Unknown action $Action"
    }
}