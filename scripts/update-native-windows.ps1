param([string] $Action, [string] $Tag)

$submodulePath = "modules/sentry-native"
$targetPlatform = "Win64"

function get-version ()
{
    git submodule update --init --no-fetch --single-branch $submodulePath | Out-Null
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

    git submodule foreach git submodule update --init --recursive

    & ./scripts/build-native-windows.ps1 $submodulePath plugin-dev/Source/ThirdParty/$targetPlatform
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