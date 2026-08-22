param(
    [string]$ConfigPath = 'release-config.json',
    [string]$OutputDirectory = 'release-output',
    [string]$Repository = $env:GITHUB_REPOSITORY
)

$ErrorActionPreference = 'Stop'
$config = Get-Content -LiteralPath $ConfigPath -Raw -Encoding UTF8 | ConvertFrom-Json

$requiredProperties = @('version', 'title', 'draft', 'prerelease', 'summary', 'changes')
foreach ($property in $requiredProperties) {
    if ($null -eq $config.$property) { throw "Missing required release property: $property" }
}
foreach ($property in @('title', 'summary')) {
    foreach ($language in @('en', 'cn')) {
        if ([string]::IsNullOrWhiteSpace($config.$property.$language)) {
            throw "Missing release property: $property.$language"
        }
    }
}
foreach ($section in @('added', 'changed', 'fixed', 'knownIssues')) {
    if ($null -eq $config.changes.$section) { throw "Missing release section: changes.$section" }
    foreach ($language in @('en', 'cn')) {
        if ($null -eq $config.changes.$section.$language) {
            throw "Missing release property: changes.$section.$language"
        }
    }
}
if ($config.version -notmatch '^\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?$') {
    throw "Invalid semantic version: '$($config.version)'"
}
if ([string]::IsNullOrWhiteSpace($Repository)) {
    $Repository = 'sheeepsoup/Environment-Smulation'
}

$tag = "v$($config.version)"
$assetBase = "https://github.com/$Repository/releases/download/$tag"
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$sections = @(
    @{ Key = 'added'; En = 'Added'; Cn = '新增' },
    @{ Key = 'changed'; En = 'Changed'; Cn = '变更' },
    @{ Key = 'fixed'; En = 'Fixed'; Cn = '修复' },
    @{ Key = 'knownIssues'; En = 'Known issues'; Cn = '已知问题' }
)

function New-Notes {
    param([ValidateSet('en', 'cn')][string]$Language)

    $nav = "[English]($assetBase/RELEASE_NOTES.en.md) | [中文]($assetBase/RELEASE_NOTES.cn.md)"
    $title = if ($Language -eq 'en') { $config.title.en } else { $config.title.cn }
    $summary = if ($Language -eq 'en') { $config.summary.en } else { $config.summary.cn }
    $lines = [System.Collections.Generic.List[string]]::new()
    $lines.Add($nav)
    $lines.Add('')
    $lines.Add("# $title - $tag")
    $lines.Add('')
    $lines.Add($summary)

    foreach ($section in $sections) {
        $items = $config.changes.($section.Key).$Language
        if ($null -eq $items -or $items.Count -eq 0) { continue }
        $heading = if ($Language -eq 'en') { $section.En } else { $section.Cn }
        $lines.Add('')
        $lines.Add("## $heading")
        $lines.Add('')
        foreach ($item in $items) { $lines.Add("- $item") }
    }

    return ($lines -join "`n") + "`n"
}

$enPath = Join-Path $OutputDirectory 'RELEASE_NOTES.en.md'
$cnPath = Join-Path $OutputDirectory 'RELEASE_NOTES.cn.md'
[IO.File]::WriteAllText($enPath, (New-Notes -Language en), [Text.UTF8Encoding]::new($false))
[IO.File]::WriteAllText($cnPath, (New-Notes -Language cn), [Text.UTF8Encoding]::new($false))

$releaseBody = @"
[English]($assetBase/RELEASE_NOTES.en.md) | [中文]($assetBase/RELEASE_NOTES.cn.md)

# $($config.title.en) / $($config.title.cn) - $tag

$($config.summary.en)

$($config.summary.cn)
"@
[IO.File]::WriteAllText((Join-Path $OutputDirectory 'RELEASE_BODY.md'), $releaseBody, [Text.UTF8Encoding]::new($false))

if ($env:GITHUB_OUTPUT) {
    "version=$($config.version)" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    "tag=$tag" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    "name=$($config.title.en) / $($config.title.cn)" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    "draft=$($config.draft.ToString().ToLowerInvariant())" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    "prerelease=$($config.prerelease.ToString().ToLowerInvariant())" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
}

Write-Host "Prepared bilingual release notes for $tag."
