@echo off

setlocal enabledelayedexpansion

set TargetPlatform=%1
set TargetName=%2
set TargetType=%3
set TargetConfig=%4
set ProjectPath=%5
set PluginPath=%6

set ProjectBinariesPath=%ProjectPath:"=%\Binaries\%TargetPlatform%
set PluginBinariesPath=%PluginPath:"=%\Source\ThirdParty\%TargetPlatform%
set ConfigPath=%ProjectPath:"=%\Config

echo Sentry: Start debug symbols upload

if "%TargetType%"=="Editor" (
    echo Sentry: Automatic symbols upload is not required for Editor target. Skipping...
    exit /B 0
)

if "%TargetPlatform%"=="Win64" (
    set CliExec=%PluginPath:"=%\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe
) else if "%TargetPlatform%"=="Linux" (
    set CliExec=%PluginPath:"=%\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe
) else if "%TargetPlatform%"=="LinuxArm64" (
    set CliExec=%PluginPath:"=%\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe
) else if "%TargetPlatform%"=="Android" (
    echo Warning: Sentry: Debug symbols upload for Android is handled by Sentry's gradle plugin if enabled
    exit /B 0
) else (
    echo Warning: Sentry: Unexpected platform %TargetPlatform%. Skipping...
    exit /B 0
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings UploadSymbolsAutomatically UploadSymbols

if /i "%SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY%" NEQ "" (
  set UploadSymbols=%SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY%
  echo Sentry: Automatic symbols upload settings were overridden via environment variable SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY with value '%SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY%'
)

if /i "%UploadSymbols%" NEQ "True" (
    echo Sentry: Automatic symbols upload is disabled. Skipping...
    exit /B 0
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings IncludeSources IncludeSourceFiles

set CliArgs=

if /i "%IncludeSourceFiles%"=="True" (
    set "CliArgs=--include-sources"
)

set CliLogLevel=

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings DiagnosticLevel CliLogLevel
if "%CliLogLevel%"=="" (
    set "CliLogLevel=info"
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings EnableBuildPlatforms EnabledPlatforms
if not "%EnabledPlatforms%"=="" (
  set PlatformToCheck=
  if "%TargetPlatform%"=="Win64" (
    set "PlatformToCheck=bEnableWindows=False"
  ) else (
    set "PlatformToCheck=bEnable%TargetPlatform%=False"
  )
  call :FindString EnabledPlatforms PlatformToCheck IsPlatformDisabled
  if "!IsPlatformDisabled!"=="true" (
      echo "Sentry: Automatic symbols upload is disabled for build platform %TargetPlatform%. Skipping..."
      exit /B 0
  )
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings EnableBuildTargets EnabledTargets
if not "%EnabledTargets%"=="" (
  set TargetToCheck="bEnable%TargetType%=False"
  call :FindString EnabledTargets TargetToCheck IsTargetDisabled
  if "!IsTargetDisabled!"=="true" (
      echo "Sentry: Automatic symbols upload is disabled for build target type %TargetType%. Skipping..."
      exit /B 0
  )
)

call :ParseIniFile "%ConfigPath%\DefaultEngine.ini" /Script/Sentry.SentrySettings EnableBuildConfigurations EnabledConfigurations
if not "%EnabledConfigurations%"=="" (
  set ConfigToCheck="bEnable%TargetConfig%=False"
  call :FindString EnabledConfigurations ConfigToCheck IsConfigDisabled
  if "!IsConfigDisabled!"=="true" (
      echo "Sentry: Automatic symbols upload is disabled for build configuration %TargetConfig%. Skipping..."
      exit /B 0
  )
)

set PropertiesFile=%ProjectPath:"=%\sentry.properties

if not exist "%PropertiesFile%" (
    echo Warning: Sentry: Properties file is missing: '%PropertiesFile%'
    exit /B 0
)

if not exist "%CliExec%" (
    echo Warning: Sentry: Sentry CLI is not configured in plugin settings. Skipping...
    exit /B 0
)

echo Sentry: Upload started using PropertiesFile '%PropertiesFile%'

set "SENTRY_PROPERTIES=%PropertiesFile%"
echo %ProjectBinariesPath%
echo %PluginBinariesPath%
call "%CliExec%" upload-dif %CliArgs% --log-level %CliLogLevel% "%ProjectBinariesPath%" "%PluginBinariesPath%"

echo Sentry: Upload finished

endlocal
exit /B 0

:FindString <sourceStr> <findStr> <result>
  setlocal
  for /f "delims=" %%A in ('echo %%%1%%') do set str1=%%A
  for /f "delims=" %%A in ('echo %%%2%%') do set str2=%%A
  echo.%str1%|findstr /C:"%str2%" >nul 2>&1
  endlocal
  if not errorlevel 1 (
    set %~3=true
  ) else (
    set %~3=false
  )
  goto :eof

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
          if defined insection (
            set insection=
            goto :eof
          )
        )
      )
    )
  )
  endlocal
  set insection=
  goto :eof