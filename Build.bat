@echo off
setlocal

:: ── 配置区 ────────────────────────────────────────────────────────────────
set QT6_PATH=C:\Qt\6.10.2\msvc2022_64
:: ─────────────────────────────────────────────────────────────────────────

set ROOT=%~dp0
set PROJECT_FILES=%ROOT%Outputs\ProjectFiles

echo [CCTool] Configuring...
cmake -S "%ROOT%" -B "%PROJECT_FILES%" ^
    -DCMAKE_PREFIX_PATH="%QT6_PATH%"
if errorlevel 1 (
    echo [CCTool] CMake configuration FAILED.
    pause & exit /b 1
)

:: 将 .sln 复制到项目根目录
if exist "%PROJECT_FILES%\CCTool.sln" (
    copy /Y "%PROJECT_FILES%\CCTool.sln" "%ROOT%CCTool.sln" >nul
    echo [CCTool] CCTool.sln copied to project root.
)

echo [CCTool] Building...
cmake --build "%PROJECT_FILES%" --config Release
if errorlevel 1 (
    echo [CCTool] Build FAILED.
    pause & exit /b 1
)

echo.
echo [CCTool] Build succeeded.
echo   .sln : %ROOT%CCTool.sln
echo   .exe : %ROOT%CCTool.exe
echo.
pause
