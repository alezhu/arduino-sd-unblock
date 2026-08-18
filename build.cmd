@echo off
setlocal enabledelayedexpansion

if not exist version.txt (
    echo Error: version.txt not found!
    exit /b 1
)

set /p VERSION=<version.txt
set VERSION=%VERSION: =%

set TARGET=%~1
if "%TARGET%"=="" set TARGET=all

echo ===================================================
echo Building SD Card Write Protection Tool (sd-unblock) v%VERSION%
echo Target platform: %TARGET%
echo ===================================================

if not exist build_out mkdir build_out

set PIO_CMD=pio
where pio >nul 2>nul
if %errorlevel% neq 0 (
    if exist "%USERPROFILE%\.platformio\penv\Scripts\pio.exe" (
        set PIO_CMD="%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
    )
)

if /i "%TARGET%"=="uno" goto build_uno_standalone
if /i "%TARGET%"=="esp8266" goto build_esp8266_standalone
if /i "%TARGET%"=="esp32" goto build_esp32_standalone
if /i "%TARGET%"=="bluepill" goto build_bluepill_standalone
if /i "%TARGET%"=="lgt8f328p" goto build_lgt8f328p_standalone
if /i "%TARGET%"=="all" goto build_all

echo Error: Unknown platform '%TARGET%'!
echo Supported platforms: uno, esp8266, esp32, bluepill, lgt8f328p, all
exit /b 1

:build_all
call :do_uno
if %errorlevel% neq 0 exit /b 1
call :do_esp8266
if %errorlevel% neq 0 exit /b 1
call :do_esp32
if %errorlevel% neq 0 exit /b 1
call :do_bluepill
if %errorlevel% neq 0 exit /b 1
call :do_lgt8f328p
if %errorlevel% neq 0 exit /b 1
goto finish

:build_uno_standalone
call :do_uno
goto finish

:build_esp8266_standalone
call :do_esp8266
goto finish

:build_esp32_standalone
call :do_esp32
goto finish

:build_bluepill_standalone
call :do_bluepill
goto finish

:build_lgt8f328p_standalone
call :do_lgt8f328p
goto finish

:do_uno
echo.
echo Building for Arduino (Uno)...
%PIO_CMD% run --environment uno
if %errorlevel% neq 0 ( echo Error: Build failed for uno & exit /b 1 )
copy /y .pio\build\uno\firmware.hex build_out\firmware_arduino_v%VERSION%.hex >nul
exit /b 0

:do_esp8266
echo.
echo Building for ESP8266...
%PIO_CMD% run --environment esp8266
if %errorlevel% neq 0 ( echo Error: Build failed for esp8266 & exit /b 1 )
copy /y .pio\build\esp8266\firmware.bin build_out\firmware_esp8266_v%VERSION%.bin >nul
exit /b 0

:do_esp32
echo.
echo Building for ESP32...
%PIO_CMD% run --environment esp32
if %errorlevel% neq 0 ( echo Error: Build failed for esp32 & exit /b 1 )
copy /y .pio\build\esp32\firmware.bin build_out\firmware_esp32_v%VERSION%.bin >nul
exit /b 0

:do_bluepill
echo.
echo Building for STM32 (Blue Pill)...
%PIO_CMD% run --environment bluepill
if %errorlevel% neq 0 ( echo Error: Build failed for bluepill & exit /b 1 )
if exist .pio\build\bluepill\firmware.bin (
    copy /y .pio\build\bluepill\firmware.bin build_out\firmware_stm32_v%VERSION%.bin >nul
) else (
    copy /y .pio\build\bluepill\firmware.hex build_out\firmware_stm32_v%VERSION%.hex >nul
)
exit /b 0

:do_lgt8f328p
echo.
echo Building for LGT8F328P...
%PIO_CMD% run --environment lgt8f328p
if %errorlevel% neq 0 ( echo Error: Build failed for lgt8f328p & exit /b 1 )
copy /y .pio\build\lgt8f328p\firmware.hex build_out\firmware_lgt8f328p_v%VERSION%.hex >nul
exit /b 0

:finish
echo.
echo ===================================================
echo Build completed successfully! Output files in build_out\:
dir build_out\firmware_*
echo ===================================================
