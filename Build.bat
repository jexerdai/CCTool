@echo off
setlocal

:: --- Config (change Qt6 path to match your installation) ---
set QT6_PATH=C:\Qt\6.10.2\msvc2022_64
:: -----------------------------------------------------------

set ROOT=%~dp0
set PROJECT_FILES=%ROOT%Outputs\ProjectFiles
set BINARIES=%ROOT%Binaries

echo [CCTool] Configuring...
cmake -S "%ROOT:~0,-1%" -B "%PROJECT_FILES%" -DCMAKE_PREFIX_PATH="%QT6_PATH%"
if errorlevel 1 (
    echo [CCTool] CMake configuration FAILED.
    pause & exit /b 1
)

echo [CCTool] Generating CCTool.sln to project root...
powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%Scripts\FixSln.ps1" -ProjectFiles "%PROJECT_FILES%" -Root "%ROOT:~0,-1%"

echo [CCTool] Building...
cmake --build "%PROJECT_FILES%" --config Release
if errorlevel 1 (
    echo [CCTool] Build FAILED.
    pause & exit /b 1
)

echo [CCTool] Deploying Qt dependencies...
"%QT6_PATH%\bin\windeployqt.exe" --release "%BINARIES%\CCTool.exe"
if errorlevel 1 (
    echo [CCTool] windeployqt FAILED.
    pause & exit /b 1
)

echo.
echo [CCTool] Build succeeded.
echo   .sln : %ROOT%CCTool.sln
echo   .exe : %BINARIES%\CCTool.exe
echo.
if not defined NOPAUSE pause
