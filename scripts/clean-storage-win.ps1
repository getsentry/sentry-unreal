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

Stop-Service -Name wuauserv -Force
Remove-Item -Path "C:\Windows\SoftwareDistribution\*" -Recurse -Force -ErrorAction SilentlyContinue
Start-Service -Name wuauserv
printSpaceFreed "Windows Update Cache Cleanup"

Dism.exe /online /Cleanup-Image /StartComponentCleanup /ResetBase
printSpaceFreed "Windows Component Store Cleanup"

Get-ChildItem -Path "C:\Program Files (x86)\Windows Kits\10\Catalogs" -Recurse | Remove-Item -Force -ErrorAction SilentlyContinue
printSpaceFreed "Windows Kits Catalogs Cleanup"

Remove-Item -Path "C:\Windows.old" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\`$WINDOWS.~BT" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\`$WINDOWS.~WS" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Windows Update Backup Folders Cleanup"

Remove-Item -Path "C:\Windows\symbols\*" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Symbol Cache Cleanup"

Remove-Item -Path "C:\Windows\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Users\*\AppData\Local\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Temp Files Cleanup"

Remove-Item -Path "C:\ProgramData\Package Cache\*" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Package Cache Cleanup"

Remove-Item -Path "C:\ProgramData\Microsoft\VisualStudio\Packages\*.vsix" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout\*.vsix" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Visual Studio Cache Cleanup"

Remove-Item -Path "C:\ghcup" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Haskell Removal"

Remove-Item -Path "C:\Program Files\Android" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Program Files (x86)\Android" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Android SDK Removal"

Remove-Item -Path "C:\Program Files\nodejs\node_modules" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Node.js Modules Cleanup"

Remove-Item -Path "C:\Program Files\MongoDB" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "MongoDB Removal"

Remove-Item -Path "C:\Program Files\MySQL" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "MySQL Removal"

Remove-Item -Path "C:\Program Files\PostgreSQL" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "PostgreSQL Removal"

Remove-Item -Path "C:\Users\runneradmin\AppData\Local\Temp\chocolatey" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Chocolatey Cache Cleanup"

Remove-Item -Path "C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Program Files (x86)\Windows Kits\10\Microsoft Application Inspector" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Windows Kits Components Removal"

Remove-Item -Path "C:\rtools44" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "R Tools Removal"

Remove-Item -Path "C:\Julia" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Julia Removal"

Remove-Item -Path "C:\Miniconda" -Recurse -Force -ErrorAction SilentlyContinue
printSpaceFreed "Miniconda Removal"

docker system prune -a -f
printSpaceFreed "Docker Cleanup"

Write-Host "Initial free space: $([math]::Round($initialSpace, 2)) GB"
Write-Host "Final free space: $([math]::Round($finalSpace, 2)) GB"
Write-Host "Total space gained: $([math]::Round($totalGained, 2)) GB"