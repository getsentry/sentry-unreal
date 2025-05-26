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

if "%TargetPlatform%"=="Android" (
	echo Sentry: Debug symbols upload for Android is handled by Sentry's Gradle plugin if enabled
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

set CliExec=%PluginPath:"=%\Source\ThirdParty\CLI\sentry-cli-Windows-x86_64.exe

if not exist "%CliExec%" (
    echo Error: Sentry: Sentry CLI is missing. Skipping...
    exit /B 0
)

set PropertiesFile=%ProjectPath:"=%\sentry.properties

echo Sentry: Looking for properties file '%PropertiesFile%'

if exist "%PropertiesFile%" (
    echo Sentry: Properties file found. Starting upload...
    call :ParseIniFile "%PropertiesFile%" Sentry defaults.project ProjectName
    if "!ProjectName!"=="" (
        echo Error: Project name is not set. Skipping...
        exit /B 0
    )
    call :ParseIniFile "%PropertiesFile%" Sentry defaults.org OrgName
    if "!OrgName!"=="" ( 
        echo Error: Project organization is not set. Skipping...
        exit /B 0
    )
    call :ParseIniFile "%PropertiesFile%" Sentry auth.token AuthToken
    if "!AuthToken!"=="" (
        echo Error: Auth token is not set. Skipping...
        exit /B 0
    )
    set "SENTRY_PROPERTIES=%PropertiesFile%"
) else (
    echo Sentry: Properties file not found. Falling back to environment variables.
    if /i "%SENTRY_PROJECT%"=="" (
        echo Error: SENTRY_PROJECT env var is not set. Skipping...
        exit /B 0
    )
    if /i "%SENTRY_ORG%"=="" (
        echo Error: SENTRY_ORG env var is not set. Skipping...
        exit /B 0
    )
    if /i "%SENTRY_AUTH_TOKEN%"=="" (
        echo Error: SENTRY_AUTH_TOKEN env var is not set. Skipping...
        exit /B 0
    )
)

call "%CliExec%" debug-files upload %CliArgs% --log-level %CliLogLevel% "%ProjectBinariesPath%" "%PluginBinariesPath%"

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

:FindInFile <filePath> <findStr> <result>
  setlocal
  for /f "tokens=*" %%A in ('findstr /i /r "%~2" "%~1"') do (
    endlocal
    set %~3=true
    goto :eof
  )
  endlocal
  set %~3=false
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
