@echo off
REM Copyright (c) 2025 Sentry. All Rights Reserved.

setlocal enabledelayedexpansion

set "TARGET_PLATFORM=%~1"
set "TARGET_NAME=%~2"
set "TARGET_TYPE=%~3"
set "TARGET_CONFIGURATION=%~4"
set "PROJECT_FILE=%~5"
set "PLUGIN_DIR=%~6"
set "ENGINE_DIR=%~7"

REM Copy crashpad handler executable to plugin's Binaries\Linux dir if it doesn't exist there
REM This enables cross-compilation for Linux on Windows with FAB version of the plugin
set "CRASHPAD_HANDLER_LINUX=%PLUGIN_DIR%\Binaries\Linux\crashpad_handler"
if "%TARGET_PLATFORM%"=="Linux" (
  if not exist "%CRASHPAD_HANDLER_LINUX%" (
    if not exist "%PLUGIN_DIR%\Binaries\Linux\" (mkdir "%PLUGIN_DIR%\Binaries\Linux")
    xcopy "%PLUGIN_DIR%\Source\ThirdParty\Linux\bin\*" "%PLUGIN_DIR%\Binaries\Linux\" /F /R /Y /I
  )
)

REM Copy crashpad handler executable to plugin's Binaries\Win64 dir if it doesn't exist there
REM Epic obfuscates any extra binaries when pre-building the plugin for FAB
set "CRASHPAD_HANDLER_WIN=%PLUGIN_DIR%\Binaries\Win64\crashpad_handler.exe"
if "%TARGET_PLATFORM%"=="Win64" (
  if not exist "%CRASHPAD_HANDLER_WIN%" (
    if not exist "%PLUGIN_DIR%\Binaries\Win64\" (mkdir "%PLUGIN_DIR%\Binaries\Win64")
    xcopy "%PLUGIN_DIR%\Source\ThirdParty\Win64\bin\*" "%PLUGIN_DIR%\Binaries\Win64\" /F /R /Y /I
  )
)

REM Call Python script for debug symbol upload
"%ENGINE_DIR%\Binaries\ThirdParty\Python3\Win64\python.exe" "%PLUGIN_DIR%\Scripts\upload-debug-symbols.py" ^
  %TARGET_PLATFORM% %TARGET_NAME% %TARGET_TYPE% %TARGET_CONFIGURATION% "%PROJECT_FILE%" "%PLUGIN_DIR%"

endlocal
