# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: windows-smoke-test.ps1 -Exe <path to Kemai.exe> -Version <version> [-Seconds <n>]
# Starts Kemai.exe on a scratch root for a few seconds, then checks that it was still running and that its log shows
# this version starting and no QML, plugin or library loading error. Exits non-zero otherwise.
# Not `Kemai.exe --version`: a WIN32 executable shows it in a message box, which would hang the runner.
param(
    [Parameter(Mandatory)][string]$Exe,
    [Parameter(Mandatory)][string]$Version,
    [int]$Seconds = 15
)
$ErrorActionPreference = 'Stop'

$root = Join-Path ([IO.Path]::GetTempPath()) "kemai-smoke-$([guid]::NewGuid())"
$process = Start-Process -FilePath $Exe -ArgumentList '--root', "`"$root`"" -PassThru
Start-Sleep -Seconds $Seconds
$running = -not $process.HasExited
if ($running) {
    Stop-Process -Id $process.Id -Force
    # Stop-Process does not wait: the log stays locked until Kemai is gone
    $process.WaitForExit()
}

$log = Join-Path $root 'logs/kemai.log'
if (-not (Test-Path $log)) {
    throw "Kemai wrote no log at $log"
}
$lines = @(Get-Content $log)
$lines | Write-Host

if (-not $running) {
    throw "Kemai exited after less than $Seconds s (exit code $($process.ExitCode))"
}
if (-not ($lines | Select-String -SimpleMatch "===== Starting Kemai $Version (")) {
    throw "The log does not show Kemai $Version starting"
}
$failures = $lines | Select-String -Pattern @(
    '\[(error|critical)\]',
    'failed to load component',
    'is not installed',
    'Cannot load library',
    'Could not (find|load) the Qt platform plugin'
)
if ($failures) {
    throw "Kemai logged loading errors:`n$($failures -join "`n")"
}
Write-Host "Kemai $Version started and ran $Seconds s without loading errors"
Remove-Item -Recurse -Force $root
