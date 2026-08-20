param(
    [string]$Message = "Auto update $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
)

$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw 'Git is not installed or is not available in PATH.'
}

git add --all
git diff --cached --quiet

if ($LASTEXITCODE -eq 0) {
    Write-Host 'No changes to upload.'
    exit 0
}

git commit -m $Message
if ($LASTEXITCODE -ne 0) { throw 'Git commit failed.' }

$branch = git branch --show-current
git push -u origin $branch
if ($LASTEXITCODE -ne 0) { throw 'GitHub upload failed.' }

Write-Host "Uploaded branch '$branch' to GitHub successfully."
