// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT

// Catch2 headers
#include <catch2/catch_test_macros.hpp>

// Project headers
#include <misc/fuzzyMatch.h>

using kemai::FuzzyMatch;

TEST_CASE("FuzzyMatch::wordPrefixMatch empty query", "[fuzzy]")
{
    CHECK(FuzzyMatch::wordPrefixMatch("", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("", ""));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch simple single-word prefix", "[fuzzy]")
{
    CHECK(FuzzyMatch::wordPrefixMatch("dev", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("kem", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("development", "Development - Kemai"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch is case-insensitive", "[fuzzy]")
{
    CHECK(FuzzyMatch::wordPrefixMatch("DEV", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("dEv", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("KEMAI", "development - kemai"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch crosses word boundaries in order", "[fuzzy]")
{
    // The canonical example: "devk" matches "Development - Kemai"
    CHECK(FuzzyMatch::wordPrefixMatch("devk", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("dk", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("devkem", "Development - Kemai"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch respects left-to-right order", "[fuzzy]")
{
    // "kdev" must NOT match — once 'k' binds to "kemai", no earlier word is reachable.
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("kdev", "Development - Kemai"));
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("kd", "Development - Kemai"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch rejects characters not reachable by a word prefix", "[fuzzy]")
{
    // 'o' is mid-word in "Development", not a word prefix — so "do" fails.
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("do", "Development - Kemai"));
    // 'z' is nowhere in the entry.
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("devz", "Development - Kemai"));
    // 'm' appears mid-"development" and in "kemai" — but after we consume 'devk' we
    // land at the start of "kemai", so 'devkm' would require 'm' to be the 2nd char
    // of "kemai" which it isn't.
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("devkm", "Development - Kemai"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch space forces a word boundary", "[fuzzy]")
{
    // "dev kem" should match across the explicit boundary.
    CHECK(FuzzyMatch::wordPrefixMatch("dev kem", "Development - Kemai"));
    // A space mid-word skips to the next word: "de k" — "de" from "development",
    // then 'k' from "kemai".
    CHECK(FuzzyMatch::wordPrefixMatch("de k", "Development - Kemai"));
    // Trailing space after a complete match is OK.
    CHECK(FuzzyMatch::wordPrefixMatch("dev ", "Development - Kemai"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch any separator acts as a word boundary in the query", "[fuzzy]")
{
    // The entry's own separators must round-trip — typing the label exactly
    // (or removing one trailing letter) must still match.
    CHECK(FuzzyMatch::wordPrefixMatch("Development - Kemai", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("Development - Kema", "Development - Kemai"));
    CHECK(FuzzyMatch::wordPrefixMatch("Development -", "Development - Kemai"));
    // Parentheses also work as boundaries.
    CHECK(FuzzyMatch::wordPrefixMatch("project (v2)", "Project (v2) - Backend"));
    CHECK(FuzzyMatch::wordPrefixMatch("project(v2)b", "Project (v2) - Backend"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch handles numbers and punctuation", "[fuzzy]")
{
    CHECK(FuzzyMatch::wordPrefixMatch("v2", "Project v2 - Backend"));
    CHECK(FuzzyMatch::wordPrefixMatch("pv2b", "Project v2 - Backend"));
    // Separators (dash, parens, etc.) are word delimiters.
    CHECK(FuzzyMatch::wordPrefixMatch("ab", "Alpha (Beta)"));
    CHECK(FuzzyMatch::wordPrefixMatch("ab", "Alpha-Beta"));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch empty entry never matches a non-empty query", "[fuzzy]")
{
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("x", ""));
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("dev", ""));
}

TEST_CASE("FuzzyMatch::wordPrefixMatch query longer than any reachable prefix fails", "[fuzzy]")
{
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("developmentx", "Development - Kemai"));
    CHECK_FALSE(FuzzyMatch::wordPrefixMatch("kemaix", "Development - Kemai"));
}
