@echo off
setlocal enabledelayedexpansion

set "TARGET_PLATFORM=%~1"
set "TARGET_NAME=%~2"
set "TARGET_TYPE=%~3"
set "TARGET_CONFIGURATION=%~4"
set "PROJECT_FILE=%~5"
set "PLUGIN_DIR=%~6"
set "ENGINE_DIR=%~7"

:: Copy crashpad handler executable to plugin's Binaries\Linux dir if it doesn't exist there to enable cross-compilation for Linux on Windows with FAB version of the plugin
set "CRASHPAD_HANDLER_LINUX=%PLUGIN_DIR%\Binaries\Linux\crashpad_handler"
if "%TARGET_PLATFORM%"=="Linux" (
  if not exist "%CRASHPAD_HANDLER_LINUX%" (xcopy "%PLUGIN_DIR%\Source\ThirdParty\Linux\bin\*" "%PLUGIN_DIR%\Binaries\Linux\" /F /R /Y /I)
)

:: Copy crashpad handler executable to plugin's Binaries\Win64 dir if it doesn't exist there to ensure FAB version of the plugin works correctly - Epic obfuscates any extra binaries when pre-building the plugin
set "CRASHPAD_HANDLER_WIN=%PLUGIN_DIR%\Binaries\Win64\crashpad_handler.exe"
if "%TARGET_PLATFORM%"=="Win64" (
  if not exist "%CRASHPAD_HANDLER_WIN%" (xcopy "%PLUGIN_DIR%\Source\ThirdParty\Win64\Crashpad\bin\*" "%PLUGIN_DIR%\Binaries\Win64\" /F /R /Y /I)
)

for /f "tokens=2 delims=:, " %%i in ('type "%PROJECT_FILE%" ^| findstr /C:"EngineAssociation"') do set "ENGINE_VERSION=%%~i"
set "ENGINE_VERSION=!ENGINE_VERSION:~0,1!"

if "!ENGINE_VERSION!"=="4" (set "EDITOR_EXE=%ENGINE_DIR%\Binaries\Win64\UE4Editor-Cmd.exe") else (set "EDITOR_EXE=%ENGINE_DIR%\Binaries\Win64\UnrealEditor-Cmd.exe")

"!EDITOR_EXE!" "%PROJECT_FILE%" -run=SentrySymbolUpload -target-platform=%TARGET_PLATFORM% -target-name=%TARGET_NAME% -target-type=%TARGET_TYPE% -target-configuration=%TARGET_CONFIGURATION% -unattended -nopause -nullrhi

endlocal