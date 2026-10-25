# C++ and QML style and architecture

This document records the agreed C++ and QML conventions. The root `.clang-format` is
the authority for mechanical formatting. `CONTRIBUTING.md` asks contributors
to run it before opening a pull request. This document records the project
conventions for all contributors, rather than relying on local agent
instructions.

## Current project conventions

The project builds as C++20 (`CMAKE_CXX_STANDARD 20` in the root
`CMakeLists.txt`) and targets Qt 6.11+.

- Classes and structs use `PascalCase`; source and header filenames use
  `camelCase`.
- Non-static data members use the `m_` prefix.
- Private implementation headers use the `_p.h` suffix.
- Prefer references when lifetime is guaranteed, then `std::unique_ptr` for
  owned objects. Use `std::shared_ptr` only for shared ownership. Use raw
  pointers where required by Qt ownership APIs.
- Qt-facing classes use `QML_ELEMENT`, `Q_INVOKABLE`, and `Q_PROPERTY` as
  appropriate. The API client uses `QFuture<T>`; asynchronous continuations
  use a context object (`.then(this, ...)`, `.onFailed(this, ...)`).
- C++ source files start with the SPDX copyright and license comments used by
  the project.

In this checkout, additional notes exist in a local `CLAUDE.md`; it is ignored
by Git and points outside this repository, so public contributors cannot rely
on it. This guide records the contributor-facing conventions and core
architecture boundaries.

## What `.clang-format` currently says

The root configuration uses Allman braces, four-space indentation, no tabs,
indented namespaces, right-aligned pointers, no column limit, include groups
regrouped by category, and alignment of consecutive declarations,
assignments, macros, and trailing comments. It permits short blocks and loops
on one line, and short lambdas on one line. Constructor initializers are split
one per line. C++ is currently parsed with `Standard: Auto`.

The agreed style choices are:

- Keep `ColumnLimit: 0`: there is no hard line-length limit.
- Keep alignment for consecutive declarations, assignments, macros, and
  trailing comments.
- Keep the configured compact forms for short blocks, loops, and lambdas.
- Keep `Standard: Auto`.
- Do not add a CI formatting check for now. The whole tree was formatted once
  (commit `80b42f4`, listed in `.git-blame-ignore-revs`); keep it clean by
  formatting the files you change.

### Include order

Use this group order, with a blank line between groups:

1. C++ standard library (STL)
2. Third-party libraries
3. Qt
4. Kemai/project headers

In header files (`.h`), use angle brackets for every include, including
project headers. Do not use quote-style includes in headers. In `.cpp` files,
the same group order applies; quote-style inclusion of the corresponding
project header is permitted.

`.clang-format` now uses `IncludeBlocks: Regroup` and categories for standard
headers, the current third-party libraries and Windows SDK, Qt, and a project
fallback. It keeps the matching header first in a `.cpp`, then groups the
remaining includes. Add a category when introducing a third-party library
whose include path is not already listed. clang-format does not change quote
delimiters, so the header-only angle-bracket rule remains a manual convention.

The following entries were updated to current option names and structures:

- `AlwaysBreakAfterReturnType` → `BreakAfterReturnType`;
- `AlwaysBreakTemplateDeclarations` → `BreakTemplateDeclarations`;
- `BinPackArguments` / `BinPackParameters` → `PackArguments` /
  `PackParameters`;
- `DeriveLineEnding` → `LineEnding`;
- `KeepEmptyLinesAtTheStartOfBlocks` → `KeepEmptyLines.AtStartOfBlock`;
- `SpacesInParentheses` and `SpacesInCStyleCastParentheses` →
  `SpacesInParens`;
- `AlignAfterOpenBracket: Align` → `AlignAfterOpenBracket: true`.

The updated `PackArguments` and `PackParameters` options require
clang-format 23. The project does not currently pin a clang-format version or
run a formatting check in CI. Contributors should therefore use clang-format
23 or newer with the repository configuration. Avoid mixing formatter
versions in a patch: newer releases can change formatting even when the
configuration remains valid.

## Formatting state and lambda arguments

All tracked C++ sources and headers are formatted with the repository
configuration and clang-format 23 (commit `80b42f4`). Run
`git config blame.ignoreRevsFile .git-blame-ignore-revs` so `git blame` skips
that commit. There are no `clang-format off/on` regions: when the formatter
renders something badly, change the shape of the code instead.

The one shape that needs care is a call whose last argument is a multi-line
lambda. With `ColumnLimit: 0`, clang-format keeps such a call on one line and
glues the lambda's closing brace to its last statement (`…; });`). Put one
argument per line instead; clang-format keeps that form and indents the body
properly:

```cpp
connect(job,
        &QKeychain::ReadPasswordJob::finished,
        this,
        [this, key](QKeychain::Job *j)
        {
            ...
        });

m_client->requestTimeSheets()
    .then(this,
          [this](const KimaiTimeSheets &sheets)
          {
              ...
          })
    .onFailed(this, [this](const KimaiApiError &e) { handleApiError(e, "Failed to fetch recent timesheets"); });
```

Short lambdas stay on one line. Headers use angle-bracket includes only.

## Architecture guide

The project is divided into a reusable core library, a Qt Quick application,
and tests. Keep dependencies flowing from the application toward the core;
the core must not depend on the application or QML.

### Layers and allowed dependencies

- **`code/core` (`KemaiCore`)** contains reusable C++ logic: Kimai API client
  and typed API errors, domain/data types, controllers, models, persistence,
  and utilities. It may depend on Qt Core/Network and declared libraries, but
  not on Qt Quick, QML application contexts, or `code/app`.
- **`code/app` (`Kemai`)** is the platform and UI integration layer. It owns
  the QML engine, QML-facing contexts and adapters, application lifecycle, and
  platform-specific presence monitoring. It may depend on `KemaiCore`; core
  code must not call back into app classes.
- **QML** presents state and sends user actions through QML-facing contexts
  and models. Keep business rules in core controllers or models rather than
  duplicating them in view code. Keep view-specific presentation state in
  QML.
- **`code/tests`** verifies core and application behavior. Use the
  `IKimaiClient` interface and its fake for controller/cache tests instead of
  coupling those tests to a live server.

### Async work and errors

- The client layer reports failed futures as `KimaiApiError`, carrying a
  typed error kind and relevant HTTP/API details. It should not construct
  translated UI strings.
- Controllers decide how an API error affects application state and what
  event or notification to emit. Async failure handlers should log and either
  surface the failure or document why it is intentionally ignored.
- The UI turns notification kinds and details into user-facing text. Keep
  these messages in the UI layer.
- Use context-bound future continuations (`.then(this, ...)` and
  `.onFailed(this, ...)`) so callbacks are tied to the owning QObject's
  lifetime and thread context.

These boundaries describe the intended direction of new code. If an existing
implementation crosses a boundary, avoid expanding that dependency and
consider correcting it as a separate change.

## QML style

QML lives in `code/app/qml`: `Main.qml`, the `Theme` singleton, full screens
and dialogs in `Views/`, reusable pieces in `Components/`. Every file is part
of the `Kemai` QML module (`qt_add_qml_module`, URI `Kemai`).

### Files and imports

- One component per file, `PascalCase.qml`, starting with the SPDX header as a
  `//` comment.
- Import Qt modules first (`QtQuick`, `QtQuick.Controls`, `QtQuick.Layouts`),
  then `import Kemai`. Never import a folder by relative path
  (`import "../Components"`): components and C++ types all come from the
  `Kemai` module.
- Add `pragma ComponentBehavior: Bound` to every file with delegates or
  nested `Component`s.
- Register each new file in `code/app/CMakeLists.txt` (`KEMAI_QML_FILES`).

### Lint-clean bindings

`cmake --build --preset <preset> --target Kemai_qmllint` must report no
warning; CI fails on any. In practice:

- Qualify member access with an id (`root.isActive`, `menu.close()`), never a
  bare name or `parent.x` inside `background` / `contentItem`.
- Delegates declare a `required property` for each role, `index` or
  `modelData` they use.
- Inside layouts, size with `Layout.preferredWidth/Height` (and
  `Layout.fillWidth`), never `width` / `height`.
- Never give an `id` the name of a property it is assigned to
  (`KemaiContext { id: kemaiContext }` passed as `kemaiContext: kemaiContext`
  binds the property to itself).
- C++ types used as QML property types are fully qualified in `Q_PROPERTY`
  and signal signatures (`kemai::SessionContext *`) and registered through
  `QML_FOREIGN` shims when they live in the core library.

### Structure of a component

Order the declarations as: `id`, `required` properties, public properties,
private properties (prefixed `_`), signals, functions, then property
assignments and child objects. Keep business rules in C++ controllers and
models; QML holds presentation state only (open/closed, highlighted row,
pending spinner…).

### Look and feel

- Colors, font sizes, radii and spacing come from `Theme`; no literal colors
  in views. Add a token to `Theme.qml` when a design introduces a new one.
- Icons are Lucide line icons through `Icon { name; size; color }`; add one
  by appending its path data to `LucideIcons.js`. No emoji, Unicode glyphs or
  bitmaps as icons.
- Text uses the application font (Raveo). Times, durations and other
  numbers use `font.features: Theme.fontFeaturesTabular`; the monospace font
  is only for hexadecimal values such as certificate fingerprints.
- Modal dialogs are `Popup`s with explicit size, manual centering,
  `padding: 0`, a `background` rectangle and a `ColumnLayout` content; do not
  use the Controls `Dialog` type. Footers use `DialogButton` (`subtle`,
  `primary`, `danger`, `ghost`); a request in progress sets `busy` at once and
  `spinning` after about 150 ms.
- Input uses pointer handlers (`TapHandler`, `HoverHandler`, `DragHandler`)
  rather than `MouseArea`.
- Every scrollable list declares a `WheelScroller { flickable: <its id> }`.
- A `Text` whose layout width follows its `implicitWidth` uses
  `Math.ceil(implicitWidth)`: a width a fraction of a pixel smaller elides it.

### Errors and user-facing text

Views turn notification kinds and details into words (see "Async work and
errors"); the core never builds user-facing text. Login failures are worded
by the login view, background failures by the notification banner, and
operations started from a dialog by that dialog.

## Applying the rules

Format changed C++ files with clang-format 23 or newer, using the root
configuration, and keep QML lint-clean (see the QML section). Review the
resulting diff; do not add `clang-format off/on` regions. Formatting is a contributor responsibility;
the project does not currently enforce it in CI. Do not use a
formatting-only whole-tree change to conceal functional edits in the same
review.
