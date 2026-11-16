# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: windows-check-signature.ps1 -Path <file> [-Version <version>] [-TestSigning]
# Checks that <file> carries a valid Authenticode signature by SignPath Foundation and, with -Version, that its
# product name is Kemai and its product version <version>. -TestSigning (SignPath's test-signing policy, whose test
# certificate Windows does not trust) only requires a signature to be present.
param(
    [Parameter(Mandatory)][string]$Path,
    [string]$Version,
    [switch]$TestSigning
)
$ErrorActionPreference = 'Stop'

$signature = Get-AuthenticodeSignature $Path
$signer = $signature.SignerCertificate.Subject
Write-Host "${Path}: signature $($signature.Status), signer $signer"
if ($TestSigning) {
    if (-not $signature.SignerCertificate) {
        throw "$Path is not signed"
    }
} elseif ($signature.Status -ne 'Valid' -or $signer -notmatch 'SignPath Foundation') {
    throw "$Path is not validly signed by SignPath Foundation"
}

if ($Version) {
    $info = (Get-Item $Path).VersionInfo
    if ($info.ProductName -ne 'Kemai' -or $info.ProductVersion -ne $Version) {
        throw "$Path is '$($info.ProductName)' $($info.ProductVersion), expected 'Kemai' $Version"
    }
}
