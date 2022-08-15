Remove-Item "package-release" -Recurse -ErrorAction SilentlyContinue
New-Item "package-release" -ItemType Directory

$exclude = @()

Copy-Item "plugin-dev/*" "package-release/" -Exclude $exclude -Recurse

Copy-Item "CHANGELOG.md" -Destination "package-release/CHANGELOG.md"
Copy-Item "LICENSE" -Destination "package-release/LICENSE"

# Create zip
Compress-Archive "package-release/*" -DestinationPath "package-release.zip" -Force
