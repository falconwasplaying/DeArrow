@echo off
echo Compiling CLI resources...
C:\falcon\Programs\MSYS2\ucrt64\bin\windres.exe resources.rc -O coff -o resources.o
if %errorlevel% neq 0 (
    echo Resource compilation failed!
    pause
    exit /b %errorlevel%
)

echo Compiling DeArrowCLI.exe...
C:\falcon\Programs\MSYS2\ucrt64\bin\g++.exe main.cpp resources.o -o DeArrowCLI.exe -mconsole -ladvapi32 -lshell32 -nostartfiles -e WinMainCRTStartup -DCLI_MODE -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections "-Wl,--gc-sections"
if %errorlevel% neq 0 (
    echo.
    echo Compilation failed!
    del resources.o >nul 2>&1
    pause
    exit /b %errorlevel%
)

del resources.o >nul 2>&1

for %%A in (DeArrowCLI.exe) do set "size=%%~zA"
echo.
echo Compilation successful! Generated standalone DeArrowCLI.exe (%size% bytes).
pause
