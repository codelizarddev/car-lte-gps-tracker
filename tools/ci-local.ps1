param(
    [string]$ToolchainBin = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
)

$ErrorActionPreference = "Stop"

function Require-Tool($path, $name) {
    if (-not (Test-Path $path)) {
        throw "$name not found at $path"
    }
}

function Invoke-Step($name, [scriptblock]$block) {
    Write-Host ""
    Write-Host "== $name =="
    & $block
}

function Test-CMakeRequires {
    $componentRoot = Join-Path $PSScriptRoot "..\firmware\components"
    $firmwareCmake = Get-Content (Join-Path $PSScriptRoot "..\firmware\CMakeLists.txt") -Raw
    $explicitComponentDirs = [regex]::Matches($firmwareCmake, 'components/([A-Za-z0-9_]+)') | ForEach-Object { $_.Groups[1].Value }
    $localComponents = @($explicitComponentDirs)

    $knownIdfComponents = @(
        "driver", "freertos", "log", "mqtt", "nvs_flash", "esp_adc"
    )

    $allowed = @{}
    foreach ($name in ($localComponents + $knownIdfComponents)) {
        $allowed[$name] = $true
    }

    $cmakeFiles = foreach ($component in $localComponents) {
        Get-Item (Join-Path $componentRoot "$component\CMakeLists.txt")
    }
    $cmakeFiles += Get-Item (Join-Path $PSScriptRoot "..\firmware\main\CMakeLists.txt")

    $errors = @()

    foreach ($file in $cmakeFiles) {
        $content = Get-Content $file.FullName
        $inRequires = $false

        foreach ($line in $content) {
            $trimmed = $line.Trim()

            if ($trimmed -match 'REQUIRES\s+(.+)$') {
                $tokens = $Matches[1] -split '\s+'
                foreach ($token in $tokens) {
                    if ($token -eq ")" -or [string]::IsNullOrWhiteSpace($token)) { continue }
                    if (-not $allowed.ContainsKey($token)) {
                        $errors += "$($file.FullName): unknown REQUIRES component '$token'"
                    }
                }
                $inRequires = $false
                continue
            }

            if ($trimmed -eq "REQUIRES") {
                $inRequires = $true
                continue
            }

            if (-not $inRequires) {
                continue
            }

            if ($trimmed -eq ")" -or $trimmed.StartsWith("INCLUDE_DIRS") -or $trimmed.StartsWith("SRCS")) {
                $inRequires = $false
                continue
            }

            if ([string]::IsNullOrWhiteSpace($trimmed)) {
                continue
            }

            $token = $trimmed.Split()[0]
            if (-not $allowed.ContainsKey($token)) {
                $errors += "$($file.FullName): unknown REQUIRES component '$token'"
            }
        }
    }

    if ($errors.Count -gt 0) {
        $errors | ForEach-Object { Write-Error $_ }
        throw "CMake dependency lint failed"
    }
}

$gcc = Join-Path $ToolchainBin "gcc.exe"
$make = Join-Path $ToolchainBin "mingw32-make.exe"
$cppcheck = Join-Path $ToolchainBin "cppcheck.exe"

Require-Tool $gcc "gcc"
Require-Tool $make "mingw32-make"
Require-Tool $cppcheck "cppcheck"

$env:PATH = "$ToolchainBin;$env:PATH"
$repoRoot = Join-Path $PSScriptRoot ".."

Invoke-Step "CMake dependency lint" {
    Test-CMakeRequires
}

Invoke-Step "Host unit tests" {
    Push-Location (Join-Path $repoRoot "firmware\test")
    try {
        & $make
        if ($LASTEXITCODE -ne 0) {
            throw "Host unit tests failed"
        }
    } finally {
        Pop-Location
    }
}

Invoke-Step "cppcheck" {
    Push-Location $repoRoot
    try {
        & $cppcheck `
            --enable=all `
            --suppress=missingIncludeSystem `
            --suppress=missingInclude `
            --suppress=unusedFunction `
            --suppress=unusedStructMember `
            --suppress=checkersReport `
            --error-exitcode=1 `
            --std=c11 `
            -I firmware/components/modem `
            -I firmware/components/gnss `
            -I firmware/components/relay `
            -I firmware/components/power `
            -I firmware/components/mqtt_wrapper `
            firmware/components/gnss/gnss.c `
            firmware/components/relay/relay.c `
            firmware/components/power/power_monitor.c `
            firmware/test/test_gnss.c `
            firmware/test/test_power.c `
            firmware/test/test_relay.c `
            firmware/test/test_mqtt_json.c
        if ($LASTEXITCODE -ne 0) {
            throw "cppcheck failed"
        }
    } finally {
        Pop-Location
    }
}

Write-Host ""
Write-Host "Local CI checks passed."
