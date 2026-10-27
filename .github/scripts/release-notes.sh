#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: release-notes.sh [--flathub] [--windows-zip <file name>] <version> <channel> [<changelog>]
# Prints the release notes: the "## [<version>]" section of the changelog (default CHANGELOG.md), then, with
# --flathub, how to install it from Flathub (flathub-beta for an alpha), with --windows-zip, how to run the signed
# Windows package attached under that name, then how to build that release from source. Exits non-zero when the changelog has no entry for <version>.
set -euo pipefail

usage="usage: release-notes.sh [--flathub] [--windows-zip <file name>] <version> <channel> [<changelog>]"
flathub=false
if [[ "${1:-}" == "--flathub" ]]; then
    flathub=true
    shift
fi
windows_zip=""
if [[ "${1:-}" == "--windows-zip" ]]; then
    windows_zip="${2:?$usage}"
    shift 2
fi

version="${1:?$usage}"
channel="${2:?$usage}"
changelog="${3:-CHANGELOG.md}"

# Section body: from the line after "## [<version>]" up to the next "## [" heading
entry="$(awk -v heading="## [$version]" '
    index($0, "## [") == 1 { if (inside) exit; inside = (index($0, heading) == 1); next }
    inside { print }
' "$changelog")"

if [[ -z "${entry//[[:space:]]/}" ]]; then
    echo "error: $changelog has no entry for $version" >&2
    exit 1
fi

cat <<NOTES
$entry
NOTES

if [[ "$flathub" == true ]]; then
    if [[ "$channel" == alpha ]]; then
        cat <<NOTES

## Linux

Install it from Flathub's beta repository, once its build is published there:

\`\`\`
flatpak remote-add --if-not-exists flathub-beta https://flathub.org/beta-repo/flathub-beta.flatpakrepo
flatpak install flathub-beta io.github.AlexandrePTJ.Kemai
\`\`\`
NOTES
    else
        cat <<NOTES

## Linux

Install it from [Flathub](https://flathub.org/apps/io.github.AlexandrePTJ.Kemai), once its build is published there: \`flatpak install flathub io.github.AlexandrePTJ.Kemai\`.
NOTES
    fi
fi

if [[ -n "$windows_zip" ]]; then
    cat <<NOTES

## Windows

Download \`$windows_zip\` below, unzip it anywhere and run \`Kemai.exe\`: nothing is installed, delete the folder to remove it. Windows 10 or later, 64-bit.

\`Kemai.exe\` is signed: Windows shows **SignPath Foundation** as its publisher. See the [code signing policy](https://github.com/AlexandrePTJ/kemai#code-signing-policy).
NOTES
fi

cat <<NOTES

## Build from source

Kemai can be built from source on every platform. To build this release:

\`\`\`
git clone --branch $version https://github.com/AlexandrePTJ/kemai.git
cd kemai
cmake --preset release -DCMAKE_PREFIX_PATH=<Qt install dir>/<Qt version>/<arch> -DKEMAI_VERSION=$version -DKEMAI_CHANNEL=$channel
cmake --build --preset release
\`\`\`

See [Build from source](https://github.com/AlexandrePTJ/kemai#build-from-source) in the README for the dependencies of each platform.
NOTES
