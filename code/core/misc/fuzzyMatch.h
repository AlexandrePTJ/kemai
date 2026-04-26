// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// Qt headers
#include <QString>

namespace kemai
{
    class FuzzyMatch
    {
    public:
        // Smart word-prefix sequential filter.
        //
        // Splits `entry` into alphanumeric words, then walks `query` left-to-right.
        // Each alphanumeric query character must either extend the current word
        // (matching its next character) or jump to a later word and match its
        // first character. Any non-alphanumeric query character (space, dash,
        // parenthesis, etc.) acts as a word boundary, mirroring the way the
        // entry was tokenized — so a label can fuzzy-match itself even when it
        // contains separators.
        //
        // Example: "devk" matches "Development - Kemai" because 'd','e','v'
        // extend "development" and 'k' starts "kemai". "kdev" does NOT match
        // because once 'k' binds to "kemai" there is no word starting with 'd'
        // after it.
        //
        // Matching is case-insensitive. An empty query matches anything.
        static bool wordPrefixMatch(const QString &query, const QString &entry);
    };
} // namespace kemai
