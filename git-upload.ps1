param(
    [string]$Message = "Auto update $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
)

$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot

function Invoke-Git {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Arguments)

    & git -c "safe.directory=$PSScriptRoot" @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Git command failed: git $($Arguments -join ' ')"
    }
}

try {
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw 'Git was not found. Please install Git for Windows first.'
    }

    if (-not (Test-Path -LiteralPath '.git')) {
        throw "The script directory is not a Git repository: $PSScriptRoot"
    }

    $branch = (& git -c "safe.directory=$PSScriptRoot" branch --show-current).Trim()
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($branch)) {
        throw 'Unable to determine the current Git branch.'
    }

    Write-Host "[1/4] Checking project changes..." -ForegroundColor Cyan
    $changes = & git -c "safe.directory=$PSScriptRoot" status --porcelain
    if ($LASTEXITCODE -ne 0) { throw 'Unable to read Git status.' }

    if ($changes) {
        Write-Host "[2/4] Creating a local commit..." -ForegroundColor Cyan
        Invoke-Git add --all
        Invoke-Git commit -m $Message
    }
    else {
        Write-Host '[2/4] No new local changes.' -ForegroundColor DarkGray
    }

    Write-Host "[3/4] Syncing the latest changes from GitHub..." -ForegroundColor Cyan
    Invoke-Git pull --rebase origin $branch

    Write-Host "[4/4] Uploading to GitHub..." -ForegroundColor Cyan
    Invoke-Git push -u origin $branch

    Write-Host ''
    Write-Host 'Upload complete. The local project and GitHub are synchronized.' -ForegroundColor Green
    exit 0
}
catch {
    Write-Host ''
    Write-Host "Upload failed: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host 'Your local files and any created commits are safe.' -ForegroundColor Yellow
    exit 1
}
