# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: windows-installer-test.ps1 -Msi <new MSI> -Version <version> -LegacyMsi <Kemai 0.11.1 MSI>
# Installs Kemai 0.11.1 and checks the new MSI refuses to install next to it (0.x MSIs installed per-user, which a
# per-machine MSI cannot upgrade). Uninstalls 0.11.1, installs the new MSI and checks: one Kemai in the installed apps,
# with the MSI version; shortcuts created; 0.11's settings file untouched. Runs the installed Kemai.exe
# (windows-smoke-test.ps1), uninstalls it and checks nothing is left but the user's files.
# Needs administrator rights (GitHub-hosted runners have them).
param(
    [Parameter(Mandatory)][string]$Msi,
    [Parameter(Mandatory)][string]$Version,
    [Parameter(Mandatory)][string]$LegacyMsi
)
$ErrorActionPreference = 'Stop'

# msiexec refuses forward slashes in a package path (D:\a\_temp/Kemai.msi): use full Windows paths
$Msi = [IO.Path]::GetFullPath($Msi)
$LegacyMsi = [IO.Path]::GetFullPath($LegacyMsi)

function Invoke-Msiexec([string[]]$Arguments, [string]$LogName, [int[]]$ExitCodes = @(0, 3010)) {
    $logFile = Join-Path ([IO.Path]::GetTempPath()) $LogName
    $process = Start-Process msiexec.exe -ArgumentList ($Arguments + '/qn', '/l*v', "`"$logFile`"") -Wait -PassThru
    # 3010: success, restart required
    if ($process.ExitCode -notin $ExitCodes) {
        Get-Content $logFile -Tail 50 | Write-Host
        throw "msiexec $($Arguments -join ' ') failed with exit code $($process.ExitCode)"
    }
}

function Get-KemaiEntries {
    $keys = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*',
            'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*'
    @(Get-ItemProperty $keys -ErrorAction SilentlyContinue | Where-Object { $_.DisplayName -like 'Kemai*' })
}

$installDir = Join-Path $env:ProgramFiles 'Kemai'

Write-Host "Installing Kemai 0.11.1"
Invoke-Msiexec @('/i', "`"$LegacyMsi`"") 'kemai-legacy-install.log'
if (-not (Test-Path (Join-Path $installDir 'Kemai.exe'))) {
    throw "Kemai 0.11.1 is not installed in $installDir"
}

# 0.11's settings (profiles and tokens) live in the user's config folder: the installers never touch it
$settings = Join-Path $env:LOCALAPPDATA 'Kemai\Kemai\settings.json'
New-Item -ItemType Directory -Force (Split-Path $settings) | Out-Null
Set-Content -Path $settings -Value '{"version": 3, "profiles": []}' -NoNewline
$settingsHash = (Get-FileHash $settings).Hash

Write-Host "Installing $Msi next to it (refused)"
# 1603: fatal error, here the launch condition
Invoke-Msiexec @('/i', "`"$Msi`"") 'kemai-refused.log' -ExitCodes 1603
$entries = @(Get-KemaiEntries)
if ($entries.Count -ne 1 -or $entries[0].DisplayVersion -ne '0.11.1') {
    $entries | Format-Table DisplayName, DisplayVersion | Out-String | Write-Host
    throw "Expected Kemai 0.11.1 alone in the installed apps after the refused install"
}
if (-not (Select-String -Path (Join-Path ([IO.Path]::GetTempPath()) 'kemai-refused.log') -Encoding Unicode -SimpleMatch 'Kemai 0.11 is installed')) {
    throw "The installer did not say Kemai 0.11 must be uninstalled first"
}

Write-Host "Uninstalling Kemai 0.11.1, then installing $Msi"
Invoke-Msiexec @('/x', "`"$LegacyMsi`"") 'kemai-legacy-uninstall.log'
Invoke-Msiexec @('/i', "`"$Msi`"") 'kemai-install.log'
$entries = @(Get-KemaiEntries)
if ($entries.Count -ne 1 -or $entries[0].DisplayVersion -ne $Version) {
    $entries | Format-Table DisplayName, DisplayVersion | Out-String | Write-Host
    throw "Expected one Kemai $Version in the installed apps"
}
$exe = Join-Path $installDir 'bin\Kemai.exe'
if (-not (Test-Path $exe)) {
    throw "Kemai.exe is not installed at $exe"
}
$startMenu = Join-Path $env:ProgramData 'Microsoft\Windows\Start Menu\Programs'
if (-not (Get-ChildItem $startMenu -Recurse -Filter 'Kemai.lnk')) {
    throw "No Kemai shortcut in the Start menu"
}
if (-not (Test-Path (Join-Path $env:PUBLIC 'Desktop\Kemai.lnk'))) {
    throw "No Kemai shortcut on the desktop"
}

& (Join-Path $PSScriptRoot 'windows-smoke-test.ps1') -Exe $exe -Version $Version

Write-Host "Uninstalling"
Invoke-Msiexec @('/x', "`"$Msi`"") 'kemai-uninstall.log'
if (@(Get-KemaiEntries).Count -ne 0) {
    throw "Kemai is still in the installed apps after uninstalling"
}
if (Test-Path $exe) {
    throw "$exe is still there after uninstalling"
}
if ((Get-FileHash $settings).Hash -ne $settingsHash) {
    throw "Kemai 0.11's settings file changed"
}
Write-Host "Kemai $Version ($Version) waited for 0.11.1 to be removed, ran and uninstalled cleanly"
