param (
    [Parameter()]
    [string] $platform
)

switch ($platform) {
    "Android" {
        return "Modules/sentry-java"
    }
    "IOS" {
        return "Modules/sentry-cocoa"
    }
    "Win64" {
        return "Modules/sentry-native"
    }
    "Mac" {
        return "Modules/sentry-native"
    }
    "Linux" {
        return "Modules/sentry-native"
    }
    Default {
        throw "Unkown variable '$platform'"
    }
}
