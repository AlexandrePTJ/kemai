#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: release-notes.sh <version> <channel> [<changelog>]
# Prints the release notes: the "## [<version>]" section of the changelog (default CHANGELOG.md), followed by
# how to build that release from source. Exits non-zero when the changelog has no entry for <version>.
set -euo pipefail

version="${1:?usage: release-notes.sh <version> <channel> [<changelog>]}"
channel="${2:?usage: release-notes.sh <version> <channel> [<changelog>]}"
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

## Build from source

Kemai releases are source-only. To build this release:

\`\`\`
git clone --branch $version https://github.com/AlexandrePTJ/kemai.git
cd kemai
cmake --preset release -DKEMAI_VERSION=$version -DKEMAI_CHANNEL=$channel
cmake --build --preset release
\`\`\`

See [Build from source](https://github.com/AlexandrePTJ/kemai#build-from-source) in the README for the dependencies of each platform.
NOTES
