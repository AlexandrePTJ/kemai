// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
.pragma library

// Components Kemai ships with, for the About dialog. Keep in step with the CPM dependencies (root CMakeLists.txt),
// the Qt modules, the font and the icon set: update this list in the same commit as the dependency.
const components = [
    { name: "Qt", license: "LGPL-3.0", url: "https://www.qt.io" },
    { name: "Lucide icons", license: "ISC", url: "https://lucide.dev" },
    { name: "Kimai logo (tray icons)", license: "CC-BY-SA-4.0", url: "https://github.com/kimai/images" },
    { name: "Raveo", license: "OFL-1.1", url: "https://github.com/jakubfoglar/raveo" },
    { name: "qtkeychain", license: "BSD-3-Clause", url: "https://github.com/frankosterfeld/qtkeychain" },
    { name: "spdlog", license: "MIT", url: "https://github.com/gabime/spdlog" },
    { name: "fmt", license: "MIT", url: "https://github.com/fmtlib/fmt" },
    { name: "magic_enum", license: "MIT", url: "https://github.com/Neargye/magic_enum" }
]
