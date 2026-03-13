$Root = Split-Path $PSScriptRoot -Parent

# Kill existing CCTool.exe
$proc = Get-Process -Name "CCTool" -ErrorAction SilentlyContinue
if ($proc) {
    Write-Host "Killing CCTool.exe..."
    $proc | Stop-Process -Force
    Start-Sleep -Seconds 1
}

# Build
Write-Host ""
Write-Host "Running Build.bat..."
$env:NOPAUSE = "1"
$build = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$Root\Build.bat`"" -WorkingDirectory $Root -Wait -PassThru -NoNewWindow
$env:NOPAUSE = $null
if ($build.ExitCode -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] Build failed (exit code $($build.ExitCode)). Aborting." -ForegroundColor Red
    exit $build.ExitCode
}

# Launch
Write-Host ""
Write-Host "Launching CCTool.exe..."
Start-Process -FilePath "$Root\Binaries\CCTool.exe"
