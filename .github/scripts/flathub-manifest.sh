#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: flathub-manifest.sh <version> <commit> <channel> [<reference manifest>]
# Prints the Flathub manifest of a release: the reference manifest (default
# packaging/flatpak/io.github.AlexandrePTJ.Kemai.yml) with Kemai built from the release tag and commit of the GitHub
# repository, with that release's version and channel. The kemai module must be the last one of the reference.
set -euo pipefail

usage="usage: flathub-manifest.sh <version> <commit> <channel> [<reference manifest>]"
version="${1:?$usage}"
commit="${2:?$usage}"
channel="${3:?$usage}"
reference="${4:-packaging/flatpak/io.github.AlexandrePTJ.Kemai.yml}"

if [[ ! "$version" =~ ^20[0-9]{2}\.(1[0-2]|[1-9])\.[0-9]+$ ]]; then
    echo "error: '$version' is not a calendar version YEAR.MONTH.PATCH" >&2
    exit 1
fi
if [[ ! "$commit" =~ ^[0-9a-f]{40}$ ]]; then
    echo "error: '$commit' is not a full commit hash" >&2
    exit 1
fi
if [[ "$channel" != alpha && "$channel" != stable ]]; then
    echo "error: channel must be alpha or stable (got '$channel')" >&2
    exit 1
fi
if ! grep -qx '  - name: kemai' "$reference"; then
    echo "error: $reference has no kemai module" >&2
    exit 1
fi

# The reference without its header comment about itself; in the kemai module, the version and channel options are
# added and the working-tree source becomes the release's git source
awk -v version="$version" -v commit="$commit" -v channel="$channel" '
    !started && /^#/ { if (/SPDX/) print; next }
    { started = 1 }
    /^  - name: kemai$/ { kemai = 1 }
    kemai && /^    sources:$/ {
        print "      - -DKEMAI_VERSION=" version
        print "      - -DKEMAI_CHANNEL=" channel
        print
        print "      - type: git"
        print "        url: https://github.com/AlexandrePTJ/kemai.git"
        print "        tag: \x27" version "\x27"
        print "        commit: " commit
        exit
    }
    { print }
' "$reference"
