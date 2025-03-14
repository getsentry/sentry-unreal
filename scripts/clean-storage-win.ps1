Set-StrictMode -Version latest

function printSpaceFreed([string] $stepName) {
    $currentSpace = (Get-PSDrive C).Free / 1GB
    $spaceFreed = $currentSpace - $script:lastReportedSpace
    Write-Host "After $stepName - Freed: $([math]::Round($spaceFreed, 2)) GB, Current free space: $([math]::Round($currentSpace, 2)) GB"
    $script:lastReportedSpace = $currentSpace
    $script:totalFreed += $spaceFreed
}

$initialSpace = (Get-PSDrive C).Free / 1GB

$script:lastReportedSpace = $initialSpace
$script:totalFreed = 0

Write-Host "Initial free space: $([math]::Round($initialSpace, 2)) GB"

# ~33.89 GB
Remove-Item -Path "C:\Program Files (x86)\Microsoft Visual Studio" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Program Files\Microsoft Visual Studio" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Visual Studio Removal"

# ~1.21 GB
Remove-Item -Path "C:\Windows\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Users\*\AppData\Local\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Temp Files Cleanup"

# ~4.26 GB
Remove-Item -Path "C:\ProgramData\Package Cache\*" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Package Cache Cleanup"

# ~12.44 GB
Remove-Item -Path "C:\ghcup" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Haskell Removal"

# ~9.25 GB
Remove-Item -Path "C:\hostedtoolcache" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "hostedtoolcache Removal"

# ~11.78 GB
Remove-Item -Path "C:\Program Files\Android" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Program Files (x86)\Android" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Android SDK Removal"

# ~1.21 GB
Remove-Item -Path "C:\Program Files\MongoDB" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "MongoDB Removal"

# ~0.54 GB
Remove-Item -Path "C:\Program Files\MySQL" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "MySQL Removal"

# ~0.89 GB
Remove-Item -Path "C:\Program Files\PostgreSQL" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "PostgreSQL Removal"

# ~11.05 GB
Remove-Item -Path "C:\Program Files (x86)\Windows Kits" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Windows Kits Components Removal"

# ~11.05 GB
Remove-Item -Path "C:\Program Files (x86)\Microsoft SDKs" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Microsoft SDKs Removal"

# ~0.58 GB
Remove-Item -Path "C:\Program Files (x86)\Epic Games" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Epic Games Removal"

# ~2.93 GB
Remove-Item -Path "C:\rtools44" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "R Tools Removal"

# ~0.68 GB
Remove-Item -Path "C:\Julia" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Julia Removal"

# ~0.46 GB
Remove-Item -Path "C:\Miniconda" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Miniconda Removal"

# ~10.24 GB
docker system prune -a -f
printSpaceFreed "Docker Cleanup"

# ~10.29 GB
Remove-Item -Path "C:\Program Files\dotnet" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed ".NET Cleanup"

# ~2.65 GB
Remove-Item -Path "C:\Program Files\LLVM" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "LLVM Cleanup"

# ~1,72 GB
Remove-Item -Path "C:\Program Files\Azure Cosmos DB Emulator" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Azure Cosmos DB Emulator Cleanup"

# ~0.76 GB
Remove-Item -Path "C:\Program Files\Google" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Google Cleanup"

$finalSpace = (Get-PSDrive C).Free / 1GB

Write-Host "Initial free space: $([math]::Round($initialSpace, 2)) GB"
Write-Host "Final free space: $([math]::Round($finalSpace, 2)) GB"
Write-Host "Total space gained: $([math]::Round($totalFreed, 2)) GB"
