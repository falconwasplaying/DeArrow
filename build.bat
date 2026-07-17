@echo off
echo Compiling resources...
C:\falcon\Programs\MSYS2\ucrt64\bin\windres.exe resources.rc -O coff -o resources.o
if %errorlevel% neq 0 (
    echo Resource compilation failed!
    pause
    exit /b %errorlevel%
)

echo Compiling DeArrow.exe...
C:\falcon\Programs\MSYS2\ucrt64\bin\g++.exe main.cpp resources.o -o DeArrow.exe -mwindows -lgdi32 -ladvapi32 -lshell32 -nostartfiles -e WinMainCRTStartup -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections "-Wl,--gc-sections"
if %errorlevel% neq 0 (
    echo.
    echo Compilation failed!
    del resources.o >nul 2>&1
    pause
    exit /b %errorlevel%
)

del resources.o >nul 2>&1

for %%A in (DeArrow.exe) do set "size=%%~zA"
echo.
echo Compilation successful! Generated standalone DeArrow.exe (%size% bytes).
pause
