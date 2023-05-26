@echo off

setlocal enabledelayedexpansion

set TargetPlatform=%1
set TargetName=%2
set TargetType=%3
set ProjectPath=%4
set PluginPath=%5

set ProjectBinariesPath=%ProjectPath:"=%\Binaries\%TargetPlatform%
set PluginBinariesPath=%PluginPath:"=%\Source\ThirdParty\%TargetPlatform%
set ConfigPath=%ProjectPath:"=%\Config

echo Sentry: Start debug symbols upload

if "%TargetType%"=="Editor" (
    echo Sentry: Automatic symbols upload is not required for Editor target. Skipping...
    exit
)

if "%TargetPlatform%"=="Win64" (
    set CliExec=%PluginPath%\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe
) else if "%TargetPlatform%"=="Linux" (
    set CliExec=%PluginPath%\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe
) else if "%TargetPlatform%"=="Android" (
    echo Warning: Sentry: Debug symbols upload for Android is handled by Sentry's gradle plugin if enabled
    exit
) else (
    echo Warning: Sentry: Unexpected platform %TargetPlatform%. Skipping...
    exit
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings UploadSymbolsAutomatically UploadSymbols

if /i "%UploadSymbols%" NEQ "True" (
    echo Sentry: Automatic symbols upload is disabled in plugin settings. Skipping...
    exit
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings IncludeSources IncludeSourceFiles

set CliArgs=

if /i "%IncludeSourceFiles%"=="True" (
    set "CliArgs=--include-sources"
)

set PropertiesFile=%ProjectPath:"=%\sentry.properties

if not exist "%PropertiesFile%" (
    echo Warning: Sentry: Properties file is missing: '%PropertiesFile%'
    exit
)

echo Sentry: Upload started using PropertiesFile '%PropertiesFile%'

set "SENTRY_PROPERTIES=%PropertiesFile%"
echo %ProjectBinariesPath%
echo %PluginBinariesPath%
call "%CliExec%" upload-dif %CliArgs% --log-level info "%ProjectBinariesPath%" "%PluginBinariesPath%"

echo Sentry: Upload finished

endlocal
exit

:ParseIniFile <filename> <section> <key> <result>
  set %~4=
  setlocal
  set insection=
  for /f "usebackq eol=; tokens=*" %%a in ("%~1") do (
    set line=%%a
    if defined insection (
      for /f "tokens=1,* delims==" %%b in ("!line!") do (
        if /i "%%b"=="%3" (
          endlocal
          set %~4=%%c
          set insection=
          goto :eof
        )
      )
    )
    if "!line:~0,1!"=="[" (
      for /f "delims=[]" %%b in ("!line!") do (
        if /i "%%b"=="%2" (
          set insection=1
        ) else (
          endlocal
          if defined insection goto :eof
        )
      )
    )
  )
  endlocal