#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Alexandre Petitjean
# SPDX-License-Identifier: MIT
#
# Usage: release-channel.sh <tag> [<commit>]
# Checks that <tag> is a calendar version (YEAR.MONTH.PATCH) and derives the release channel from the remote
# branches containing <commit> (default: the tag's commit): main/master -> stable, else develop -> alpha.
# Prints "version=<tag>" and "channel=<channel>" (GitHub output format); exits non-zero when nothing may be published.
# Needs the full history and the remote branches (actions/checkout with fetch-depth: 0).
set -euo pipefail

tag="${1:?usage: release-channel.sh <tag> [<commit>]}"
commit="${2:-$tag}"

if [[ ! "$tag" =~ ^20[0-9]{2}\.(1[0-2]|[1-9])\.[0-9]+$ ]]; then
    echo "error: tag '$tag' is not a calendar version YEAR.MONTH.PATCH" >&2
    exit 1
fi

sha="$(git rev-parse --verify --quiet "${commit}^{commit}")" || {
    echo "error: '$commit' is not a commit" >&2
    exit 1
}

branches="$(git branch -r --contains "$sha" --format='%(refname:short)')"

# Stable wins: a release merged from develop into main is on both
if grep -qxE 'origin/(main|master)' <<<"$branches"; then
    channel=stable
elif grep -qx 'origin/develop' <<<"$branches"; then
    channel=alpha
else
    echo "error: commit $sha of tag '$tag' is on none of main, master or develop" >&2
    exit 1
fi

echo "version=$tag"
echo "channel=$channel"
