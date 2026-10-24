# C++ style and architecture

This document records the agreed C++ conventions. The root `.clang-format` is
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
- Do not add a CI formatting check and do not reformat the whole tree as part
  of this guide.

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

## Audit findings

This is a working-tree audit, not a clean-branch baseline: there were already
uncommitted source and QML changes while it was performed. With the current
configuration and local clang-format 23.1.3, `clang-format --dry-run --Werror`
reports formatting differences in 46 tracked C++ source/header files. This
count was measured after enabling include regrouping and includes the locally
modified tracked files; it is not a clean-branch baseline. The check was
read-only.

Representative differences include:

- [appPreferences.cpp](../code/app/context/appPreferences.cpp): constructor
  initializer spacing differs from the configured form.
- [appPreferences.h](../code/app/context/appPreferences.h): manually aligned
  return types differ from the formatter's alignment output.
- [kemaiContext.cpp](../code/app/context/kemaiContext.cpp): callback chains
  are exempted with `clang-format off/on`; other files also have local
  exemptions, for example `secretStorage.cpp` and `kimaiClient_p.h`.
- Include organization in existing source is inconsistent: some files group
  STL, Qt, project, and third-party headers in different orders, while others
  use unlabelled groups. `IncludeBlocks: Regroup` now makes clang-format
  normalize the groups when a file is formatted.
- Three headers use quote-style includes, contrary to the agreed header rule:
  [linuxPresenceBackend.h](../code/app/monitor/linuxPresenceBackend.h),
  [kimaiClient_p.h](../code/core/client/kimaiClient_p.h) (two includes).
- Some project-specific guidance currently lives only in the ignored local
  `CLAUDE.md`. It is not part of the public repository and cannot serve as a
  contributor-facing source of truth.

These findings show that the formatter is not being applied consistently.
They do not by themselves indicate that the conventions encoded in
`.clang-format` are wrong. A whole-tree formatting pass would produce a large
diff and should be considered separately from deciding the rules.

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

## Applying the rules

Format changed C++ files with clang-format 23 or newer, using the root
configuration. Review the resulting diff, especially around existing
`clang-format off/on` regions. Formatting is a contributor responsibility;
the project does not currently enforce it in CI. Do not use a
formatting-only whole-tree change to conceal functional edits in the same
review.
