@echo off
setlocal Enabledelayedexpansion

:: Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Requesting administrator privileges...
    powershell -Command "Start-Process -FilePath '%0' -Verb RunAs"
    exit /b
)

:menu
cls
echo.
echo ^> DeArrow CLI
echo ^>^> Made by falconwasplaying
echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo.
echo ^> Type 'remove' to hide shortcut arrows, or 'restore' to show them.
echo.
set /p choice="> Choice: "

if /i "!choice!"=="remove" goto remove
if /i "!choice!"=="restore" goto restore
echo.
echo Invalid Input!
timeout /t 2 >nul
goto menu

:remove
reg add "HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Icons" /v "29" /t REG_SZ /d "shell32.dll,50" /f >nul 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Failed to write registry value.
    goto end
)
goto success

:restore
reg delete "HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Icons" /v "29" /f >nul 2>&1
goto success

:success
:restart_prompt
echo.
echo Registry updated successfully!
echo.
set /p restart="> Would you like to restart Windows Explorer now to apply changes? (y/n): "
if /i "!restart!"=="y" (
    taskkill /f /im explorer.exe >nul 2>&1
    start explorer.exe
    exit /b
)
if /i "!restart!"=="n" (
    echo.
    echo ^> Changes will apply on next boot or when windows explorer is restarted
    echo.
    echo ^> Press any key to close...
    pause >nul
    exit /b
)
echo.
echo Invalid choice. Please type 'y' or 'n'.
goto restart_prompt

:end
echo.
echo Press any key to close...
pause >nul
exit /b