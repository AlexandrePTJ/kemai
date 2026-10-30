// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
.pragma library

// Periods of the history, in menu order (keys of SessionController::historyPeriod)
const keys = ["week", "month", "last30"]

function name(key) {
    return key === "month" ? "This month" : key === "last30" ? "Last 30 days" : "This week"
}

// The period inside a sentence: "Nothing tracked <this week> yet"
function phrase(key) {
    return key === "month" ? "this month" : key === "last30" ? "in the last 30 days" : "this week"
}

// Where the filter looks: "The filter looks <in this week> (…) only"
function scope(key) {
    return key === "month" ? "in this month" : key === "last30" ? "in the last 30 days" : "in this week"
}

// "Oct 26 – Nov 1", "Oct 1 – 31", "Dec 28 – Jan 3"
function dates(first, last, locale) {
    if (!first || !last || isNaN(first.getTime()) || isNaN(last.getTime()))
        return ""
    const sameMonth = first.getFullYear() === last.getFullYear() && first.getMonth() === last.getMonth()
    return first.toLocaleDateString(locale, "MMM d") + " – " + last.toLocaleDateString(locale, sameMonth ? "d" : "MMM d")
}
