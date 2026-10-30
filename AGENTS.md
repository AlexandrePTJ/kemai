# Instructions for AI coding agents

This file is for AI coding agents (Claude Code, Codex, Copilot, Cursor…) working on Kemai on behalf of a contributor. Humans: see [CONTRIBUTING.md](CONTRIBUTING.md), which these instructions follow.

## What you may do

Kemai accepts AI-assisted contributions under conditions (CONTRIBUTING.md, *Using AI tools*). Check which kind of task you were given before writing code:

- **Bug fix**: go ahead. Keep the diff limited to the fix, add a test that fails without it when possible, and do not refactor or reformat code around it.
- **New feature or change of behavior** (anything a user would notice beyond a bug being fixed): **do not implement it** unless the contributor points you to a proposal the maintainer has agreed to. Instead, help them write that proposal: the need, what the user sees and does, the states and errors, what is out of scope, the impact on existing users. No visual design: the maintainer designs new screens.
- When a "fix" turns out to need a behavior change, stop and say so.

Never open issues or pull requests, post comments, or reply to reviews yourself. Never push. Leave that to the contributor, and remind them to mention in the pull request that an AI tool was used.

## The project

Kemai is a desktop client for [Kimai](https://www.kimai.org/) (time tracking), in C++20 with Qt 6.11+ and QML, built with CMake 3.30+ and Ninja. Work is based on the `develop` branch; the 0.11.x line (`master`) no longer changes.

```
code/core/   kemai_core: pure C++ library (REST client, cache, controllers, models, storage). No Qt Quick.
code/app/    the QML application: thin QML wrappers over core controllers, QML views and components.
code/tests/  Catch2 tests of core and app logic, with a fake Kimai client (mocks/fakeKimaiClient.h).
```

Business logic goes in a core controller, which the app exposes to QML through a thin wrapper. Read [docs/CODE_STYLE.md](docs/CODE_STYLE.md) before changing code: it describes the layers, the error-handling rules and the C++ and QML conventions. The release workflow and the Windows packaging (version resource, runtime deployment, installer, signing) are described in [docs/RELEASING.md](docs/RELEASING.md).

## Commands

The presets need Qt: pass `-DCMAKE_PREFIX_PATH=<Qt dir>/<version>/<arch>` at configure time when Qt is not on the path.

```shell
cmake --preset debug                                   # configure (tests included)
cmake --build --preset debug                           # build
ctest --test-dir build/debug --output-on-failure       # tests
cmake --build --preset debug --target Kemai_qmllint    # QML lint: must print no "Warning:" line
clang-format -i <changed C++ files>                    # root .clang-format
qmlformat -i <changed QML files>                       # root .qmlformat.ini
```

A task is done only when the build passes, the tests pass and the QML lint prints no warning. Format only the files you changed.

Integration tests run against a real Kimai server when `KEMAI_TEST_HOST` and `KEMAI_TEST_TOKEN` are set, and create and delete a time entry there. Do not set them yourself; never point them at a production server.

## Rules that are easy to miss

- **Secrets**: never log, print, store in a plain file or show an API token. Tokens go to the system keychain only.
- **Errors**: the core types errors (`KimaiApiError`), controllers decide what to do with them, the UI words them. Every failed request is logged and either reaches the UI or carries a comment saying why it is silent. No user-facing text in `code/core`.
- **Async**: QFuture continuations use a context object: `.then(this, …)` and `.onFailed(this, …)`.
- **QML**: no Qt Quick Controls `Dialog` (use the `Popup` pattern of the existing dialogs), no `MouseArea` (use pointer handlers), no Qt Widgets. Declare `required property` in delegates and qualify member access with an id.
- **Files**: every new source file starts with the SPDX header:

  ```cpp
  // SPDX-FileCopyrightText: 2026 <contributor's name>
  // SPDX-License-Identifier: MIT
  ```

- **Dependencies**: do not add one without the contributor's agreement. When one is added, removed or relicensed, update `code/app/qml/Views/ThirdParty.js` in the same change.
- **Settings files**: never write, move or delete `settings.json`: it belongs to Kemai 0.11.x, which may be installed alongside. Read it only through `LegacySettingsReader`, to import its profiles. The stable Windows installer stops until 0.11.x is uninstalled, and never touches its settings.
- **Kimai API**: Kimai 2.x rejects unknown fields in request bodies; send only the fields the endpoint needs.

## Commits

One short line saying what the commit does (`Stop the timer from the tray menu`), no body. For a user-visible change, add a line under `## [Unreleased]` in `CHANGELOG.md`.
