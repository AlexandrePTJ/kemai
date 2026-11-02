# Code style and architecture

## Layers

- **`code/core`** (`kemai_core`): Kimai API client, typed errors, data types, controllers, models, storage, utilities. Qt Core/Network only: no Qt Quick, nothing from `code/app`.
- **`code/app`** (`Kemai`): QML engine, thin QML wrappers over core controllers, views, platform integration. Business rules stay in core; QML holds presentation state only.
- **`code/tests`**: Catch2 tests; controller and cache tests use the `IKimaiClient` fake, not a server.

If existing code crosses a boundary, do not extend it; fix it in a separate change.

## Errors and async

- The client fails futures with a `KimaiApiError` (kind, HTTP status, server message). No user-facing text in core.
- Controllers decide what an error does. Every failure handler logs, then either reaches the UI or says in a comment why it is silent.
- The UI words errors: the login view, the notification banner, or the dialog that started the operation.
- Continuations take a context object: `.then(this, …)`, `.onFailed(this, …)`.

## C++

- C++20, Qt 6.11+, namespace `kemai`.
- Classes `PascalCase`, files `camelCase`, members `m_`, private headers `_p.h`.
- Ownership, strictest first: reference, `std::unique_ptr`, `std::shared_ptr` (only when shared), raw pointer (only for Qt ownership).
- Every file starts with the SPDX header.
- Includes in groups: standard library, third-party, Qt, Kemai. Headers use angle brackets only.
- Format with the root `.clang-format` and **clang-format 23+** (`git config blame.ignoreRevsFile .git-blame-ignore-revs` hides the formatting commit). No `clang-format off` regions: reshape the code instead. A call whose last argument is a multi-line lambda takes one argument per line:

```cpp
m_client->requestTimeSheets()
    .then(this,
          [this](const KimaiTimeSheets &sheets)
          {
              ...
          })
    .onFailed(this, [this](const KimaiApiError &e) { handleApiError(e, "Failed to fetch recent timesheets"); });
```

## QML

Files live in `code/app/qml` (`Views/`, `Components/`), all in the `Kemai` module, registered in `KEMAI_QML_FILES`. Format with `qmlformat -i` (root `.qmlformat.ini`).

- One `PascalCase.qml` component per file, SPDX header first. Imports: Qt modules, then `import Kemai`, then JavaScript files. Never import a folder by path.
- Order: `id`, `required` properties, public properties, private ones (`_name`), signals, functions, assignments and children.

**Lint**: `Kemai_qmllint` must print no warning.

- `pragma ComponentBehavior: Bound` in files with delegates or nested `Component`s; delegates declare a `required property` per role, `index` or `modelData`.
- Qualify member access with an id (`root.isActive`), never a bare name or `parent.x` in `background`/`contentItem`.
- In layouts, `Layout.preferredWidth/Height`, never `width`/`height`.
- Never give an `id` the name of the property it is assigned to (`kemaiContext: kemaiContext` binds to itself).
- C++ types in `Q_PROPERTY` and signals are fully qualified (`kemai::SessionContext *`); core types go through `QML_FOREIGN` shims.

**Look and behavior**

- Colors, sizes, radii and spacing from `Theme`; no literal colors.
- Lucide icons through `Icon { name; size; color }` (path data in `LucideIcons.js`); no emoji or bitmaps.
- Raveo everywhere; numbers, times and durations with `font.features: Theme.fontFeaturesTabular`. Monospace only for certificate fingerprints and raw log text.
- Modal dialogs are `Popup`s (explicit size, manual centering, `padding: 0`, `background` rectangle, `ColumnLayout`), never Controls `Dialog`. Footers use `DialogButton`; a running request sets `busy` at once and `spinning` after ~150 ms.
- Pointer handlers, never `MouseArea`. A clickable `TapHandler` sets `gesturePolicy: TapHandler.ReleaseWithinBounds`, or the press also reaches what lies beneath.
- Every scrollable list declares `WheelScroller { flickable: <its id> }`.
- A `Text` sized to its `implicitWidth` uses `Math.ceil(implicitWidth)`, or it elides.
