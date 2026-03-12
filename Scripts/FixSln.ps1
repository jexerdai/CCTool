param(
    [string]$ProjectFiles,
    [string]$Root
)

$src = Join-Path $ProjectFiles "CCTool.sln"
$dst = Join-Path $Root "CCTool.sln"

if (-not (Test-Path $src)) {
    Write-Host "[CCTool] ERROR: $src not found."
    exit 1
}

$content = Get-Content $src -Raw
$content = $content -replace '(?<=[" =])(\w+\.vcxproj)', "Outputs\ProjectFiles\`$1"
Set-Content -Path $dst -Value $content -NoNewline -Encoding UTF8
Write-Host "[CCTool] CCTool.sln written to project root."
