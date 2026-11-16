// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
.pragma library

// StyledText of `text` with the first case-insensitive match of `query` in `color` and bold (StyledText ignores
// <span style>); the text is escaped
function highlight(text, query, color) {
    const esc = s => s.replace(/&/g, "&amp;").replace(/</g, "&lt;")
    const i = query === "" ? -1 : text.toLowerCase().indexOf(query.toLowerCase())
    if (i < 0)
        return esc(text)
    return esc(text.slice(0, i)) + "<font color='" + color + "'><b>" + esc(text.slice(i, i + query.length)) + "</b></font>" + esc(text.slice(i + query.length))
}
