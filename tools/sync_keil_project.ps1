param(
    [Parameter(Mandatory = $true)] [string] $ProjectFile,
    [Parameter(Mandatory = $true)] [string] $ProjectRoot,
    # C51 的 DATA 空间很小。默认只同步头文件搜索路径，保留用户在 Keil
    # 中选择的当前练习 .c 文件；显式传入本选项才把 src 下所有 .c 加入工程。
    [switch] $SyncAllSources
)

# 在不重新序列化 Keil XML 的前提下同步：
# 1. src/ 下包含头文件的目录；
# 2. src/ 下所有 .c 文件。
# 保留原始 .uvproj 格式，避免旧版 UV4 返回错误 15。

$ErrorActionPreference = "Stop"
$projectText = Get-Content -LiteralPath $ProjectFile -Raw
$singleLine = [System.Text.RegularExpressions.RegexOptions]::Singleline
$ignoreCase = [System.Text.RegularExpressions.RegexOptions]::IgnoreCase

$srcDirectory = Join-Path $ProjectRoot "src"
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
    $projectText = [regex]::Replace($projectText, $srcFilePattern, '', $singleLine -bor $ignoreCase)

    $sourceEntries = ""
    if (Test-Path -LiteralPath $srcDirectory) {
        Get-ChildItem -LiteralPath $srcDirectory -File -Recurse -Filter *.c -Name | ForEach-Object {
            $relative = ('..\src\' + ([string]$_).Replace('/', '\'))
            $fileName = Split-Path -Leaf ([string]$_)
            $sourceEntries += "            <File>`r`n"
            $sourceEntries += "              <FileName>$fileName</FileName>`r`n"
            $sourceEntries += "              <FileType>1</FileType>`r`n"
            $sourceEntries += "              <FilePath>$relative</FilePath>`r`n"
            $sourceEntries += "            </File>`r`n"
            Write-Host "Added source: $relative"
        }
    }

    # 只把 src 文件加入 Source Group 1，不能对所有 </Files> 节点做全局替换，
    # 否则会把同一批源文件重复插入 drivers 等其他文件组。
    $sourceGroupPattern = '(<GroupName>Source Group 1</GroupName>\s*<Files>.*?)(</Files>)'
    $sourceGroupMatch = [regex]::Match($projectText, $sourceGroupPattern, $singleLine -bor $ignoreCase)
    if (-not $sourceGroupMatch.Success) {
        throw "Source Group 1 Files node was not found in $ProjectFile"
    }

    $sourceGroupContent = $sourceGroupMatch.Groups[1].Value + $sourceEntries + $sourceGroupMatch.Groups[2].Value
    $projectText = $projectText.Substring(0, $sourceGroupMatch.Index) +
                   $sourceGroupContent +
                   $projectText.Substring($sourceGroupMatch.Index + $sourceGroupMatch.Length)
} else {
    Write-Host "Keil source list preserved. Add only the .c files required by the current test."
}
Set-Content -LiteralPath $ProjectFile -Value $projectText -Encoding UTF8
Write-Host "Keil project synchronized: $ProjectFile"
