@echo off

setlocal enabledelayedexpansion

set "TARGET_PLATFORM=%~1"
set "TARGET_NAME=%~2"
set "TARGET_TYPE=%~3"
set "TARGET_CONFIGURATION=%~4"
set "PROJECT_FILE=%~5"
set "PLUGIN_DIR=%~6"
set "ENGINE_DIR=%~7"

set "LINUX_BIN_DIR=%PLUGIN_DIR%\Binaries\Linux"
set "LINUX_SOURCE_DIR=%PLUGIN_DIR%\Source\ThirdParty\Linux\bin"

REM Copy crashpad handler to plugin's Binaries\Linux for cross-compilation on Windows
if "%TARGET_PLATFORM%"=="Linux" (
  if not exist "%LINUX_BIN_DIR%" (mkdir "%LINUX_BIN_DIR%")
  if not exist "%LINUX_BIN_DIR%\crashpad_handler" (
    copy /Y "%LINUX_SOURCE_DIR%\crashpad_handler" "%LINUX_BIN_DIR%\"
  )
)

set "WIN64_BIN_DIR=%PLUGIN_DIR%\Binaries\Win64"
set "WIN64_SOURCE_DIR=%PLUGIN_DIR%\Source\ThirdParty\Win64\bin"

REM Copy crashpad binaries to plugin's Binaries\Win64 (FAB compatibility)
if "%TARGET_PLATFORM%"=="Win64" (
  if not exist "%WIN64_BIN_DIR%" (mkdir "%WIN64_BIN_DIR%")
  if not exist "%WIN64_BIN_DIR%\crashpad_handler.exe" (
    copy /Y "%WIN64_SOURCE_DIR%\crashpad_handler.exe" "%WIN64_BIN_DIR%\"
  )
  if not exist "%WIN64_BIN_DIR%\crashpad_wer.dll" (
    copy /Y "%WIN64_SOURCE_DIR%\crashpad_wer.dll" "%WIN64_BIN_DIR%\"
  )
)

REM Call Python script for debug symbol upload
"%ENGINE_DIR%\Binaries\ThirdParty\Python3\Win64\python.exe" "%PLUGIN_DIR%\Scripts\upload-debug-symbols.py" ^
  %TARGET_PLATFORM% %TARGET_NAME% %TARGET_TYPE% %TARGET_CONFIGURATION% "%PROJECT_FILE%" "%PLUGIN_DIR%"

endlocal
