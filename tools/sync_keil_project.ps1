param(
    [Parameter(Mandatory = $true)] [string] $ProjectFile,
    [Parameter(Mandatory = $true)] [string] $ProjectRoot,
    # C51 的 DATA 空间很小。默认只同步头文件搜索路径，保留用户在 Keil
    # 中选择的当前练习 .c 文件；显式传入本选项才把 src 下所有 .c 加入工程。
    [switch] $SyncAllSources
)

# 在不重新序列化 Keil XML 的前提下同步：
# 1. src/ 下包含头文件的目录；
# 2. src/ 下除 Labs 外的普通模块 .c 文件。
# 保留原始 .uvproj 格式，避免旧版 UV4 返回错误 15。

$ErrorActionPreference = "Stop"
$projectText = Get-Content -LiteralPath $ProjectFile -Raw
$singleLine = [System.Text.RegularExpressions.RegexOptions]::Singleline
$ignoreCase = [System.Text.RegularExpressions.RegexOptions]::IgnoreCase

$srcDirectory = Join-Path $ProjectRoot "src"
$labsDirectory = Join-Path $srcDirectory "Labs"
$includeDirectories = @(
    "..\device",
    "..\drivers",
    "..\src"
)

if (Test-Path -LiteralPath $srcDirectory) {
    Get-ChildItem -LiteralPath $srcDirectory -Directory -Recurse -Name | ForEach-Object {
        $relativeDirectory = ([string]$_).Replace('/', '\')
        $absoluteDirectory = Join-Path $srcDirectory $relativeDirectory
        if (Get-ChildItem -LiteralPath $absoluteDirectory -File -Filter *.h -ErrorAction SilentlyContinue) {
            $includeDirectories += ('..\src\' + $relativeDirectory)
        }
    }
}

$includePath = ($includeDirectories | Select-Object -Unique) -join ';'
$projectText = [regex]::Replace($projectText, '<IncludePath>.*?</IncludePath>', "<IncludePath>$includePath</IncludePath>", $singleLine)

if ($SyncAllSources) {
    # 删除旧版本同步器产生的 src/ 或 UNC 绝对路径文件条目。
    # 注意：必须使用 \s* 和单行模式兼容不同换行格式（LF/CRLF）。
    $srcFilePattern = '<File>.*?<FilePath>[^<]*(?:\.\.\\src\\|Mac|Home)[^<]*</FilePath>.*?</File>'

    $sourceEntries = ""
    if (Test-Path -LiteralPath $srcDirectory) {
        Get-ChildItem -LiteralPath $srcDirectory -File -Recurse -Filter *.c |
            Where-Object { $_.FullName -notlike "$labsDirectory\*" } |
            Sort-Object FullName |
            ForEach-Object {
            $relativePath = $_.FullName.Substring($srcDirectory.Length).TrimStart('\', '/')
            $relative = ('..\src\' + $relativePath.Replace('/', '\'))
            $fileName = $_.Name
            $sourceEntries += "            <File>`r`n"
            $sourceEntries += "              <FileName>$fileName</FileName>`r`n"
            $sourceEntries += "              <FileType>1</FileType>`r`n"
            $sourceEntries += "              <FilePath>$relative</FilePath>`r`n"
            $sourceEntries += "            </File>`r`n"
            Write-Host "Added source: $relative"
        }
    }

    # 仅替换 BSP 内的普通 src 模块；Labs 由 sync_labs_to_keil.sh 维护，
    # 不能在全工程范围内删除，否则会清空 Labs 分组。
    $bspGroupPattern = '(<Group>\s*<GroupName>BSP</GroupName>\s*<Files>)(.*?)(</Files>\s*</Group>)'
    $bspGroupMatch = [regex]::Match($projectText, $bspGroupPattern, $singleLine -bor $ignoreCase)
    if (-not $bspGroupMatch.Success) {
        throw "BSP Files node was not found in $ProjectFile"
    }

    $bspFiles = [regex]::Replace($bspGroupMatch.Groups[2].Value, $srcFilePattern, '', $singleLine -bor $ignoreCase)
    $bspGroupContent = $bspGroupMatch.Groups[1].Value + $bspFiles + $sourceEntries + $bspGroupMatch.Groups[3].Value
    $projectText = $projectText.Substring(0, $bspGroupMatch.Index) +
                   $bspGroupContent +
                   $projectText.Substring($bspGroupMatch.Index + $bspGroupMatch.Length)
} else {
    Write-Host "Keil source list preserved. Add only the .c files required by the current test."
}
Set-Content -LiteralPath $ProjectFile -Value $projectText -Encoding UTF8
Write-Host "Keil project synchronized: $ProjectFile"
